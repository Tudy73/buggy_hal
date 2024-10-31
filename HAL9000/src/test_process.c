#include "test_common.h"
#include "test_process.h"
#include "process.h"
#include "iomu.h"

const PROCESS_TEST PROCESS_TESTS[] =
{
    // arguments
    { "TestUserArgsNone", "Args", NULL},
    { "TestUserArgsOne", "Args", "Argument"},
    { "TestUserArgsMany", "Args", "Johnny is a good kid"},
    { "TestUserArgsAll", "Args", "a b c d e f g h i j k l m n o p r s t u v q x y z"},

    // bad-actions
    { "BadJumpKernel", "Badjum~1", NULL},
    { "BadJumpNoncanonical", "Badjum~2", NULL},
    { "BadJumpNull", "Badjum~3", NULL},

    { "BadReadIoPort", "Badrea~1", NULL},
    { "BadReadKernel", "Badrea~2", NULL},
    { "BadReadMsr", "Badrea~3", NULL},
    { "BadReadNonCanonical", "Badrea~4", NULL},
    { "BadReadNull", "BA05C5~1", NULL},

    { "BadWriteIoPort", "Badwri~1", NULL},
    { "BadWriteKernel", "Badwri~2", NULL},
    { "BadWriteMsr", "Badwri~3", NULL},
    { "BadWriteNonCanonical", "Badwri~4", NULL},
    { "BadWriteNull", "BAAEFD~1", NULL},

    // file-syscalls
    { "FileCloseBad", "Filecl~1", NULL},
    { "FileCloseNormal", "Filecl~2", NULL},
    { "FileCloseStdout", "Filecl~3", NULL},
    { "FileCloseTwice", "Filecl~4", NULL},

    { "FileCreateBadPointer", "Filecr~1", NULL},
    { "FileCreateEmptyPath", "Filecr~2", NULL},
    { "FileCreateExistent", "Filecr~3", NULL},
    { "FileCreateMissing", "Filecr~4", NULL},
    { "FileCreateNormal", "FI3ACB~1", NULL},
    { "FileCreateNull", "FIB562~1", NULL},
    { "FileCreateTwice", "FI8E28~1", NULL},

    { "FileReadBadHandle", "Filere~1", NULL},
    { "FileReadBadPointer", "Filere~2", NULL},
    { "FileReadKernel", "Filere~3", NULL},
    { "FileReadNormal", "Filere~4", NULL},
    { "FileReadStdout", "FI031C~1", NULL},
    { "FileReadZero", "FI124A~1", NULL},

    // process-syscalls
    { "ProcessCloseFile", "Proces~1", NULL},
    { "ProcessCloseNormal", "Proces~2", NULL},
    { "ProcessCloseParentHandle", "Proces~3", NULL},
    { "ProcessCloseTwice", "Proces~4", NULL},

    { "ProcessCreateBadPointer", "PR07C5~1", NULL},
    { "ProcessCreateMissingFile", "PR3479~1", NULL},
    { "ProcessCreateMultiple", "PRD622~1", NULL},
    { "ProcessCreateOnce", "PR0947~1", NULL},
    { "ProcessCreateWithArguments", "PR9005~1", NULL},

    { "ProcessExit", "PRC65A~1", NULL},
    { "ProcessGetPid", "PR41E3~1", NULL},

    { "ProcessWaitBadHandle", "PR9843~1", NULL},
    { "ProcessWaitClosedHandle", "PRFC97~1", NULL},
    { "ProcessWaitNormal", "PRCDF7~1", NULL},
    { "ProcessWaitTerminated", "PR2A4D~1", NULL},

    // thread-syscalls
    { "ThreadCloseTwice", "Thread~1", NULL},

    { "ThreadCreateBadPointer", "Thread~2", NULL},
    { "ThreadCreateMultiple", "Thread~3", NULL},
    { "ThreadCreateOnce", "Thread~4", NULL},
    { "ThreadCreateWithArguments", "TH38A7~1", NULL},

    { "ThreadExit", "TH2D1A~1", NULL},
    { "ThreadGetTid", "THFB9D~1", NULL},

    { "ThreadWaitBadHandle", "THC1A9~1", NULL},
    { "ThreadWaitClosedHandle", "TH9BE5~1", NULL},
    { "ThreadWaitNormal", "THA76F~1", NULL},
    { "ThreadWaitTerminated", "THE738~1", NULL},
};

const DWORD PROCESS_TOTAL_NO_OF_TESTS = ARRAYSIZE(PROCESS_TESTS);


void
TestProcessFunctionality(
    IN      PROCESS_TEST*               ProcessTest
    )
{
    STATUS status;
    STATUS terminationStatus;
    PPROCESS pProcess;
    char fullPath[MAX_PATH];
    const char* pSystemPartition;

    pSystemPartition = IomuGetSystemPartitionPath();

    LOG_TEST_LOG("Test [%s] START!\n", ProcessTest->TestName);

    __try
    {
        if (pSystemPartition == NULL)
        {
            LOG_ERROR("Cannot run user tests without knowing the system partition!\n");
            __leave;
        }

        snprintf(fullPath, MAX_PATH,
                 "%s%s\\%s.exe", pSystemPartition, "APPLIC~1",
                 ProcessTest->ProcessName);

        printf("Full path is [%s]\n", fullPath);

        status = ProcessCreate(fullPath,
                               ProcessTest->ProcessCommandLine,
                               &pProcess);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("ProcessCreate", status);
            __leave;
        }

        ProcessWaitForTermination(pProcess, &terminationStatus);

        ProcessCloseHandle(pProcess);
        pProcess = NULL;
    }
    __finally
    {
        LOG_TEST_LOG("Test [%s] END!\n", ProcessTest->TestName);
    }
}

void
TestAllProcessFunctionalities(
    void
    )
{
    for (DWORD i = 0; i < PROCESS_TOTAL_NO_OF_TESTS; ++i)
    {
        TestProcessFunctionality(&PROCESS_TESTS[i]);
    }
}
