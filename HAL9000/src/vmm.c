#include "HAL9000.h"
#include "vmm.h"
#include "synch.h"
#include "pmm.h"
#include "bitmap.h"
#include "cpumu.h"
#include "mtrr.h"
#include "io.h"
#include "vm_reservation_space.h"
#include "thread_internal.h"
#include "process_internal.h"
#include "mdl.h"

#define VMM_SIZE_FOR_RESERVATION_METADATA            (5*TB_SIZE)

typedef struct _VMM_DATA
{
    VMM_RESERVATION_SPACE   VmmReservationSpace;


    // Global paging related defines
    // No matter what CR3 we're using the same WB and UC indexes will be used
    BYTE                    WriteBackIndex;
    BYTE                    UncacheableIndex;
} VMM_DATA, *PVMM_DATA;

static VMM_DATA m_vmmData;

static
void
_VmSetupPagingStructure(
    IN      PPAGING_DATA            PagingData,
    IN      PVOID                   PagingStructure
    );

static
BOOL_SUCCESS
BOOLEAN
_VmDeterminePatIndices(
    IN      QWORD                   PatValues,
    OUT     PBYTE                   WbIndex,
    OUT     PBYTE                   UcIndex
    );

__forceinline
static
PHYSICAL_ADDRESS
_VmRetrieveNextPhysicalAddressForPagingStructure(
    IN      PPAGING_DATA            PagingData
    )
{
    QWORD nextAddress;
    DWORD currentIndex;

    ASSERT( NULL != PagingData );

    currentIndex = PagingData->CurrentIndex;

    ASSERT( currentIndex < PagingData->NumberOfFrames );

    nextAddress = (QWORD) PagingData->BasePhysicalAddress + currentIndex * PAGE_SIZE;
    PagingData->CurrentIndex++;

    return (PHYSICAL_ADDRESS) nextAddress;
}

__forceinline
static
BOOLEAN
_VmIsKernelAddress(
    IN      PVOID                   Address
    )
{
    return IsBooleanFlagOn((QWORD)Address, (QWORD)1 << VA_HIGHEST_VALID_BIT);
}

__forceinline
static
BOOLEAN
_VmIsKernelRange(
    IN      PVOID                   Address,
    IN      QWORD                   RangeSize
    )
{
    ASSERT(RangeSize != 0);

    // Check both ends, if one of the addresses is a KM address => game over :)
    return _VmIsKernelAddress(Address) || _VmIsKernelAddress(PtrOffset(Address, RangeSize - 1));
}

__forceinline
static
PTR_SUCCESS
PVMM_RESERVATION_SPACE
_VmmRetrieveReservationSpaceForAddress(
    IN      PVOID           VirtualAddress
    )
{
    if (_VmIsKernelAddress(VirtualAddress))
    {
        return &m_vmmData.VmmReservationSpace;
    }
    else
    {
        PTHREAD pThread;

        // by the time we have UM processes it's clear that we'll have a populated THREAD structure
        // with a process to which it belongs

        pThread = GetCurrentThread();

        ASSERT(pThread != NULL);
        ASSERT(pThread->Process != NULL);

        return pThread->Process->VaSpace;
    }
}

_No_competing_thread_
void
VmmPreinit(
    void
    )
{
    memzero(&m_vmmData, sizeof(VMM_DATA));
}

_No_competing_thread_
STATUS
VmmInit(
    IN      PVOID                   BaseAddress
    )
{
    if (BaseAddress == NULL)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    VmReservationSpaceInit(BaseAddress,
                           NULL,
                           VMM_SIZE_FOR_RESERVATION_METADATA,
                           &m_vmmData.VmmReservationSpace);

    return STATUS_SUCCESS;
}

PTR_SUCCESS
PVOID
VmmMapMemoryEx(
    IN      PPAGING_DATA            PagingData,
    IN      PHYSICAL_ADDRESS        PhysicalAddress,
    IN      QWORD                   Size,
    IN      PAGE_RIGHTS             PageRights,
    IN      BOOLEAN                 Invalidate,
    IN      BOOLEAN                 Uncacheable
    )
{
    PVOID pVirtualAddress;

    if (PagingData == NULL)
    {
        return NULL;
    }

    if (!IsAddressAligned(PhysicalAddress, PAGE_SIZE))
    {
        return NULL;
    }

    if ((0 == Size) || (!IsAddressAligned(Size, PAGE_SIZE)))
    {
        return NULL;
    }

    pVirtualAddress = VmReservationSpaceDetermineNextFreeVirtualAddress(&m_vmmData.VmmReservationSpace, Size);
    LOG_TRACE_VMM("Virtual address: 0x%X\n", pVirtualAddress);
    ASSERT(IsAddressAligned(pVirtualAddress, PAGE_SIZE));

    VmmMapMemoryInternal(PagingData,
                         PhysicalAddress,
                         Size,
                         pVirtualAddress,
                         PageRights,
                         Invalidate,
                         Uncacheable
                         );

    return pVirtualAddress;
}

