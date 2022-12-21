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


//class FOLDER_INFO
FOLDER_INFO::FOLDER_INFO()
{
    this->isDeleted = false;
}

//class FILE_INFO
FILE_INFO::FILE_INFO()
{
    this->isDeleted = false;
}

//class FAT32_INFO
FAT32_INFO::FAT32_INFO(HANDLE hdisk)
{
    unsigned char buffer[512];
    //std::copy(data, data + 512, buffer);
    //this->boot_sector = new BOOT_SECTOR_INFO(buffer);
    this->setBootSector(hdisk);
    this->FAT_1_offset = 512 + this->boot_sector->bpb->reserved_sectors*512;
    this->FAT_2_offset = this->FAT_1_offset + this->boot_sector->bpb->sectors_per_fat_table * 512;
    this->setFAT(hdisk, this->FAT_1, this->FAT_1_offset);
    this->setFAT(hdisk, this->FAT_2, this->FAT_2_offset);
}


bool FAT32_INFO::setBootSector(HANDLE hdisk)
{
    BYTE buffer[512];
    DWORD read = readDisk(hdisk, { 0 }, buffer, 512);
    this->boot_sector = new BOOT_SECTOR_INFO(buffer);
    return true;
}


bool FAT32_INFO::setFAT(HANDLE hdisk, std::vector<unsigned char*>& FAT, unsigned int FAT_offset)
{
    unsigned char* FAT_sector;
    for (unsigned int i = 0; i < this->boot_sector->bpb->sectors_per_fat_table; i++)
    {
        FAT_sector = new unsigned char[512];
        //std::copy(data + i * 512, data + (i + 1) * 512, buffer);
        readDisk(hdisk, { FAT_offset + i * 512 }, FAT_sector, 512);
        FAT.push_back(FAT_sector);
    }
    return true;
}

bool FAT32_INFO::setFolders(HANDLE hdisk)
{
    return true;
}

bool FAT32_INFO::setFiles(HANDLE hdisk)
{
    return true;
}

std::string FAT32_INFO::toString()
{
    return "|| FAT32 INFO START ||\n"
        "\n" + this->boot_sector->toString() + "\n"
        + "FAT1 offset: "+std::to_string(this->FAT_1_offset)+"\n"
        + "FAT1 size: " + std::to_string(this->FAT_1.size()) + "\n"
        + "FAT2 offset: " + std::to_string(this->FAT_2_offset) + "\n"
        + "FAT2 size: " + std::to_string(this->FAT_2.size()) + "\n"
        + "\n|| FAT32 INFO END ||\n";
}