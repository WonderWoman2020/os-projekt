#include<string>
#include<vector>
#include<Windows.h>
#include<iostream>

#include "helper_functions.hpp"

#include "boot_sector_info.hpp"
#include "file_entry.hpp"



class FAT_TABLE
{
public:
    unsigned char* entries;
    unsigned int size;
    FAT_TABLE(unsigned char* data, unsigned int size);
    bool isValidClusterNumber(unsigned int cluster_number);
    unsigned int getNextFileClusterNumber(unsigned int cluster_number);
    bool isLastFileCluster(unsigned int cluster_number);
    bool isFreeCluster(unsigned int cluster_number);
    bool isBadCluster(unsigned int cluster_number);
};

class FAT32_INFO
{
public:
    BOOT_SECTOR_INFO* boot_sector;
    std::vector<FAT_TABLE*> FATs;
    std::vector<FILE_ENTRY*> files_and_dirs;
    FAT32_INFO(HANDLE hdisk);

    bool setBootSector(HANDLE hdisk);
    bool setFATs(HANDLE hdisk);
    FAT_TABLE* getFAT(short table_number);
    bool readDirEntries(HANDLE hdisk, FAT_TABLE* FAT, FILE_ENTRY * dir_entry);
    unsigned char* readFile(HANDLE hdisk, FAT_TABLE* FAT, FILE_ENTRY* file_entry);
    unsigned int checkFileLengthInFAT(FAT_TABLE* FAT, unsigned int starting_cluster_number);
    bool checkIfFileIsAValidDirectory(HANDLE hdisk, FAT_TABLE* FAT, unsigned int starting_cluster_number);
    void showFilesEntries();
    std::string toString();

private:
    unsigned int calculateFileEntryPosition(unsigned int entry_number);
    unsigned char* readFileWithFAT(HANDLE hdisk, FAT_TABLE* FAT, FILE_ENTRY* file_entry);
    unsigned char* readDeletedFile(HANDLE hdisk, FAT_TABLE* FAT, FILE_ENTRY* file_entry);
};