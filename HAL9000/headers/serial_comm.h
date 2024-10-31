#pragma once

SAL_SUCCESS
STATUS
SerialCommunicationInitialize(
    IN_READS(NoOfPorts)     WORD*           Ports,
    IN                      DWORD           NoOfPorts
    );

SAL_SUCCESS
STATUS
SerialCommunicationReinitialize(
    void
    );

void
SerialCommWriteBuffer(
    IN_Z char*  Buffer
    );