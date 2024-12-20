#pragma once

#include "time.h"

DATETIME
OsTimeGetCurrentDateTime(
    void
    );

SAL_SUCCESS
STATUS
OsTimeGetStringFormattedTime(
    IN_OPT                      PDATETIME       DateTime,
    OUT_WRITES_Z(BufferSize)    char*           Buffer,
    IN                          DWORD           BufferSize
    );