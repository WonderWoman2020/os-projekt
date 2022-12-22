#include<string>
#include "helper_functions.hpp"


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