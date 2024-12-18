#pragma once

#include "lapic.h"

SAL_SUCCESS
STATUS
LapicSystemInit(
    void
    );

SAL_SUCCESS
STATUS
LapicSystemInitializeCpu(
    IN      BYTE                            TimerInterruptVector
    );

// Disables or enables the LAPIC in SW
void
LapicSystemSetState(
    IN      BOOLEAN                         Enable
    );

BOOLEAN
LapicSystemGetState(
    void
    );

void
LapicSystemSendEOI(
    IN      BYTE                            Vector
    );

void
LapicSystemEnableTimer(
    IN      DWORD                           Microseconds
    );

void
LapicSystemSendIpi(
    _When_(ApicDestinationShorthandNone == DeliveryMode, IN)
    _When_(ApicDestinationShorthandNone != DeliveryMode, _Reserved_)
            APIC_ID                         ApicId,
    IN      _Strict_type_match_
            APIC_DELIVERY_MODE              DeliveryMode,
    IN      _Strict_type_match_
            APIC_DESTINATION_SHORTHAND      DestinationShorthand,
    IN      _Strict_type_match_
            APIC_DESTINATION_MODE           DestinationMode,
    IN_OPT  BYTE*                           Vector
    );

BYTE
LapicSystemGetPpr(
    void
    );

QWORD
LapicSystemGetTimerElapsedUs(
    void
    );

BOOLEAN
LapicSystemIsInterruptServiced(
    IN      BYTE                            Vector
    );