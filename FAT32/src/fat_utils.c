#include "fat32_base.h"
#include "fat_operations.h"
#include "fat_utils.h"

static
SAL_SUCCESS
STATUS
_ConvertFatDateToDate(
    IN      FATDATE*    FatDate,
    OUT     DATE*       RealDate
    );

static
SAL_SUCCESS
STATUS
_ConvertFatTimeToTime(
    IN      FATTIME*    FatTime,
    OUT     TIME*       RealTime
    );

static
void
_ConvertDateToFatDate(
    IN      DATE*       RealDate,
    OUT     FATDATE*    FatDate
    );

static
void
_ConvertTimeToFatTime(
    IN      TIME*       RealTime,
    OUT     FATTIME*    FatTime
    );

SAL_SUCCESS
STATUS
NextSectorInClusterChain(
    IN      PFAT_DATA       FatData,
    IN      QWORD           CurrentSector,
    OUT     QWORD*          NextSector
    )
{
    STATUS status;
    QWORD currentCluster;
    QWORD nextCluster;

    ASSERT(NULL != FatData);
    ASSERT(NULL != NextSector);

    status = STATUS_SUCCESS;
    currentCluster = 0;
    nextCluster = 0;

    // we get the cluster of the current sector
    status = ClusterOfSector(FatData, CurrentSector, &currentCluster);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("ClusterOfSector", status);
        return status;
    }

    // we get the cluster following the current one
    status = NextClusterInChain(FatData, currentCluster, &nextCluster);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("NextClusterInChain", status);
        return status;
    }

    nextCluster = nextCluster & FAT32_CLUSTER_MASK;

    // we check to see if it's a valid cluster
    if ((nextCluster >= FAT32_BAD_CLUSTER) || (0 == nextCluster))
    {
        // arrived to the end of the cluster chain
        *NextSector = 0;
        return status;
    }

    // we get the sector belonging to the found cluster
    status = FirstSectorOfCluster(FatData, nextCluster, NextSector);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("FirstSectorOfCluster", status);
        return status;
    }

    return status;
}

SAL_SUCCESS
STATUS
NextClusterInChain(
    IN      PFAT_DATA       FatData,
    IN      QWORD           CurrentCluster,
    OUT     QWORD*          NextCluster
    )
{
    STATUS status;
    BYTE* pFat32Entries;        // pointer to a FAT table
    QWORD sectorNumber;
    QWORD offsetInSector;
    QWORD bytesToRead;
    QWORD offset;

    ASSERT(NULL != FatData);
    ASSERT(NULL != NextCluster);

    status = STATUS_SUCCESS;
    pFat32Entries = NULL;

    // each FAT32 entry occupies a DWORD
    sectorNumber = FatData->ReservedSectors + ((sizeof(DWORD) * CurrentCluster) / FatData->BytesPerSector);
    offsetInSector = (sizeof(DWORD) * CurrentCluster) % FatData->BytesPerSector;
    bytesToRead = FatData->BytesPerSector;
    offset = sectorNumber * FatData->BytesPerSector;

    pFat32Entries = (PBYTE) ExAllocatePoolWithTag(PoolAllocateZeroMemory, FatData->BytesPerSector, HEAP_TEMP_TAG, 0 );
    if (NULL == pFat32Entries)
    {
        // could not allocate memory for a FAT32 sector
        LOG_FUNC_ERROR_ALLOC("HeapAllocatePoolWithTag", FatData->BytesPerSector);
        return STATUS_HEAP_NO_MORE_MEMORY;
    }

    // We read the sector
    status = IoReadDevice(FatData->VolumeDevice,
                          pFat32Entries,
                          &bytesToRead,
                          offset
                          );
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("IoReadDevice", status);
        return status;
    }
    ASSERT(bytesToRead == FatData->BytesPerSector);

    // We get the value from SectorOffset
    *NextCluster = *((DWORD*)(&pFat32Entries[offsetInSector]));

    // We free the memory allocated
    ASSERT(NULL != pFat32Entries);
    ExFreePoolWithTag(pFat32Entries, HEAP_TEMP_TAG);

    return STATUS_SUCCESS;
}