void
VmmMapMemoryInternal(
    IN      PPAGING_DATA            PagingData,
    IN      PHYSICAL_ADDRESS        PhysicalAddress,
    IN      QWORD                   Size,
    IN      PVOID                   BaseAddress,
    IN      PAGE_RIGHTS             PageRights,
    IN      BOOLEAN                 Invalidate,
    IN      BOOLEAN                 Uncacheable
    )
{
    PML4_ENTRY* pml4Entries;
    PDPT_ENTRY_PD* pdptEntries;
    PD_ENTRY_PT* pdEntries;
    PT_ENTRY* ptEntries;

    WORD pageOffset;
    WORD pteOffset;
    WORD pdeOffset;
    WORD pdpteOffset;
    WORD pml4Offset;

    QWORD offset;
    PVOID tempAddress;

    ASSERT(PagingData != NULL);
    ASSERT(IsAddressAligned(PhysicalAddress, PAGE_SIZE));
    ASSERT(0 != Size && IsAddressAligned(Size, PAGE_SIZE));

    // we may need to map multiple pages => we iterate until we map all the
    // addresses
    for(offset = 0;
        offset < Size;
        offset = offset + PAGE_SIZE)
    {
        // address to map
        tempAddress = (PBYTE)BaseAddress + offset;

        // these are the offsets to the corresponding structures
        pml4Offset = MASK_PML4_OFFSET(tempAddress);
        pdpteOffset = MASK_PDPTE_OFFSET(tempAddress);
        pdeOffset = MASK_PDE_OFFSET(tempAddress);
        pteOffset = MASK_PTE_OFFSET(tempAddress);
        pageOffset = MASK_PAGE_OFFSET(tempAddress);

        pml4Entries = (PML4_ENTRY*)PA2VA(PagingData->BasePhysicalAddress);
        pml4Entries = &(pml4Entries[pml4Offset]);

        if (!PteIsPresent(pml4Entries))
        {
            _VmSetupPagingStructure(PagingData, pml4Entries);
        }

        tempAddress = PteGetPhysicalAddress(pml4Entries);
        pdptEntries = (PDPT_ENTRY_PD*)PA2VA(tempAddress);

        pdptEntries = &(pdptEntries[pdpteOffset]);

        if (!PteIsPresent(pdptEntries))
        {
            _VmSetupPagingStructure(PagingData, pdptEntries);
        }

        ASSERT(0 == pdptEntries->PageSize);

        tempAddress = PteGetPhysicalAddress(pdptEntries);
        pdEntries = (PD_ENTRY_PT*)PA2VA(tempAddress);

        pdEntries = &(pdEntries[pdeOffset]);
        if (!PteIsPresent(pdEntries))
        {
            _VmSetupPagingStructure(PagingData, pdEntries);
        }

        ASSERT(0 == pdEntries->PageSize);

        tempAddress = PteGetPhysicalAddress(pdEntries);
        ptEntries = (PT_ENTRY*)PA2VA(tempAddress);


        ptEntries = &(ptEntries[pteOffset]);

        // if we must invalidate the entry or the entry is not present, map it
        if (Invalidate || (!PteIsPresent(ptEntries)))
        {
            PHYSICAL_ADDRESS physAddr = (PHYSICAL_ADDRESS)((PBYTE)PhysicalAddress + offset);
            PTE_MAP_FLAGS flags = { 0 };

            flags.Executable = IsBooleanFlagOn(PageRights, PAGE_RIGHTS_EXECUTE);
            flags.Writable = IsBooleanFlagOn(PageRights, PAGE_RIGHTS_WRITE);
            flags.PatIndex = Uncacheable ? m_vmmData.UncacheableIndex : m_vmmData.WriteBackIndex;
            flags.GlobalPage = PagingData->KernelSpace;
            flags.UserAccess = !PagingData->KernelSpace;

            PteMap(ptEntries, physAddr, flags );

            __invlpg((PBYTE) BaseAddress + offset );
        }
    }
}

