#pragma once

#include "cpu.h"
#include "isr.h"

void
DumpProcessorState(
    IN  PROCESSOR_STATE*    ProcessorState
    );

void
DumpControlRegisters(
    void
    );

void
DumpInterruptStack(
    IN  PINTERRUPT_STACK_COMPLETE       InterruptStack,
    IN  BOOLEAN                         ErrorCodeValid
    );

void
DumpFeatureInformation(
    IN  PCPUID_FEATURE_INFORMATION       FeatureInformation
    );

void
DumpCpuidValues(
    IN  DWORD               Index,
    IN  DWORD               SubIndex,
    IN  CPUID_INFO          Cpuid
    );