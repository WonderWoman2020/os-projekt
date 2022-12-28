#include "FAT32_info.hpp"



FAT_TABLE::FAT_TABLE(unsigned char* data, unsigned int size)
{
    this->entries = data;
    this->size = size;
}

unsigned int FAT_TABLE::getNextFileClusterNumber(unsigned int cluster_number) // rozdziel na checkIfValidClusterNumber
{
    if (cluster_number >= 2 && cluster_number * 4 <= this->size)
        return convertBytesToInt(this->entries + cluster_number * 4, 4);
    else
        return 0xFFFFFFFF;
}

bool FAT_TABLE::isLastFileCluster(unsigned int cluster_number)
{
    unsigned int fat_entry_val = this->getNextFileClusterNumber(cluster_number) & 0x0FFFFFFF;
    std::cout << fat_entry_val << std::endl;
    if (fat_entry_val >= 0x0FFFFFF8)
        return true;
    else
        return false;
}

bool FAT_TABLE::isFreeCluster(unsigned int cluster_number)
{
    if ((this->getNextFileClusterNumber(cluster_number) & 0x0FFFFFFF) == 0)
        return true;
    else
        return false;
}

bool FAT_TABLE::isBadCluster(unsigned int cluster_number)
{
    if ((this->getNextFileClusterNumber(cluster_number) & 0x0FFFFFFF) == 0x0FFFFFF7)
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
    this->readDirEntries2(hdisk, this->getFAT(1), this->boot_sector->getRootClusterNumber());
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
        readDisk(hdisk, { this->boot_sector->getFatOffset(i+1) }, FAT, this->boot_sector->getFatSize());
        this->FATs.push_back(new FAT_TABLE(FAT, this->boot_sector->getFatSize()));
    }
    return true;
}


FAT_TABLE* FAT32_INFO::getFAT(short table_number)
{
    return this->FATs.at(table_number-1);
}


unsigned int FAT32_INFO::checkDirLengthInFAT(FAT_TABLE* FAT, unsigned int starting_cluster_number)
{
    std::cout <<"Sprawdzany klaster: " << starting_cluster_number << std::endl;
    if (FAT->isFreeCluster(starting_cluster_number) || (starting_cluster_number & 0x0FFFFFFF) >= 0x0FFFFFF8)
        return 0;

    unsigned int cluster_number = starting_cluster_number;
    unsigned int length = 1;
    while (!FAT->isLastFileCluster(cluster_number)) //&& !FAT->isFreeCluster(cluster_number))
    {
        length++;
        cluster_number = FAT->getNextFileClusterNumber(cluster_number);
        //std::cout << length << std::endl;
    }

    return length;
}

unsigned char* FAT32_INFO::readDir(HANDLE hdisk, FAT_TABLE* FAT, unsigned int starting_cluster_number)
{
    unsigned int dir_length = this->checkDirLengthInFAT(FAT, starting_cluster_number);
    std::cout << dir_length << std::endl;
    
    if (dir_length == 0)
        return nullptr;  // TODO sprawdzanie czy to usuniêty folder 
    // i czytanie tylko pierwszego klastra LUB lepiej sprawdzanie, czy to jedyne 'entry' o takim klastrze (detektyw)
    // bo, jeœli folder usuniêto i nie zosta³ nadpisany, jest git, a jeœli zosta³ nadpisany, to jeœli przez inny folder to git (ale nie czytam usuniêtego
    // bo sobie odczytam to samo w tym nowym), a jak przez plik to nie (bo bêd¹ losowe rzeczy w entries)
    // czyli w sumie wystarczy sprawdzaæ, czy nie zosta³ nadpisany folder i go czytaæ tylko jeœli nie zosta³

    unsigned char* directory_data = new unsigned char[dir_length* this->boot_sector->getClusterSize()];
    unsigned int cluster_number = starting_cluster_number;

    for (int i = 0; i < dir_length; i++)
    {
        readDisk(hdisk, this->boot_sector->getClusterPosition(cluster_number), directory_data + i * this->boot_sector->getClusterSize(), this->boot_sector->getClusterSize());
        cluster_number = FAT->getNextFileClusterNumber(cluster_number);
    }

    return directory_data;
}

bool FAT32_INFO::readDirEntries2(HANDLE hdisk, FAT_TABLE* FAT, unsigned int starting_cluster_number)
{
    unsigned char* directory_data = this->readDir(hdisk, FAT, starting_cluster_number);
    unsigned int dir_length = this->checkDirLengthInFAT(FAT, starting_cluster_number);
    unsigned int number_of_entries = (dir_length * this->boot_sector->getClusterSize()) / 32;

    for(int i = 0; i<number_of_entries; i++)
    {
        FILE_ENTRY* entry = new FILE_ENTRY(directory_data + this->calculateFileEntryPosition(i));
        if (entry->name[0] == 0) //koniec wpisów
            break;

        this->files_and_dirs.push_back(entry);
        if (entry->isFolder() && !entry->isDeleted() && entry->starting_cluster != starting_cluster_number && entry->name[0] != '.')
            readDirEntries2(hdisk, FAT, entry->starting_cluster);

    }
    delete[] directory_data;
    return true;
}

void FAT32_INFO::showFilesEntries()
{
    for (unsigned int i = 0; i < this->files_and_dirs.size(); i++)
    {
        FILE_ENTRY* entry = this->files_and_dirs.at(i);
        std::cout << i << ": " << entry->toString() << std::endl;
    }
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