void
VmmUnmapMemoryEx(
    IN      PML4                    Cr3,
    IN      PVOID                   VirtualAddress,
    IN      QWORD                   Size,
    IN      BOOLEAN                 ReleaseMemory
    )
{
    PML4_ENTRY* pml4Entries;
    PDPT_ENTRY_PD* pdptEntries;
    PD_ENTRY_PT* pdEntries;
    PT_ENTRY* ptEntries;

    WORD pageOffset;
    WORD pteOffset;
    WORD pdeOffset;
    WORD pdpteOffset;
    WORD pml4Offset;

    QWORD offset;
    PVOID tempAddress;

    if ((NULL == VirtualAddress) || (!IsAddressAligned(VirtualAddress, PAGE_SIZE)))
    {
        return;
    }

    if ((0 == Size) || (!IsAddressAligned(Size, PAGE_SIZE)))
    {
        return;
    }

    offset = 0;

    // we may need to map multiple pages => we iterate until we map all the
    // addresses
    for(offset = 0;
        offset < Size;
        offset = offset + PAGE_SIZE)
    {
        // address to ummap
        tempAddress = (PVOID)((BYTE*)VirtualAddress + offset);

        // these are the offsets to the corresponding structures
        pml4Offset = MASK_PML4_OFFSET(tempAddress);
        pdpteOffset = MASK_PDPTE_OFFSET(tempAddress);
        pdeOffset = MASK_PDE_OFFSET(tempAddress);
        pteOffset = MASK_PTE_OFFSET(tempAddress);
        pageOffset = MASK_PAGE_OFFSET(tempAddress);

        pml4Entries = (PML4_ENTRY*)PA2VA(Cr3.Pcide.PhysicalAddress << SHIFT_FOR_PHYSICAL_ADDR);
        pml4Entries = &(pml4Entries[pml4Offset]);

        if (!PteIsPresent(pml4Entries))
        {
            // it was never allocated
            // go to next PAGE
            continue;
        }

        tempAddress = PteGetPhysicalAddress(pml4Entries);
        pdptEntries = (PDPT_ENTRY_PD*)PA2VA(tempAddress);

        pdptEntries = &(pdptEntries[pdpteOffset]);

        if (!PteIsPresent(pdptEntries))
        {
            // it was never allocated
            // go to next PAGE
            continue;
        }

        ASSERT(0 == pdptEntries->PageSize);

        tempAddress = PteGetPhysicalAddress(pdptEntries);
        pdEntries = (PD_ENTRY_PT*)PA2VA(tempAddress);

        pdEntries = &(pdEntries[pdeOffset]);
        if (!PteIsPresent(pdEntries))
        {
            // it was never allocated
            // go to next PAGE
            continue;
        }

        ASSERT(0 == pdEntries->PageSize);

        tempAddress = PteGetPhysicalAddress(pdEntries);
        ptEntries = (PT_ENTRY*)PA2VA(tempAddress);


        ptEntries = &(ptEntries[pteOffset]);

        if (!PteIsPresent(ptEntries))
        {
            // it was never allocated
            // go to next PAGE
            continue;
        }
        else
        {
            PHYSICAL_ADDRESS pa = PteGetPhysicalAddress(ptEntries);

            PteUnmap(ptEntries);

            __invlpg((QWORD)VirtualAddress + offset);

            if (ReleaseMemory)
            {
                MmuReleaseMemory(pa, 1);
            }

            /// TODO: signal other processors to invalidate the mapping
        }
    }
}

PTR_SUCCESS
PHYSICAL_ADDRESS
VmmGetPhysicalAddress(
    IN      PML4                    Cr3,
    IN      PVOID                   VirtualAddress
    )
{
    PML4_ENTRY* pml4Entries;
    PDPT_ENTRY_PD* pdptEntries;
    PD_ENTRY_PT* pdEntries;
    PT_ENTRY* ptEntries;

    WORD pageOffset;
    WORD pteOffset;
    WORD pdeOffset;
    WORD pdpteOffset;
    WORD pml4Offset;

    PVOID tempAddress;

    ASSERT( NULL != VirtualAddress );
    ASSERT( IsAddressAligned(VirtualAddress, PAGE_SIZE));

    tempAddress = VirtualAddress;

    // these are the offsets to the corresponding structures
    pml4Offset = MASK_PML4_OFFSET(tempAddress);
    pdpteOffset = MASK_PDPTE_OFFSET(tempAddress);
    pdeOffset = MASK_PDE_OFFSET(tempAddress);
    pteOffset = MASK_PTE_OFFSET(tempAddress);
    pageOffset = MASK_PAGE_OFFSET(tempAddress);

    pml4Entries = (PML4_ENTRY*)PA2VA(Cr3.Pcide.PhysicalAddress << SHIFT_FOR_PHYSICAL_ADDR);
    pml4Entries = &(pml4Entries[pml4Offset]);

    if (!PteIsPresent(pml4Entries))
    {
        // it was never allocated
        return NULL;
    }

    tempAddress = PteGetPhysicalAddress(pml4Entries);
    pdptEntries = (PDPT_ENTRY_PD*)PA2VA(tempAddress);

    pdptEntries = &(pdptEntries[pdpteOffset]);

    if (!PteIsPresent(pdptEntries))
    {
        // it was never allocated
        return NULL;
    }

    ASSERT(0 == pdptEntries->PageSize);

    tempAddress = PteGetPhysicalAddress(pdptEntries);
    pdEntries = (PD_ENTRY_PT*)PA2VA(tempAddress);

    pdEntries = &(pdEntries[pdeOffset]);
    if (!PteIsPresent(pdEntries))
    {
        // it was never allocated
        return NULL;
    }

    if (pdEntries->PageSize)
    {
        // Large 2MB page
        return PtrOffset(PteLargePageGetPhysicalAddress(pdEntries),AddressOffset(VirtualAddress, PAGE_2MB_OFFSET + 1));
    }

    tempAddress = PteGetPhysicalAddress(pdEntries);
    ptEntries = (PT_ENTRY*)PA2VA(tempAddress);


    ptEntries = &(ptEntries[pteOffset]);

    if (!PteIsPresent(ptEntries))
    {
        // it was never allocated
        // go to next PAGE
        return NULL;
    }

    return PteGetPhysicalAddress(ptEntries);
}

