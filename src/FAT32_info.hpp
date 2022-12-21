#include<string>
#include<vector>
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
    std::string toString();

};

class FOLDER_INFO
{
    bool isDeleted;
    FOLDER_INFO();
};

class FILE_INFO
{
    bool isDeleted;
    FILE_INFO();
};

class FAT32_INFO
{
public:
    BOOT_SECTOR_INFO* boot_sector;
    unsigned int FAT_1_offset;
    unsigned int FAT_2_offset;
    std::vector<unsigned char[512]> FAT_1;
    std::vector<unsigned char[512]> FAT_2;
    std::vector<FOLDER_INFO*> folders;
    std::vector<FILE_INFO*> files;
    FAT32_INFO(unsigned char* data);

    bool setFAT();
    bool setFolders();
    bool setFiles();
};