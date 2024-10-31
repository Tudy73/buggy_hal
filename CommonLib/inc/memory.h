#pragma once

//******************************************************************************
// Function:        memset
// Description:     Sets bytes in a memory area to a value.
// Returns:         void
// Parameter:       OUT PVOID address    - Address at which to set memory
// Parameter:       IN  BYTE value       - Value which to set
// Parameter:       IN  DWORD size       - Number of bytes to set
//******************************************************************************
_At_buffer_( address, i, size, _Post_satisfies_( ((PBYTE)address)[i] == value ))
void
memset( 
    OUT_WRITES_BYTES_ALL(size)  PVOID address, 
    IN                          BYTE value, 
    IN                          DWORD size 
    );

#define memzero(addr,size)      memset((addr),0,(size))

//******************************************************************************
// Function:     memcpy
// Description:  This function does not guarantee proper handling of overlapped
//               memory regions. Use memmove in those cases.
// Returns:      void
// Parameter:    OUT PVOID Destination
// Parameter:    IN PVOID Source
// Parameter:    IN QWORD Count
//******************************************************************************
_At_buffer_(Destination,i, Count,
            _Post_satisfies_(((PBYTE)Destination)[i] == ((PBYTE)Source)[i]))
void
memcpy(
    OUT_WRITES_BYTES_ALL(Count) PVOID   Destination,
    IN_READS(Count)             PVOID   Source,
    IN                          QWORD   Count
    );

//******************************************************************************
// Function:     memmove
// Description:  Executes slower than memcpy but it can be used for overlapped
//               memory regions.
// Returns:      void
// Parameter:    OUT PVOID Destination
// Parameter:    IN PVOID Source
// Parameter:    IN QWORD Count
//******************************************************************************
_At_buffer_(Destination, i, Count,
            _Post_satisfies_(((PBYTE)Destination)[i] == ((PBYTE)Source)[i]))
void
memmove(
    OUT_WRITES_BYTES_ALL(Count) PVOID   Destination,
    IN_READS(Count)             PVOID   Source,
    IN                          QWORD   Count
    );


//******************************************************************************
// Function:        memcmp
// Description:     Compares two memory regions.
// Returns:         int - 0 if equal, not 0 if different
// Parameter:       IN PVOID ptr1 - pointer to first memory region
// Parameter:       IN PVOID ptr2 - pointer to second memory region
// Parameter:       IN DWORD size - size of region to compare
//******************************************************************************
int 
memcmp(
    IN_READS_BYTES(size)    PVOID ptr1,
    IN_READS_BYTES(size)    PVOID ptr2,
    IN                      DWORD size
    );


//******************************************************************************
// Function:     memscan
// Description:  Scans a memory region in search for a different value than the
//               one received as input.
// Returns:      int - Index of first byte found different from value. If the
//               whole buffer contains only value then the return value will be
//               the size of the buffer.
// Parameter:    PVOID buffer
// Parameter:    IN DWORD size
// Parameter:    IN BYTE value
//******************************************************************************
int
memscan(
    IN_READS_BYTES(size)    PVOID buffer,
    IN                      DWORD size,
    IN                      BYTE  value
    );