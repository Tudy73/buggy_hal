#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

#define EXPECTED_BUFFER "UserModeApplications:Applications"

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    STATUS status;
    UM_HANDLE handle;
    QWORD bytesRead;
    BYTE bufferRead[sizeof(EXPECTED_BUFFER)];

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    handle = UM_INVALID_HANDLE_VALUE;
    bufferRead[sizeof(EXPECTED_BUFFER)-1] = '\0';

    __try
    {
        status = SyscallFileCreate("HAL9000.ini",
                                   sizeof("HAL9000.ini"),
                                   FALSE,
                                   FALSE,
                                   &handle);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallFileCreate", status);
            __leave;
        }

        status = SyscallFileRead(handle,
                                 bufferRead,
                                 sizeof(EXPECTED_BUFFER) - 1,
                                 &bytesRead);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallFileRead", status);
            __leave;
        }

        if (bytesRead != sizeof(EXPECTED_BUFFER) - 1)
        {
            LOG_ERROR("We expected to read %U bytes, while we actually read %U!\n",
                      sizeof(EXPECTED_BUFFER) - 1, bytesRead);
            __leave;
        }

        if (memcmp(bufferRead, EXPECTED_BUFFER, (DWORD) bytesRead) != 0)
        {
            LOG_ERROR("Expected buffer is [%s], buffer read is [%s]\n",
                      EXPECTED_BUFFER, bufferRead);
            __leave;
        }
    }
    __finally
    {
        if (handle != UM_INVALID_HANDLE_VALUE)
        {
            status = SyscallFileClose(handle);
            handle = UM_INVALID_HANDLE_VALUE;
        }
    }

    return STATUS_SUCCESS;
}