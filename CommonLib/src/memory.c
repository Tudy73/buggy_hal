#include "common_lib.h"
#include "memory.h"

extern void CpuClearDirectionFlag();

_At_buffer_( address, i, size, _Post_satisfies_( ((PBYTE)address)[i] == value ))
void
memset(
    OUT_WRITES_BYTES_ALL(size)  PVOID address,
    IN                          BYTE value,
    IN                          DWORD size
    )
{
    DWORD i;

    // validate parameters
    // size validation is done implicitly in the for loop
    if (NULL == address)
    {
        return;
    }

    for (i = 0; i < size; ++i)
    {
        ((BYTE*)address)[i] = value;
    }
}

_At_buffer_(Destination, i, Count,
            _Post_satisfies_(((PBYTE)Destination)[i] == ((PBYTE)Source)[i]))
void
memcpy(
    OUT_WRITES_BYTES_ALL(Count) PVOID   Destination,
    IN_READS(Count)             PVOID   Source,
    IN                          QWORD   Count
    )
{
    PBYTE dst;
    PBYTE src;
    QWORD alignedCount;
    QWORD unalignedCount;

    if( (NULL == Destination) || (NULL == Source))
    {
        return;
    }

    unalignedCount = Count & 0x7;
    alignedCount = Count - unalignedCount;

    dst = (PBYTE)Destination;
    src = (PBYTE)Source;

    if (unalignedCount & 0x4)
    {
        *((PDWORD)dst) = *((PDWORD)src);
        dst = dst + sizeof(DWORD);
        src = src + sizeof(DWORD);
    }

    if (unalignedCount & 0x2)
    {
        *((PWORD)dst) = *((PWORD)src);
        dst = dst + sizeof(WORD);
        src = src + sizeof(WORD);
    }
    
    if (unalignedCount & 0x1)
    {
        *((PBYTE)dst) = *((PBYTE)src);
        dst = dst + sizeof(BYTE);
        src = src + sizeof(BYTE);
    }

    if (0 != alignedCount)
    {
        ASSERT(IsAddressAligned(alignedCount, sizeof(QWORD)));

        dst = (PBYTE)Destination + unalignedCount;
        src = (PBYTE)Source + unalignedCount;

        CpuClearDirectionFlag();

        __movsq(dst, src, alignedCount / sizeof(QWORD));
    }
}

_At_buffer_(Destination, i, Count,
            _Post_satisfies_(((PBYTE)Destination)[i] == ((PBYTE)Source)[i]))
void
memmove(
    OUT_WRITES_BYTES_ALL(Count) PVOID   Destination,
    IN_READS(Count)             PVOID   Source,
    IN                          QWORD   Count
    )
{
    PBYTE dst;
    PBYTE src;
    QWORD i;

    if ((NULL == Destination) || (NULL == Source))
    {
        return;
    }

    dst = Destination;
    src = Source;

    for (i = 0; i < Count; ++i)
    {
        dst[i] = src[i];
    }
}

int
memcmp(
    IN_READS_BYTES(size)    PVOID ptr1,
    IN_READS_BYTES(size)    PVOID ptr2,
    IN                      DWORD size
    )
{
    INT64 i = size;
    PBYTE p1;
    PBYTE p2;

    if ((NULL == ptr1) || (NULL == ptr2))
    {
        // TODO: what is the best result to return?
        return size;
    }


    p1 = (PBYTE)ptr1;
    p2 = (PBYTE)ptr2;

    for (i = 0; i < size; ++i)
    {
        if (p1[i] != p2[i])
        {
            return p1[i] - p2[i];
        }
    }

    return 0;
}

int
memscan(
    IN_READS_BYTES(size)    PVOID buffer,
    IN                      DWORD size,
    IN                      BYTE  value
    )
{
    DWORD i;
    PBYTE pData;

    if (NULL == buffer)
    {
        return 0;
    }

    pData = (PBYTE)buffer;

    for (i = 0; i < size; ++i)
    {
        if (pData[i] != value)
        {
            // game over
            return i;
        }
    }

    return i;
}