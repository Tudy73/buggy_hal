#pragma once

// basic includes
#include "base.h"
#include "sal_interface.h"
#include "status.h"
#include "va_list.h"
#include "intutils.h"
#include "memory.h"
#include "assert.h"

#pragma pack(push,16)
typedef struct _COMMON_LIB_INIT
{
    // size of the COMMON_LIB_INIT structure
    // this is used to avoid incompatibilities between
    // the library and the application using it
    DWORD                       Size;

    // function to be used for ASSERTs
    PFUNC_AssertFunction        AssertFunction;

    BOOLEAN                     MonitorSupport;
} COMMON_LIB_INIT, *PCOMMON_LIB_INIT;
#pragma pack(pop)

//******************************************************************************
// Function:     CommonLibInit
// Description:  Initializes the library.
// Returns:      STATUS
// Parameter:    IN PCOMMON_LIB_INIT InitSettings - Initial library settings
//******************************************************************************
STATUS
CommonLibInit(
    IN      PCOMMON_LIB_INIT        InitSettings
    );