SAL_SUCCESS
STATUS
FirstSectorOfCluster(
    IN      PFAT_DATA   FatData,
    IN      QWORD       Cluster,
    OUT     QWORD*      Sector
    )
{
    ASSERT(NULL != FatData);
    ASSERT(NULL != Sector);

    if ((Cluster > FatData->CountOfClusters + 1) || (Cluster < 2))
    {
        // there is no such cluster
        LOG_TRACE_FILESYSTEM("Cluster: 0x%X\n", Cluster);
        LOG_TRACE_FILESYSTEM("Count of clusters: 0x%x\n", FatData->CountOfClusters);
        LOG_TRACE_FILESYSTEM("Sectors per cluster: 0x%x\n", FatData->SectorsPerCluster);
        LOG_TRACE_FILESYSTEM("FatData->FirstDataSector: 0x%x\n", FatData->FirstDataSector);

        return STATUS_DEVICE_CLUSTER_INVALID;
    }

    *Sector = (((Cluster - 2) * FatData->SectorsPerCluster) + FatData->FirstDataSector);

    return STATUS_SUCCESS;
}

SAL_SUCCESS
STATUS
ClusterOfSector(
    IN      PFAT_DATA   FatData,
    IN      QWORD       Sector, 
    OUT     QWORD*      Cluster
    )
{
    ASSERT(NULL != Cluster);

    *Cluster = (((Sector - FatData->FirstDataSector) / FatData->SectorsPerCluster) + 2);

    LOG_TRACE_FILESYSTEM("Sector: 0x%x\n", Sector);
    LOG_TRACE_FILESYSTEM("*Result: 0x%x\n", *Cluster);
    LOG_TRACE_FILESYSTEM("Count of clusters: 0x%x\n", FatData->CountOfClusters);
    LOG_TRACE_FILESYSTEM("Sectors per cluster: 0x%x\n", FatData->SectorsPerCluster);
    LOG_TRACE_FILESYSTEM("FatData->FirstDataSector: 0x%x\n", FatData->FirstDataSector);

    if ((*Cluster > FatData->CountOfClusters + 1) || (*Cluster < 2))
    {
        return STATUS_DEVICE_CLUSTER_INVALID;
    }

    return STATUS_SUCCESS;
}

SAL_SUCCESS
STATUS
ConvertFatDateTimeToDateTime(
    IN      FATDATE*    FatDate,
    IN      FATTIME*    FatTime,
    OUT     DATETIME*   DateTime
    )
{
    STATUS status;

    ASSERT(NULL != DateTime);

    status = STATUS_SUCCESS;

    status = _ConvertFatDateToDate(FatDate, &DateTime->Date);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("ConvertFatDateToDate", status);
        return status;
    }

    status = _ConvertFatTimeToTime(FatTime, &DateTime->Time);
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("ConvertFatTimeToTime", status);
        return status;
    }

    return status;
}

void
ConvertDateTimeToFatDateTime(
    IN      PDATETIME   DateTime,
    OUT     FATDATE*    FatDate,
    OUT     FATTIME*    FatTime
    )
{
    ASSERT(NULL != DateTime);

    _ConvertDateToFatDate(&DateTime->Date, FatDate);
    _ConvertTimeToFatTime(&DateTime->Time, FatTime);
}