_No_competing_thread_
STATUS
VmmPreparePagingData(
    void
    )
{
    QWORD ia32PatValues;
    BOOLEAN bResult;

    ia32PatValues = 0;
    bResult = FALSE;

    ia32PatValues = __readmsr(IA32_PAT);
    bResult = _VmDeterminePatIndices(ia32PatValues,
                                     &m_vmmData.WriteBackIndex,
                                     &m_vmmData.UncacheableIndex
    );
    if (!bResult)
    {
        LOG_ERROR("_VmDeterminePatIndices failed!\n");
        return STATUS_PAT_LAYOUT_NOT_COMPATIBLE;
    }

    LOG("IA32_PAT: 0x%X\n", ia32PatValues);
    LOG("WbIndex: %u\n", m_vmmData.WriteBackIndex);
    LOG("UcIndex: %u\n", m_vmmData.UncacheableIndex);

    return STATUS_SUCCESS;
}

STATUS
VmmSetupPageTables(
    INOUT   PPAGING_DATA            PagingDataWhereToMap,
    OUT     PPAGING_DATA            PagingData,
    IN      PHYSICAL_ADDRESS        BasePhysicalAddress,
    IN      DWORD                   FramesReserved,
    IN      BOOLEAN                 KernelStructures
    )
{
    STATUS status;
    DWORD sizeReservedForPagingStructures;
    PVOID pBaseVirtualAddress;

    ASSERT(PagingDataWhereToMap != NULL);
    ASSERT(PagingData != NULL);
    ASSERT(BasePhysicalAddress != NULL);
    ASSERT(FramesReserved != 0);

    status = STATUS_SUCCESS;
    sizeReservedForPagingStructures = 0;
    pBaseVirtualAddress = (PVOID) PA2VA(BasePhysicalAddress);

    // Current index should start at 1 because at 0 we have the CR3 (PML4 table address)
    PagingData->CurrentIndex = 1;
    PagingData->NumberOfFrames = FramesReserved;
    PagingData->BasePhysicalAddress = BasePhysicalAddress;
    PagingData->KernelSpace = KernelStructures;

    sizeReservedForPagingStructures = FramesReserved * PAGE_SIZE;

    LOG_TRACE_VMM("Will setup paging tables at physical address: 0x%X\n", PagingData->BasePhysicalAddress);
    LOG_TRACE_VMM("BaseAddress: 0x%X\n", pBaseVirtualAddress);
    LOG_TRACE_VMM("Size of paging tables: 0x%x\n", sizeReservedForPagingStructures);

    // 1. We cannot zero the memory before mapping it (because it's not mapped)
    // 2. We cannot zero it after it was mapped because we already have some entries
    // populated to describe the mapped memory.

    // This is not a problem anyway because when we setup a new paging structure we
    // zero it anyway.

    VmmMapMemoryInternal(PagingDataWhereToMap,
                         PagingData->BasePhysicalAddress,
                         sizeReservedForPagingStructures,
                         pBaseVirtualAddress,
                         PAGE_RIGHTS_READWRITE,
                         TRUE,
                         FALSE
                         );
    LOG_TRACE_VMM("VmmMapMemoryInternal finished\n");

    if (PagingDataWhereToMap != PagingData)
    {
        // Zero the newly mapped PML4 if we didn't map it in it's own
        // paging structures - else it would already be populated
        memzero(pBaseVirtualAddress, PAGE_SIZE);
    }

    return status;
}

void
VmmChangeCr3(
    IN      PHYSICAL_ADDRESS        Pml4Base,
    IN_RANGE(PCID_FIRST_VALID_VALUE, PCID_TOTAL_NO_OF_VALUES - 1)
            PCID                    Pcid,
    IN      BOOLEAN                 Invalidate
    )
{
    ASSERT(IsAddressAligned(Pml4Base,PAGE_SIZE));
    ASSERT(PCID_IS_VALID(Pcid));

    // Intel System Programming Manual Vol 3C
    // Section 4.10.4.1 Operations that Invalidate TLBs and Paging-Structure Caches

    // If CR4.PCIDE = 1 and bit 63 of the instruction’s source operand is 0, the instruction invalidates all TLB
    // entries associated with the PCID specified in bits 11:0 of the instruction’s source operand except those for
    // global pages.It also invalidates all entries in all paging - structure caches associated with that PCID.It is not
    // required to invalidate entries in the TLBs and paging - structure caches that are associated with other PCIDs.

    // If CR4.PCIDE = 1 and bit 63 of the instruction’s source operand is 1, the instruction is not required to
    // invalidate any TLB entries or entries in paging - structure caches.
    __writecr3((Invalidate ? 0 : MOV_TO_CR3_DO_NOT_INVALIDATE_PCID_MAPPINGS) | (QWORD)Pml4Base | Pcid);

    /// TODO: This should be broadcast on all CPUs to invalidate their mappings for PCID Pcid
}

_No_competing_thread_
void
VmmInitReservationSystem(
    void
    )
{
    VmReservationSpaceFinishInit(&m_vmmData.VmmReservationSpace);
}

