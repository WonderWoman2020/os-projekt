#include "FAT32_info.hpp"



FAT_TABLE::FAT_TABLE(unsigned char* data, unsigned int size)
{
    this->entries = data;
    this->size = size;
}

unsigned int FAT_TABLE::getNextFileClusterNumber(unsigned int cluster_number)
{
    if (this->size >= cluster_number * 4)
        return convertBytesToInt(this->entries + cluster_number * 4, 4);
    else
        return 0xFFFFFFFF;
}

bool FAT_TABLE::isLastFileCluster(unsigned int cluster_number)
{
    if ((cluster_number & 0x0FFFFFFF) >= 0x0FFFFFF8)
        return true;
    else
        return false;
}

bool FAT_TABLE::isFreeCluster(unsigned int cluster_number)
{
    if ((cluster_number & 0x0FFFFFFF) == 0)
        return true;
    else
        return false;
}

bool FAT_TABLE::isBadCluster(unsigned int cluster_number)
{
    if ((cluster_number & 0x0FFFFFFF) == 0x0FFFFFF7)
        return true;
    else
        return false;
}


//class FAT32_INFO
FAT32_INFO::FAT32_INFO(HANDLE hdisk)
{
    unsigned char buffer[512];
    this->setBootSector(hdisk);
    this->setFATs(hdisk);
    this->readDirEntries(hdisk, this->boot_sector->getRootClusterNumber());
}


bool FAT32_INFO::setBootSector(HANDLE hdisk)
{
    BYTE buffer[512];
    DWORD read = readDisk(hdisk, { 0 }, buffer, 512);
    this->boot_sector = new BOOT_SECTOR_INFO(buffer);
    return true;
}


bool FAT32_INFO::setFATs(HANDLE hdisk)
{
    for (unsigned char i = 0; i < 2; i++)
    {
        unsigned char* FAT = new unsigned char[this->boot_sector->getFatSize()];
        readDisk(hdisk, { this->boot_sector->getFatOffset(i) }, FAT, this->boot_sector->getFatSize());
        this->FATs.push_back(new FAT_TABLE(FAT, this->boot_sector->getFatSize()));
    }
    return true;
}


FAT_TABLE* FAT32_INFO::getFAT(short table_number)
{
    return this->FATs.at(table_number-1);
}


bool FAT32_INFO::readDirEntries(HANDLE hdisk, unsigned int starting_cluster_number)
{
    unsigned char* cluster = new unsigned char[this->boot_sector->getClusterSize()];
    unsigned int number_of_entries_in_cluster = this->boot_sector->getClusterSize() / 32;

    unsigned int cluster_number = starting_cluster_number;
    while (cluster_number != 0xFFFFFFFF && cluster_number != 0)
    {
        readDisk(hdisk, { this->boot_sector->getClusterPosition(starting_cluster_number) }, cluster, this->boot_sector->getClusterSize());

        for (int i = 0; i < number_of_entries_in_cluster; i++)
        {
            FILE_ENTRY* entry = new FILE_ENTRY(cluster + this->calculateFileEntryPosition(i));
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

unsigned int FAT32_INFO::calculateFileEntryPosition(unsigned int entry_number)
{
    return entry_number * 32;
}

std::string FAT32_INFO::toString()
{
    return "|| FAT32 INFO START ||\n"
        "\n" + this->boot_sector->toString() + "\n"
        + "FAT1 offset: "+std::to_string(this->boot_sector->getFatOffset(1))+"\n"
        + "FAT1 size (in sectors): " + std::to_string(this->getFAT(1)->size/this->boot_sector->getSectorSize()) + "\n"
        + "FAT2 offset: " + std::to_string(this->boot_sector->getFatOffset(2)) + "\n"
        + "FAT2 size (in sectors): " + std::to_string(this->getFAT(2)->size / this->boot_sector->getSectorSize()) + "\n"
        + "User data offset: " + std::to_string(this->boot_sector->getUserDataOffset())+"\n"
        + "\n|| FAT32 INFO END ||\n";
}

