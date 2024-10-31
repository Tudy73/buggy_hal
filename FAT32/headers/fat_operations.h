#pragma once

// Structure containing information about the
// FAT32 partition
typedef struct _FAT_DATA
{
    PDEVICE_OBJECT      VolumeDevice;

    DWORD               CountOfClusters;            // = DataSectors / SectorsPerCluster

    QWORD               RootDirectoryStartSector;   // Sector where root directory starts

    DWORD               ReservedSectors;            // Number of reserved sectors

    DWORD               FirstDataSector;            // First Sector where the Data Region starts
    DWORD               SectorsPerCluster;          // Number of sectors / cluster

    DWORD               BytesPerSector;             // Number of bytes / sector
    DWORD               EntriesPerSector;           // Directory entries / sector

    DWORD               AllocationSize;
} FAT_DATA, *PFAT_DATA;

SAL_SUCCESS
STATUS
FatInitVolume(
    INOUT          PFAT_DATA           FatData
    );

SAL_SUCCESS
STATUS
FatSearch(
    IN      PFAT_DATA               FatData,
    IN_Z    char*                   Name,
    OUT     QWORD*                  DirectorySector,
    IN      BYTE                    SearchType,
    OUT_OPT PFILE_INFORMATION       FileInformation,
    OUT     QWORD*                  ParentSector
    );

SAL_SUCCESS
STATUS
FatSearchDirectoryEntry(
    IN      PFAT_DATA               FatData,
    IN      QWORD                   SectorToSearch,
    IN_Z    char*                   Name,
    IN      BYTE                    SearchType,
    OUT     QWORD*                  SearchResult,
    OUT_OPT PFILE_INFORMATION       FileInformation,
    OUT     QWORD*                  ParentSector
    );

SAL_SUCCESS
STATUS
FatReadFile(
    IN      PFAT_DATA   FatData,
    IN      QWORD       BaseFileSector,
    IN      QWORD       SectorOffset,
    IN      PVOID       Buffer,
    IN      QWORD       SectorsToRead,
    OUT     QWORD*      SectorsRead,
    IN      BOOLEAN     Asynchronous
    );

SAL_SUCCESS
STATUS
FatCreateDirectoryEntry(
    IN      PFAT_DATA       FatData,
    IN_Z    char*           Name,
    IN      BYTE            FileAttributes
    );

SAL_SUCCESS
STATUS
FatQueryDirectory(
    IN                                              PFAT_DATA                       FatData,
    IN                                              QWORD                           DirectorySector,
    IN                                              DWORD                           DirectoryInformationSize,
    OUT_WRITES_BYTES(DirectoryInformationSize)      PFILE_DIRECTORY_INFORMATION     DirectoryInformation,
    OUT                                             DWORD*                          RequiredDirectionInformationSize
    );