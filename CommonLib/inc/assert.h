#pragma once

#include "string.h"

//******************************************************************************
// Function:     FUNC_AssertFunction
// Description:  Function which is responsible for handling a system assertion
//               failure.
// Returns:      void
// Parameter:    IN_Z            char*           Message
//******************************************************************************
typedef
void
(__cdecl FUNC_AssertFunction)(
    IN_Z            char*           Message
    );

typedef FUNC_AssertFunction*        PFUNC_AssertFunction;

/// These ASSERT* functions cannot be used before
/// AssertSetFunction is called

// The ASSERT macros validate a specified condition, in case the
// condition does not hold the registered FUNC_AssertFunction is called
#define ASSERT(Cond)                ASSERT_INFO((Cond),"")

#define ASSERT_INFO(Cond,Msg,...)   if((Cond)){} else                                                                                                    \
                                    {                                                                                                                    \
                                        AssertInfo( "[ASSERT][%s][%d]Condition: (" ## #Cond ## ") failed\n" ##Msg, strrchr(__FILE__, '\\') + 1, __LINE__, __VA_ARGS__ );      \
                                    }

#define NOT_REACHED                 __pragma(warning(suppress: 4127)) ASSERT(FALSE)

void
AssertInfo(
    IN_Z            char*       Message,
    ...
    );

void
AssertSetFunction(
    IN              PFUNC_AssertFunction    AssertFunction
    );

#ifndef _COMMONLIB_NO_LOCKS_
REQUIRES_EXCL_LOCK(m_assertLock)
RELEASES_EXCL_AND_NON_REENTRANT_LOCK(m_assertLock)
void
AssertFreeLock(
    void
    );
#endif // _COMMONLIB_NO_LOCKS_