static
void
_VmmMapDescribedRegion(
    IN      PVOID                   BaseAddress,
    IN      PMDL                    Mdl,
    IN      PAGE_RIGHTS             Rights,
    IN_OPT  PPAGING_LOCK_DATA       PagingData
    )
{
    QWORD currentOffset = 0;

    ASSERT(BaseAddress != NULL);
    ASSERT(IsAddressAligned(BaseAddress, PAGE_SIZE));
    ASSERT(Mdl != NULL);

    for (DWORD i = 0;
         i < MdlGetNumberOfPairs(Mdl);
         ++i)
    {
        PHYSICAL_ADDRESS alignedPa;
        QWORD alignedPaSize;
        PMDL_TRANSLATION_PAIR pMdlPair;

        pMdlPair = MdlGetTranslationPair(Mdl, i);

        alignedPa = (PHYSICAL_ADDRESS)AlignAddressLower(pMdlPair->Address, PAGE_SIZE);
        alignedPaSize = AlignAddressUpper(pMdlPair->NumberOfBytes + AddressOffset(pMdlPair->Address, PAGE_SIZE), PAGE_SIZE);

        MmuMapMemoryInternal(alignedPa,
                             alignedPaSize,
                             Rights,
                             PtrOffset(BaseAddress, currentOffset),
                             FALSE,
                             FALSE,
                             PagingData);

        // advance to the next VA->PA physical mapping
        currentOffset += alignedPaSize;
    }
}

PTR_SUCCESS
PVOID
VmmAllocRegionEx(
    IN_OPT  PVOID                   BaseAddress,
    IN      QWORD                   Size,
    IN      VMM_ALLOC_TYPE          AllocType,
    IN      PAGE_RIGHTS             Rights,
    IN      BOOLEAN                 Uncacheable,
    IN_OPT  PFILE_OBJECT            FileObject,
    IN_OPT  PVMM_RESERVATION_SPACE  VaSpace,
    IN_OPT  PPAGING_LOCK_DATA       PagingData,
    IN_OPT  PMDL                    Mdl
    )
{
    PVOID pBaseAddress;
    QWORD alignedSize;
    STATUS status;
    PVMM_RESERVATION_SPACE pVaSpace;
    PHYSICAL_ADDRESS pa;

    ASSERT(Size != 0);

    // We need at least one of these flags set, else we have nothing to do
    ASSERT(IsFlagOn(AllocType, VMM_ALLOC_TYPE_COMMIT | VMM_ALLOC_TYPE_RESERVE));

    // If the MDL is non-NULL we must have the VMM_ALLOC_TYPE_NOT_LAZY set,
    // we don't have an implementation for allocating a VA lazily for memory ranges described by a MDL
    ASSERT(Mdl == NULL || IsBooleanFlagOn(AllocType, VMM_ALLOC_TYPE_NOT_LAZY));

    // We cannot have both the Mdl and the FileObject non-NULL, the region either is already backed up by some physical
    // frames or it is backed up by a file, or it is not backed up by anything
    ASSERT((Mdl == NULL) || (FileObject == NULL));

    status = STATUS_SUCCESS;
    pBaseAddress = NULL;
    pa = NULL;
    alignedSize = 0;

    pVaSpace = (VaSpace == NULL) ? &m_vmmData.VmmReservationSpace : VaSpace;
    ASSERT(pVaSpace != NULL);

    __try
    {
        status = VmReservationSpaceAllocRegion(pVaSpace,
                                               BaseAddress,
                                               Size,
                                               AllocType,
                                               Rights,
                                               Uncacheable,
                                               FileObject,
                                               &pBaseAddress,
                                               &alignedSize);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("VmReservationSpaceAllocRegion", status);
            __leave;
        }
        ASSERT(NULL != pBaseAddress);

        if (IsBooleanFlagOn(AllocType, VMM_ALLOC_TYPE_NOT_LAZY))
        {
            ASSERT(IsBooleanFlagOn(AllocType, VMM_ALLOC_TYPE_COMMIT));

            if (Mdl != NULL)
            {
                // If we received a MDL as a parameter => we already have some initialized physical frames
                // which we want to map to pBaseAddress
                ASSERT(Mdl->ByteCount <= alignedSize);

                _VmmMapDescribedRegion(pBaseAddress, Mdl, Rights, PagingData);
            }
            else
            {
                // This area is not described by an MDL, we need to reserve it now
                // We will ask for a continuous number of frames, we have no way of
                // asking for discontinuous frames - even if we call the function multiple
                // times with 1 frame we probably will receive a continuous mapping because
                // there is noone else requesting physical frames in the meantime

                ASSERT(alignedSize / PAGE_SIZE <= MAX_DWORD);
                DWORD noOfFrames = (DWORD)(alignedSize / PAGE_SIZE);

                pa = PmmReserveMemory(noOfFrames);
                if (NULL == pa)
                {
                    LOG_ERROR("PmmReserverMemory failed!\n");
                    __leave;
                }

                MmuMapMemoryInternal(pa,
                                     alignedSize,
                                     Rights,
                                     pBaseAddress,
                                     TRUE,
                                     Uncacheable,
                                     PagingData
                );

                // Check if the mapping is backed up by a file
                if (FileObject != NULL)
                {
                    QWORD fileOffset;
                    QWORD bytesRead;

                    // make sure we read from the start of the file
                    fileOffset = 0;

                    status = IoReadFile(FileObject,
                                        alignedSize,
                                        &fileOffset,
                                        pBaseAddress,
                                        &bytesRead);
                    if (!SUCCEEDED(status))
                    {
                        LOG_FUNC_ERROR("IoReadFile", status);
                        __leave;
                    }

                    LOG_TRACE_VMM("File size is 0x%X, bytes read are 0x%X\n", alignedSize, bytesRead);

                    // zero the rest of the file
                    ASSERT(alignedSize - bytesRead <= MAX_DWORD);

                    // memzero the rest of the allocation (in case the size of the allocation is greater than the
                    // size of the file)
                    memzero(PtrOffset(pBaseAddress, bytesRead), (DWORD)(alignedSize - bytesRead));
                }
            }
        }

    }
    __finally
    {
        if (!SUCCEEDED(status))
        {
            LOG_ERROR("We failed with status 0x%x\n", status);

            if (pBaseAddress != NULL)
            {
                PVOID pAlignedAddress;

                VmReservationSpaceFreeRegion(pVaSpace,
                                             pBaseAddress,
                                             0,
                                             VMM_FREE_TYPE_RELEASE,
                                             &pAlignedAddress,
                                             &alignedSize
                                             );
                ASSERT(pAlignedAddress == pBaseAddress);
                pBaseAddress = NULL;

                if (pa != NULL)
                {
                    MmuUnmapMemoryEx(pAlignedAddress, (DWORD) alignedSize, TRUE, PagingData);
                    pa = NULL;
                }
            }
            ASSERT(pa == NULL);
        }
        else
        {
            if (IsBooleanFlagOn(Rights, PAGE_RIGHTS_EXECUTE | PAGE_RIGHTS_WRITE))
            {
                LOG_WARNING("Successfully mapped W+X memory at 0x%X of size 0x%X\n",
                            pBaseAddress, alignedSize);
            }
        }
    }

    return pBaseAddress;
}

