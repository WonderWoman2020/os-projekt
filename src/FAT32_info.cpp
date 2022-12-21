#include "FAT32_info.hpp"


//class BPB_INFO
BPB_INFO::BPB_INFO(unsigned char data[512])
{
    this->bytes_per_sector = this->calculateField(data, 11, 2);
    this->sectors_per_cluster = this->calculateField(data, 13, 1);
    this->reserved_sectors = this->calculateField(data, 14, 2);
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

//class FAT32_INFO
FAT32_INFO::FAT32_INFO(HANDLE hdisk)
{
    unsigned char buffer[512];
    this->setBootSector(hdisk);
    this->FAT_1_offset = this->boot_sector->bpb->reserved_sectors*512;
    this->FAT_2_offset = this->FAT_1_offset + this->boot_sector->bpb->sectors_per_fat_table * 512;
    this->user_data_offset = this->FAT_2_offset + this->boot_sector->bpb->sectors_per_fat_table * 512;
    this->setFAT(hdisk, this->FAT_1, this->FAT_1_offset);
    this->setFAT(hdisk, this->FAT_2, this->FAT_2_offset);
    this->readDirEntries(hdisk, this->boot_sector->bpb->root_first_cluster);
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

LARGE_INTEGER FAT32_INFO::getClusterPosition(unsigned int cluster_number)
{
    return { this->user_data_offset + (cluster_number-2) * this->boot_sector->bpb->sectors_per_cluster * 512 };
}


bool FAT32_INFO::readDirEntries(HANDLE hdisk, unsigned int starting_cluster_number)
{
    unsigned int cluster_size_bytes = this->boot_sector->bpb->sectors_per_cluster * 512;
    unsigned char* cluster = new unsigned char[cluster_size_bytes];
    unsigned int number_of_entries_in_cluster = cluster_size_bytes / 32;

    unsigned int cluster_number = starting_cluster_number;
    while (cluster_number != 0xFFFFFFFF && cluster_number != 0)
    {
        readDisk(hdisk, { this->getClusterPosition(starting_cluster_number) }, cluster, cluster_size_bytes);

        for (int i = 0; i < number_of_entries_in_cluster; i++)
        {
            FILE_ENTRY* entry = new FILE_ENTRY(cluster, i);
            if (entry->isFolder)
                readDirEntries(hdisk, entry->starting_cluster);
            this->files_and_dirs.push_back(entry);
        }
        cluster_number = 0;//this->FAT_1[512*]
    }
    return true;
}

void FAT32_INFO::showFilesEntries()
{
    for (unsigned int i = 0; i < this->files_and_dirs.size(); i++)
        std::cout << i<< ": " << this->files_and_dirs.at(i)->toString() << std::endl;
}

std::string FAT32_INFO::toString()
{
    return "|| FAT32 INFO START ||\n"
        "\n" + this->boot_sector->toString() + "\n"
        + "FAT1 offset: "+std::to_string(this->FAT_1_offset)+"\n"
        + "FAT1 size: " + std::to_string(this->FAT_1.size()) + "\n"
        + "FAT2 offset: " + std::to_string(this->FAT_2_offset) + "\n"
        + "FAT2 size: " + std::to_string(this->FAT_2.size()) + "\n"
        + "User data offset: " + std::to_string(this->user_data_offset)+"\n"
        + "\n|| FAT32 INFO END ||\n";
}

//class FILE_ENTRY
FILE_ENTRY::FILE_ENTRY(unsigned char* data, unsigned int entry_number)
{
    this->isDeleted = false;
    this->isFolder = false;
    std::fill(this->name, this->name + 12, 0);
    std::copy(data+entry_number*32, data + entry_number*32 + 11, this->name);
    unsigned char buffer[4];
    unsigned char clusterLow[2];
    unsigned char clusterHigh[2];
    std::copy(data + entry_number * 32 + 26, data + entry_number * 32 + 28, clusterLow);
    std::copy(data + entry_number * 32 + 20, data + entry_number * 32 + 22, clusterHigh);
    std::copy(clusterLow, clusterLow + 2, buffer);
    std::copy(clusterHigh, clusterHigh + 2, buffer + 2);
    this->starting_cluster = convertBytesToInt(buffer, 4);
    std::copy(data + entry_number * 32 + 28, data + entry_number * 32 + 32, buffer);
    this->size = convertBytesToInt(buffer, 4);
}

std::string FILE_ENTRY::toString()
{
    return "= FILE ENTRY START =\n"
        "Name: "+std::string((char*)this->name) + "\n"
        + "Is deleted: " + (this->isDeleted ? "true" : "false") + "\n"
        + "Is folder: " + (this->isFolder ? "true" : "false") + "\n"
        + "Starting cluster: " + std::to_string(this->starting_cluster) + "\n"
        + "Size: "+std::to_string(this->size) + "\n"
        + "\n= FILE ENTRY END =\n";
}