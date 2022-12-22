#include "FAT32_info.hpp"


//class FAT32_INFO
FAT32_INFO::FAT32_INFO(HANDLE hdisk)
{
    unsigned char buffer[512];
    this->setBootSector(hdisk);
    this->setFAT(hdisk, this->FAT_1, this->boot_sector->getFatOffset(1));
    this->setFAT(hdisk, this->FAT_2, this->boot_sector->getFatOffset(2));
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
        FAT_sector = new unsigned char[this->boot_sector->getSectorSize()];
        //std::copy(data + i * 512, data + (i + 1) * 512, buffer);
        readDisk(hdisk, { FAT_offset + i * 512 }, FAT_sector, this->boot_sector->getSectorSize());
        FAT.push_back(FAT_sector);
    }
    return true;
}


bool FAT32_INFO::readDirEntries(HANDLE hdisk, unsigned int starting_cluster_number)
{
    //unsigned int cluster_size_bytes = this->boot_sector->bpb->sectors_per_cluster * 512;
    unsigned char* cluster = new unsigned char[this->boot_sector->getClusterSize()];
    unsigned int number_of_entries_in_cluster = this->boot_sector->getClusterSize() / 32;

    unsigned int cluster_number = starting_cluster_number;
    while (cluster_number != 0xFFFFFFFF && cluster_number != 0)
    {
        readDisk(hdisk, { this->boot_sector->getClusterPosition(starting_cluster_number) }, cluster, this->boot_sector->getClusterSize());

        for (int i = 0; i < number_of_entries_in_cluster; i++)
        {
            FILE_ENTRY* entry = new FILE_ENTRY(cluster + this->calculateEntryPosition(i));
            this->files_and_dirs.push_back(entry);
            /*if (entry->isFolder)
                readDirEntries(hdisk, entry->starting_cluster);*/
        }
        cluster_number = 0;//this->FAT_1[512*]
    }
    delete[] cluster;
    return true;
}

void FAT32_INFO::showFilesEntries()
{
    for (unsigned int i = 0; i < this->files_and_dirs.size(); i++)
        std::cout << i<< ": " << this->files_and_dirs.at(i)->toString() << std::endl;
}

unsigned int FAT32_INFO::calculateEntryPosition(unsigned int entry_number)
{
    return entry_number * 32;
}

std::string FAT32_INFO::toString()
{
    return "|| FAT32 INFO START ||\n"
        "\n" + this->boot_sector->toString() + "\n"
        + "FAT1 offset: "+std::to_string(this->boot_sector->getFatOffset(1))+"\n"
        + "FAT1 size (in sectors): " + std::to_string(this->FAT_1.size()) + "\n"
        + "FAT2 offset: " + std::to_string(this->boot_sector->getFatOffset(2)) + "\n"
        + "FAT2 size (in sectors): " + std::to_string(this->FAT_2.size()) + "\n"
        + "User data offset: " + std::to_string(this->boot_sector->getUserDataOffset())+"\n"
        + "\n|| FAT32 INFO END ||\n";
}

