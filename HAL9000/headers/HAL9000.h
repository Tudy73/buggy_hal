#pragma once

#include "base.h"
#include "sal_interface.h"
#include "status.h"
#include "hal.h"
#include "msr.h"
#include "common_lib.h"
#include "assert.h"
#include "log.h"
#include "ex.h"

extern QWORD gVirtualToPhysicalOffset;

// Virtual to Physical address conversion and vice-versa
#define VA2PA(addr)                         ((PHYSICAL_ADDRESS)((QWORD)(addr)-gVirtualToPhysicalOffset))
#define PA2VA(addr)                         ((QWORD)(addr)+gVirtualToPhysicalOffset)