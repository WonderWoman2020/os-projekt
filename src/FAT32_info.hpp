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
    int getNextFileClusterNumber(int cluster_number);
    bool isLastFileCluster(int cluster_number);
    bool isFreeCluster(int cluster_number);
    bool isBadCluster(int cluster_number);
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
    bool readDirEntries(HANDLE hdisk, unsigned int starting_cluster_number);
    FAT_TABLE* getFAT(short table_number);

    unsigned int calculateEntryPosition(unsigned int entry_number);

    void showFilesEntries();
    std::string toString();
};