#pragma once

#pragma warning(push)
#include "acpi.h"
#pragma warning(pop)

void
AcpiInterfacePreinit(
    void
    );

SAL_SUCCESS
STATUS
AcpiInterfaceInit(
    void
    );

void
AcpiShutdown(
    void
    );

SAL_SUCCESS
STATUS
AcpiInterfaceLateInit(
    void
    );

SAL_SUCCESS
STATUS
AcpiRetrieveNextCpu(
    IN      BOOLEAN                     RestartSearch,
    OUT_PTR ACPI_MADT_LOCAL_APIC**      AcpiEntry
    );

SAL_SUCCESS
STATUS
AcpiRetrieveNextIoApic(
    IN      BOOLEAN                     RestartSearch,
    OUT_PTR ACPI_MADT_IO_APIC**         AcpiEntry
    );

SAL_SUCCESS
STATUS
AcpiRetrieveNextInterruptOverride(
    IN      BOOLEAN                         RestartSearch,
    OUT_PTR ACPI_MADT_INTERRUPT_OVERRIDE**  AcpiEntry
    );

SAL_SUCCESS
STATUS
AcpiRetrieveNextMcfgEntry(
    IN      BOOLEAN                     RestartSearch,
    OUT_PTR ACPI_MCFG_ALLOCATION**      AcpiEntry
    );

SAL_SUCCESS
STATUS
AcpiRetrieveNextPrtEntry(
    IN      BOOLEAN                     RestartSearch,
    OUT_PTR ACPI_PCI_ROUTING_TABLE**    AcpiEntry,
    OUT     BYTE*                       BusNumber,
    OUT     WORD*                       SegmentNumber
    );