#pragma once

SAL_SUCCESS
STATUS
NextSectorInClusterChain(
    IN      PFAT_DATA       FatData,
    IN      QWORD           CurrentSector,
    OUT     QWORD*          NextSector
    );

SAL_SUCCESS
STATUS
NextClusterInChain(
    IN      PFAT_DATA       FatData,
    IN      QWORD           CurrentCluster,
    OUT     QWORD*          Result
    );

SAL_SUCCESS
STATUS
FirstSectorOfCluster(
    IN      PFAT_DATA   FatData,
    IN      QWORD       Cluster,
    OUT     QWORD*      Result
    );

SAL_SUCCESS
STATUS
ClusterOfSector(
    IN      PFAT_DATA   FatData,
    IN      QWORD       Sector,
    OUT     QWORD*      Result
    );

SAL_SUCCESS
STATUS
ConvertFatDateTimeToDateTime(
    IN      FATDATE*    FatDate,
    IN      FATTIME*    FatTime,
    OUT     DATETIME*   DateTime
    );

void
ConvertDateTimeToFatDateTime(
    IN      PDATETIME   DateTime,
    OUT     FATDATE*    FatDate,
    OUT     FATTIME*    FatTime
    );

SAL_SUCCESS
STATUS
ConvertFatNameToName(
    IN_READS(SHORT_NAME_CHARS)      char*       FatName,
    IN                              DWORD       BufferSize,
    OUT_WRITES(BufferSize)          char*       Buffer,
    OUT                             DWORD*      ActualNameLength
    );