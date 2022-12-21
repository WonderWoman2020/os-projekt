#include "FAT32_info.hpp"


//class BPB_INFO
BPB_INFO::BPB_INFO(unsigned char data[512])
{
    this->bytes_per_sector = this->calculateField(data, 11, 2);
    this->sectors_per_cluster = this->calculateField(data, 13, 1);
    this->reserved_sectors = this->calculateField(data, 14, 1);
    this->media_descriptor = this->calculateField(data, 21, 1);
    this->all_sectors_on_volume = this->calculateField(data, 0x20, 4);
    this->sectors_per_fat_table = this->calculateField(data, 0x24, 4);
    this->fs_version = this->calculateField(data, 0x2A, 2);
    this->root_first_cluster = this->calculateField(data, 0x2C, 4);
    this->fs_info_sector = this->calculateField(data, 0x30, 2);
}

unsigned int BPB_INFO::calculateField(unsigned char data[512], short offset, short size)
{
    unsigned char* buffer = new unsigned char[size];
    std::copy(data + offset, data + offset + size, buffer);
    return convertBytesToInt(buffer, size);
}

std::string BPB_INFO::toString()
{
    return "** BPB INFO START **\n\n"
        "Bytes per sector: " + std::to_string(this->bytes_per_sector) + "\n"
        + "Sectors per cluster: " + std::to_string(this->sectors_per_cluster) + "\n"
        + "Reserved sectors: " + std::to_string(this->reserved_sectors) + "\n"
        + "Media descriptor: " + std::to_string(this->media_descriptor) + "\n"
        + "All sectors on volume: " + std::to_string(this->all_sectors_on_volume) + "\n"
        + "Sectors per FAT table: " + std::to_string(this->sectors_per_fat_table) + "\n"
        + "File system version: " + std::to_string(this->fs_version) + "\n"
        + "Root first cluster: " + std::to_string(this->root_first_cluster) + "\n"
        + "File system info sector: " + std::to_string(this->fs_info_sector) + "\n"
        "\n** BPB INFO END **\n";
}



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


/*class FAT32_INFO
{
public:
    BOOT_SECTOR_INFO boot_sector;
};*/