void
VmmFreeRegionEx(
    IN      PVOID                   Address,
    _When_(VMM_FREE_TYPE_RELEASE == FreeType, _Reserved_)
    _When_(VMM_FREE_TYPE_RELEASE != FreeType, IN)
            QWORD                   Size,
    IN      VMM_FREE_TYPE           FreeType,
    IN      BOOLEAN                 Release,
    IN_OPT  PVMM_RESERVATION_SPACE  VaSpace,
    IN_OPT  PPAGING_LOCK_DATA       PagingData
    )
{
    PVOID alignedAddress;
    QWORD alignedSize;

    ASSERT(Address != NULL);
    ASSERT(IsBooleanFlagOn( FreeType, VMM_FREE_TYPE_RELEASE ) ^ IsBooleanFlagOn(FreeType, VMM_FREE_TYPE_DECOMMIT ));
    ASSERT((IsBooleanFlagOn(FreeType, VMM_FREE_TYPE_RELEASE) && (Size == 0))
           || (IsBooleanFlagOn(FreeType, VMM_FREE_TYPE_DECOMMIT) && (Size != 0)));

    alignedAddress = NULL;
    alignedSize = 0;

    VmReservationSpaceFreeRegion((VaSpace == NULL) ? &m_vmmData.VmmReservationSpace : VaSpace,
                                 Address,
                                 Size,
                                 FreeType,
                                 &alignedAddress,
                                 &alignedSize);

    if (IsFlagOn(FreeType, VMM_FREE_TYPE_DECOMMIT | VMM_FREE_TYPE_RELEASE ))
    {
        ASSERT( NULL != alignedAddress);
        ASSERT( 0 != alignedSize);

        LOG_TRACE_VMM("Will free %U bytes of memory starting from VA: 0x%X\n", alignedSize, alignedAddress );

        // un-map memory
        // this must be called without the reservation lock taken
        MmuUnmapMemoryEx(alignedAddress,
                         alignedSize,
                         Release,
                         PagingData);
    }
}

