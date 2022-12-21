#include<string>
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

class FAT32_INFO
{
public:
    BOOT_SECTOR_INFO boot_sector;
};