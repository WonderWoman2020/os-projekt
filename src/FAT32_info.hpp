#include<string>
#include<vector>
#include<Windows.h>
#include<iostream>

#include"helper_functions.hpp"


class BPB_INFO
{
public:
    unsigned short bytes_per_sector;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char media_descriptor;
    unsigned int all_sectors_on_volume;
    unsigned int sectors_per_fat_table;
    unsigned short fs_version;
    unsigned int root_first_cluster;
    unsigned short fs_info_sector;

    BPB_INFO(unsigned char data[512]);
    unsigned int calculateField(unsigned char data[512], short offset, short size);
    std::string toString();
};

class BOOT_SECTOR_INFO
{
public:
    unsigned char sanity_check_code[2];
    BPB_INFO* bpb;
    unsigned char oem_id[9];

    BOOT_SECTOR_INFO(unsigned char data[512]);
    unsigned int getFatOffset(short table_number);
    unsigned int getUserDataOffset();
    unsigned int getFatSize();
    unsigned int getClusterSize();
    LARGE_INTEGER getClusterPosition(unsigned int cluster_number);
    unsigned int getSectorSize();
    unsigned int getRootClusterNumber();
    std::string toString();
};


class FILE_ENTRY
{
public:
    unsigned char name[12];
    unsigned int starting_cluster;
    unsigned int size;
    unsigned char dir_atrribute;
    FILE_ENTRY(unsigned char* data);
    std::string toString();
    bool isFolder();
    bool isDeleted();
    bool setStartingCluster(unsigned char* data);
};


/*class FAT_TABLE
{
    unsigned char* entries;
    FAT_TABLE();
    int getNextFileClusterNumber(int cluster_number);
    bool isLastFileCluster(int cluster_number);
    bool isFreeCluster(int cluster_number);
    bool isBadCluster(int cluster_number);
};*/

class FAT32_INFO
{
public:
    BOOT_SECTOR_INFO* boot_sector;
    std::vector<unsigned char*> FAT_1;
    std::vector<unsigned char*> FAT_2;
    std::vector<FILE_ENTRY*> files_and_dirs;
    FAT32_INFO(HANDLE hdisk);

    bool setBootSector(HANDLE hdisk);
    bool setFAT(HANDLE hdisk, std::vector<unsigned char*> &FAT, unsigned int FAT_offset);
    bool readDirEntries(HANDLE hdisk, unsigned int starting_cluster_number);
    
    unsigned int calculateEntryPosition(unsigned int entry_number);

    void showFilesEntries();
    std::string toString();
};