BOOLEAN
VmmSolvePageFault(
    IN      PVOID                   FaultingAddress,
    IN      PAGE_RIGHTS             RightsRequested,
    IN      PPAGING_LOCK_DATA       PagingData
    )
{
    BOOLEAN bSolvedPageFault;
    BOOLEAN bAccessValid;
    STATUS status;
    PPCPU pCpu;
    PAGE_RIGHTS pageRights;
    BOOLEAN uncacheable;
    PFILE_OBJECT pBackingFile;
    QWORD fileOffset;
    BOOLEAN bKernelAddress;
    QWORD bytesReadFromFile;

    ASSERT(INTR_OFF == CpuIntrGetState());
    ASSERT(PagingData != NULL);

    // we will certainly not use the first virtual page of memory
    if (NULL == (PVOID) AlignAddressLower(FaultingAddress, PAGE_SIZE))
    {
        return FALSE;
    }

    bKernelAddress = _VmIsKernelAddress(FaultingAddress);

    if (bKernelAddress && !PagingData->Data.KernelSpace)
    {
        LOG_TRACE_VMM("User code should not access KM pages!\n");
        return FALSE;
    }
    else if (!bKernelAddress && PagingData->Data.KernelSpace)
    {
        LOG_ERROR("Kernel code should not access UM pages!\n");
        return FALSE;
    }

    bSolvedPageFault = FALSE;
    bAccessValid = FALSE;
    status = STATUS_SUCCESS;
    pCpu = GetCurrentPcpu();
    pageRights = 0;
    uncacheable = FALSE;
    pBackingFile = NULL;
    fileOffset = 0;
    bytesReadFromFile = 0;

    // See if the VA is already committed and retrieve its description (the page rights with which it was mapped,
    // cacheability and for memory backed by files the FILE_OBJECT and corresponding offset in file)
    bAccessValid = VmReservationCanAddressBeAccessed(_VmmRetrieveReservationSpaceForAddress(FaultingAddress),
                                                     FaultingAddress,
                                                     RightsRequested,
                                                     &pageRights,
                                                     &uncacheable,
                                                     &pBackingFile,
                                                     &fileOffset);

    __try
    {
        if (bAccessValid)
        {
            PHYSICAL_ADDRESS pa;
            PVOID alignedAddress;

            // solve #PF

            // 1. Reserve one frame of physical memory
            pa = PmmReserveMemory(1);
            ASSERT(NULL != pa);

            alignedAddress = (PVOID)AlignAddressLower(FaultingAddress, PAGE_SIZE);

            // 2. Map the aligned faulting address to the newly acquired physical frame
            MmuMapMemoryInternal(pa,
                                 PAGE_SIZE,
                                 pageRights,
                                 alignedAddress,
                                 TRUE,
                                 uncacheable,
                                 PagingData
                                 );

            // 3. If the virtual address is backed by a file read its contents
            if (pBackingFile != NULL)
            {
                LOGL("Will read data from file 0x%X and offset 0x%X\n", pBackingFile, fileOffset);

                status = IoReadFile(pBackingFile,
                                    PAGE_SIZE,
                                    &fileOffset,
                                    alignedAddress,
                                    &bytesReadFromFile);
                if (!SUCCEEDED(status))
                {
                    LOG_FUNC_ERROR("IoReadFile", status);
                    __leave;
                }

                LOGL("Bytes read 0x%X\n", bytesReadFromFile);
                ASSERT(bytesReadFromFile <= PAGE_SIZE);
            }

            // 4. Zero the rest of the memory (in case the remaining file size was smaller than a page
            /// TODO: check if this is really necessary (we have a ZERO worker thread already!)
            if (bytesReadFromFile != PAGE_SIZE)
            {
                /// TODO: Check if we really need to remove the WP (I'd rather not do this)
                /// According to the Intel manual the WP flag has nothing to do with accessing UM pages
                /// It is more generic, if WP is set => supervisor accesses can write to any virtual address
                /// even if it is read-only
                __writecr0(__readcr0() & ~CR0_WP);
                memzero(PtrOffset(alignedAddress, bytesReadFromFile), PAGE_SIZE - (DWORD)bytesReadFromFile);
                __writecr0(__readcr0() | CR0_WP);
            }

            if (NULL != pCpu)
            {
                // solved another page fault :)
                pCpu->PageFaults = pCpu->PageFaults + 1;
            }
            bSolvedPageFault = TRUE;
        }
    }
    __finally
    {

    }

    return bSolvedPageFault;
}

PVMM_RESERVATION_SPACE
VmmRetrieveReservationSpaceForSystemProcess(
    void
    )
{
    return &m_vmmData.VmmReservationSpace;
}

STATUS
VmmCreateVirtualAddressSpace(
    OUT_PTR struct _VMM_RESERVATION_SPACE** ReservationSpace,
    IN      QWORD                           ReservationMetadataSize,
    IN      PVOID                           StartOfVirtualAddressSpace
    )
{
    STATUS status;
    PVMM_RESERVATION_SPACE pProcessVaHeader;
    PVOID pReservationMetadataStart;

    if (ReservationSpace == NULL)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (ReservationSpace == 0)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    if (StartOfVirtualAddressSpace == NULL)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    status = STATUS_SUCCESS;
    pProcessVaHeader = NULL;
    pReservationMetadataStart = NULL;

    __try
    {
        pProcessVaHeader = ExAllocatePoolWithTag(PoolAllocateZeroMemory, sizeof(VMM_RESERVATION_SPACE), HEAP_PROCESS_TAG, 0);
        if (pProcessVaHeader == NULL)
        {
            status = STATUS_INSUFFICIENT_MEMORY;
            LOG_FUNC_ERROR_ALLOC("ExAllocatePoolWithTag", sizeof(VMM_RESERVATION_SPACE));
            __leave;
        }

        pReservationMetadataStart = VmmAllocRegionEx(NULL,
                                                     ReservationMetadataSize,
                                                     VMM_ALLOC_TYPE_RESERVE | VMM_ALLOC_TYPE_COMMIT,
                                                     PAGE_RIGHTS_READWRITE,
                                                     FALSE,
                                                     NULL,
                                                     NULL,
                                                     NULL,
                                                     NULL);
        if (pReservationMetadataStart == NULL)
        {
            status = STATUS_INSUFFICIENT_MEMORY;
            LOG_FUNC_ERROR_ALLOC("VmmAllocRegionEx", ReservationMetadataSize);
            __leave;
        }

        VmReservationSpaceInit(pReservationMetadataStart,
                               StartOfVirtualAddressSpace,
                               ReservationMetadataSize,
                               pProcessVaHeader);
        VmReservationSpaceFinishInit(pProcessVaHeader);
    }
    __finally
    {
        if (!SUCCEEDED(status))
        {
            if (pReservationMetadataStart != NULL)
            {
                VmmFreeRegion(pReservationMetadataStart, 0, VMM_FREE_TYPE_RELEASE);
                pReservationMetadataStart = NULL;
            }

            if (pProcessVaHeader != NULL)
            {
                ExFreePoolWithTag(pProcessVaHeader, HEAP_PROCESS_TAG);
                pProcessVaHeader = NULL;
            }
        }
        else
        {
            *ReservationSpace = pProcessVaHeader;
        }
    }

    return status;
}