SAL_SUCCESS
STATUS
ConvertFatNameToName(
    IN_READS(SHORT_NAME_CHARS)      char*       FatName,
    IN                              DWORD       BufferSize,
    OUT_WRITES(BufferSize)          char*       Buffer,
    OUT                             DWORD*      ActualNameLength
    )
{
    STATUS status;
    DWORD nameLen;
    char extName[4];
    char normalName[9];
    DWORD lastSpaceIndex;
    DWORD i;

    ASSERT(NULL != FatName);
    ASSERT(NULL != Buffer);
    ASSERT(NULL != ActualNameLength);

    status = STATUS_SUCCESS;
    nameLen = 0;
    lastSpaceIndex = SHORT_NAME_NAME;

    // retrieve normal name
    for (i = 0; i < SHORT_NAME_NAME; ++i)
    {
        normalName[i] = FatName[i];
        if (FatName[i] == ' ')
        {
            if (lastSpaceIndex > i)
            {
                lastSpaceIndex = i;
            }
        }
        else
        {
            lastSpaceIndex = SHORT_NAME_NAME;
        }
    }
    normalName[lastSpaceIndex] = '\0';

    // retrieve extension
    for (i = 0; i < SHORT_NAME_EXT; ++i)
    {
        extName[i] = FatName[i+SHORT_NAME_NAME];
        if (FatName[i + SHORT_NAME_NAME] == ' ')
        {
            // we found a space and it's over
            break;
        }
    }
    extName[i] = '\0';
    
    nameLen = strlen(normalName) + i + ( ( 0 != i ) ? 1 : 0 );
    *ActualNameLength = nameLen;
    if (nameLen >= BufferSize)
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (0 == i)
    {
        // there is no extension
        strcpy(Buffer, normalName);
    }
    else
    {
        // there is an extension
        status = snprintf(Buffer, BufferSize, "%s.%s", normalName, extName);
    }

    return status;
}

static
SAL_SUCCESS
STATUS
_ConvertFatDateToDate(
    IN      FATDATE*    FatDate,
    OUT     DATE*       RealDate
    )
{
    ASSERT(NULL != RealDate);
    ASSERT(NULL != FatDate);

    RealDate->Day = (BYTE) FatDate->Day;

    if ((RealDate->Day < FAT_DAY_RANGE_MIN) || (RealDate->Day > FAT_DAY_RANGE_MAX))
    {
        // The day is not valid
        return STATUS_TIME_INVALID;
    }

    RealDate->Month = (BYTE) FatDate->Month;

    if ((RealDate->Month < FAT_MONTH_RANGE_MIN) || (RealDate->Month > FAT_MONTH_RANGE_MAX))
    {
        // the month is invalid
        return STATUS_TIME_INVALID;
    }

    RealDate->Year = FatDate->Year + FAT_START_YEAR;

    return STATUS_SUCCESS;
}

static
SAL_SUCCESS
STATUS
_ConvertFatTimeToTime(
    IN      FATTIME*    FatTime,
    OUT     TIME*       RealTime
    )
{
    ASSERT(NULL != RealTime);

    RealTime->Second = (BYTE) FatTime->Second;

    if ((RealTime->Second < FAT_SECOND_RANGE_MIN) || (RealTime->Second > FAT_SECOND_RANGE_MAX))
    {
        // the seconds parameter is invalid
        return STATUS_TIME_INVALID;
    }

    RealTime->Second *= 2;

    RealTime->Minute = (BYTE) FatTime->Minute;

    if ((RealTime->Minute < FAT_MINUTE_RANGE_MIN) || (RealTime->Minute > FAT_MINUTE_RANGE_MAX))
    {
        // the minutes parameter is invalid
        return STATUS_TIME_INVALID;
    }

    RealTime->Hour = (BYTE) FatTime->Hour;

    if ((RealTime->Hour < FAT_HOUR_RANGE_MIN) || (RealTime->Hour > FAT_HOUR_RANGE_MAX))
    {
        // the hours parameter is invalid
        return STATUS_TIME_INVALID;
    }

    return STATUS_SUCCESS;
}

static
void
_ConvertDateToFatDate(
    IN      DATE*       RealDate,
    OUT     FATDATE*    FatDate
    )
{
    ASSERT(NULL != RealDate);
    ASSERT(NULL != FatDate);

    FatDate->Year = RealDate->Year - FAT_START_YEAR;
    FatDate->Month = RealDate->Month;
    FatDate->Day = RealDate->Day;
}

static
void 
_ConvertTimeToFatTime(
    IN      TIME*       RealTime,
    OUT     FATTIME*    FatTime
    )
{
    ASSERT(NULL != RealTime);
    ASSERT(NULL != FatTime);

    FatTime->Hour = RealTime->Hour;
    FatTime->Minute = RealTime->Minute;
    FatTime->Second = RealTime->Second / 2;
}