#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"


STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    UM_HANDLE hThread;
    STATUS status;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    hThread = UM_INVALID_HANDLE_VALUE;

    __try
    {
        status = UmThreadCreate(NULL, NULL, &hThread);
        if (SUCCEEDED(status))
        {
            LOG_ERROR("UmThreadCreate succeeded, but it should have failed because of NULL function\n");
        }
    }
    __finally
    {

    }

    return STATUS_SUCCESS;
}