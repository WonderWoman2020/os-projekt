#include "boot_sector_info.hpp"


//class BOOT_SECTOR_INFO
BOOT_SECTOR_INFO::BOOT_SECTOR_INFO(unsigned char data[512])
{
    this->bpb = new BPB_INFO(data);
    std::copy(data + 3, data + 11, this->oem_id);
    std::copy(data + 510, data + 512, this->sanity_check_code);
}

std::string BOOT_SECTOR_INFO::toString()
{
    return "--- BOOT SECTOR INFO START ---\n\n"
        "OEM ID: " + std::string((char*)this->oem_id) + "\n"
        + "\n" + bpb->toString() + "\n"
        + "Sanity check code: " + bytesToFormattedString(this->sanity_check_code, 2)
        + "\n--- BOOT SECTOR INFO END ---\n";
}

LARGE_INTEGER BOOT_SECTOR_INFO::getClusterPosition(unsigned int cluster_number)
{
    return { this->getUserDataOffset() + (cluster_number - 2) * this->getClusterSize() };
}

unsigned int BOOT_SECTOR_INFO::getFatSize()
{
    return this->bpb->sectors_per_fat_table * this->getSectorSize();
}

unsigned int BOOT_SECTOR_INFO::getFatOffset(short table_number)
{
    return this->bpb->reserved_sectors * this->getSectorSize() + (table_number - 1) * this->getFatSize();
}
unsigned int BOOT_SECTOR_INFO::getUserDataOffset()
{
    return this->bpb->reserved_sectors * this->getSectorSize() + 2 * this->getFatSize();
}

unsigned int BOOT_SECTOR_INFO::getClusterSize()
{
    return this->bpb->sectors_per_cluster * this->getSectorSize();
}


unsigned int BOOT_SECTOR_INFO::getSectorSize()
{
    return this->bpb->bytes_per_sector;
}

unsigned int BOOT_SECTOR_INFO::getRootClusterNumber()
{
    return this->bpb->root_first_cluster;
}
