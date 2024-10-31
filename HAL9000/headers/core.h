#pragma once

void
CorePreinit(
    void
    );

SAL_SUCCESS
STATUS
CoreInit(
    void
    );

void
CoreUpdateIdleTime(
    IN  BOOLEAN             IdleScheduled
    );

DWORD
CoreSetIdlePeriod(
    IN  DWORD               SecondsForIdle
    );