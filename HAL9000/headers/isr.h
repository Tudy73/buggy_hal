#pragma once

typedef struct _INTERRUPT_STACK
{
    QWORD                       Rip;
    QWORD                       CS;
    QWORD                       RFLAGS;
    QWORD                       Rsp;
    QWORD                       SS;
} INTERRUPT_STACK, *PINTERRUPT_STACK;

typedef struct _INTERRUPT_STACK_COMPLETE
{
    QWORD                       ErrorCode;
    INTERRUPT_STACK             Registers;
} INTERRUPT_STACK_COMPLETE, *PINTERRUPT_STACK_COMPLETE;

typedef
BOOLEAN
(__cdecl FUNC_IsrRoutine)(
    IN_OPT  PVOID               Context
    );

typedef FUNC_IsrRoutine*        PFUNC_IsrRoutine;

SAL_SUCCESS
STATUS
IsrInstallEx(
    IN      BYTE                Vector,
    IN      PFUNC_IsrRoutine    IsrRoutine,
    IN_OPT  PVOID               Context
    );