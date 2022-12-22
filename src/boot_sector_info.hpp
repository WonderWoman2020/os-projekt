#include "bpb_info.hpp"
#include<string>
#include<Windows.h>


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