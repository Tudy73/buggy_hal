#include "HAL9000.h"

#include "multiboot.h"
#include "dmp_multiboot.h"

#include "system.h"
#include "cmd_interpreter.h"
#include "common_lib.h"
#include "hal_assert.h"
#include "synch.h"
#include "cpumu.h"

//#define TST

#ifdef TST
#include "test_common.h"
#include "keyboard.h"
#endif

void
Entry64(
    IN  int                 argc,
    IN  ASM_PARAMETERS*     argv
    )
{
    STATUS status;
    COMMON_LIB_INIT initSettings;

    CHECK_STACK_ALIGNMENT;

    status = STATUS_SUCCESS;
    memzero(&initSettings, sizeof(COMMON_LIB_INIT));

    initSettings.Size = sizeof(COMMON_LIB_INIT);
    initSettings.AssertFunction = Hal9000Assert;
    ///HALT HERE
    CpuMuPreinit();

    status = CpuMuSetMonitorFilterSize(sizeof(MONITOR_LOCK));
    initSettings.MonitorSupport = SUCCEEDED(status);
    ///HALT HERE
    status = CommonLibInit(&initSettings);
    if (!SUCCEEDED(status))
    {
        // not good lads
        __halt();
    }

    ASSERT_INFO(1 == argc, "We are always expecting a single parameter\n");
    ASSERT_INFO(NULL != argv, "We are expecting a non-NULL pointer\n");

    gVirtualToPhysicalOffset = argv->VirtualToPhysicalOffset;
    ///HALT HERE
    SystemPreinit();///PROBLEM HERE
    ///HALT HERE
    DumpParameters(argv);
    ///HALT HERE
    status = SystemInit(argv);///PROBLEMS HERE  
    ASSERT(SUCCEEDED(status));

    LOGL("InitSystem executed successfully\n");

#ifdef TST
    TestRunAllFunctional();
    TestRunAllPerformance();
    KeyboardResetSystem();
#endif

    // will run until exit command
    CmdRun();

    // uninitialize system
    // at the end of this routine interrupts should be disabled
    SystemUninit();

    __halt();
}