void
VmmDestroyVirtualAddressSpace(
    _Pre_valid_ _Post_ptr_invalid_
        struct _VMM_RESERVATION_SPACE*      ReservationSpace
    )
{
    ASSERT(ReservationSpace != NULL);

    if (ReservationSpace->ReservationList != NULL)
    {
        /// TODO: go through each reservation belonging to the process and
        /// release all the physical frames used (else we have physical memory leaks :) - not fun)

        VmmFreeRegion(ReservationSpace->ReservationList, 0, VMM_FREE_TYPE_RELEASE);
        ReservationSpace->ReservationList = NULL;
    }

    ExFreePoolWithTag(ReservationSpace, HEAP_PROCESS_TAG);
}

STATUS
VmmIsBufferValid(
    IN          PVOID                               Buffer,
    IN          QWORD                               BufferSize,
    IN          PAGE_RIGHTS                         RightsRequested,
    IN          PVMM_RESERVATION_SPACE              ReservationSpace,
    IN          BOOLEAN                             KernelAccess
    )
{
    STATUS status;
    PAGE_RIGHTS rightsGranted;

    ASSERT(Buffer != NULL);
    ASSERT(BufferSize != 0);
    ASSERT(ReservationSpace != NULL);

    if (!KernelAccess && _VmIsKernelRange(Buffer, BufferSize))
    {
        // We shouldn't be accessing from UM kernel buffers :)
        LOG_ERROR("Usermode code should not be accessing kernel memory between 0x%X -> 0x%X\n",
                  BufferSize, PtrOffset(BufferSize, BufferSize - 1));
        return STATUS_MEMORY_PREVENTS_USERMODE_ACCESS;
    }

    status = VmReservationReturnRightsForAddress(ReservationSpace,
                                                 Buffer,
                                                 BufferSize,
                                                 &rightsGranted);
    if (!SUCCEEDED(status))
    {
        LOG_TRACE_VMM("VmReservationReturnRightsForAddress", status);
        return status;
    }

    if (!IsBooleanFlagOn(rightsGranted, RightsRequested))
    {
        LOG_ERROR("The VA space has paging rights of 0x%x, but we requested 0x%x. Will certainly not allow!\n",
                  rightsGranted, RightsRequested);
        return STATUS_MEMORY_INSUFFICIENT_ACCESS_RIGHTS;
    }

    return STATUS_SUCCESS;
}

static
void
_VmSetupPagingStructure(
    IN      PPAGING_DATA            PagingData,
    IN      PVOID                   PagingStructure
    )
{
    PHYSICAL_ADDRESS physicalAddr;
    PTE_MAP_FLAGS flags = { 0 };

    ASSERT(NULL != PagingData);
    ASSERT(NULL != PagingStructure);

    // request 1 frame of physical memory
    physicalAddr = _VmRetrieveNextPhysicalAddressForPagingStructure(PagingData);
    flags.Writable = TRUE;
    flags.Executable = TRUE;
    flags.PagingStructure = TRUE;
    flags.UserAccess = !PagingData->KernelSpace;

    // for paging structure PA2VA can always be used :)
    PteMap(PagingStructure, physicalAddr, flags);

    __invlpg((PVOID)PA2VA(physicalAddr));

    // Zero the current paging structure entry => we cannot get stray memory accesses
    memzero((PVOID)PA2VA(physicalAddr), PAGE_SIZE);
}

static
BOOL_SUCCESS
BOOLEAN
_VmDeterminePatIndices(
    IN      QWORD                   PatValues,
    OUT     PBYTE                   WbIndex,
    OUT     PBYTE                   UcIndex
    )
{
    BOOLEAN bFoundWb;
    BOOLEAN bFoundUc;
    PBYTE patArrayValue;

    ASSERT( NULL != WbIndex );
    ASSERT( NULL != UcIndex );

    bFoundWb = bFoundUc = FALSE;
    patArrayValue = (PBYTE) &PatValues;

    for (BYTE i = 0; i < sizeof(QWORD); ++i)
    {
        if (MemoryCachingStrongUncacheable == patArrayValue[i] )
        {
            if (!bFoundUc)
            {
                bFoundUc = TRUE;
                *UcIndex = i;
            }
        }
        else if (MemoryCachingWriteBack == patArrayValue[i])
        {
            if( !bFoundWb)
            {
                bFoundWb = TRUE;
                *WbIndex = i;
            }
        }
    }

    return bFoundWb && bFoundUc;
}