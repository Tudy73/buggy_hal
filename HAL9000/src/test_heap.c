#include "test_common.h"
#include "test_heap.h"
#include "heap.h"

static
void
TestHeapAllocation(
    IN      DWORD       HeapAllocationSize,
    IN      DWORD       NumberOfHeapAllocations,
    IN      DWORD       HeapAlignment
    )
{
    DWORD i;
    PVOID pointers[TEST_MAX_NO_OF_HEAP_ALLOCATIONS];
    PVOID firstAllocationPointer = NULL;
    PVOID allocationPointerAfterDealloc = NULL;

    ASSERT(NumberOfHeapAllocations <= TEST_MAX_NO_OF_HEAP_ALLOCATIONS);

    for (i = 0; i < NumberOfHeapAllocations; ++i)
    {
        pointers[i] = ExAllocatePoolWithTag(PoolAllocatePanicIfFail, HeapAllocationSize, HEAP_TEST_TAG, HeapAlignment);
        ASSERT(IsAddressAligned(pointers[i], HeapAlignment));
        if (0 == i)
        {
            firstAllocationPointer = pointers[i];
        }

        LOGL("[%d]Allocation succeeded: %X\n", i, pointers[i]);
    }

    LOGL("Tests for addresses aligned at %d bytes succeeded\n", HeapAlignment);

    for (i = 1; i <= NumberOfHeapAllocations; ++i)
    {
        ExFreePoolWithTag(pointers[NumberOfHeapAllocations - i], HEAP_TEST_TAG);
        pointers[NumberOfHeapAllocations - i] = NULL;
    }

    allocationPointerAfterDealloc = ExAllocatePoolWithTag(PoolAllocatePanicIfFail, HeapAllocationSize, HEAP_TEST_TAG, HeapAlignment);
    ASSERT(firstAllocationPointer == allocationPointerAfterDealloc);

    LOGL("Heap de-allocation does not leak memory for %d bytes alignment\n", HeapAlignment);

    ExFreePoolWithTag(allocationPointerAfterDealloc, HEAP_TEST_TAG);
}

void
TestHeapFunctions(
    IN          DWORD           NoOfAllocations,
    IN          DWORD           AllocationSize
    )
{
    TestHeapAllocation(AllocationSize, NoOfAllocations, HEAP_DEFAULT_ALIGNMENT);

    LOGL("Default alignment tests succeeded\n");

    TestHeapAllocation(AllocationSize, NoOfAllocations, 1);

    LOGL("1 byte alignment tests succeeded\n");

    TestHeapAllocation(AllocationSize, NoOfAllocations, PAGE_SIZE);

    LOGL("PAGE-size alignment tests succeeded\n");
}