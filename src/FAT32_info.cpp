#include "FAT32_info.hpp"



FAT_TABLE::FAT_TABLE(unsigned char* data, unsigned int size)
{
    this->entries = data;
    this->size = size;
}

bool FAT_TABLE::isValidClusterNumber(unsigned int cluster_number)
{
    if (cluster_number < 2)
        return false;
    if (cluster_number * 4 >= this->size)
        return false;
    if ((cluster_number & 0x0FFFFFFF) >= 0x0FFFFFF8)
        return false;

    return true;
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
    this->readDirEntries(hdisk, this->getFAT(1), this->boot_sector->getRootClusterNumber(), false);
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


unsigned int FAT32_INFO::checkFileLengthInFAT(FAT_TABLE* FAT, unsigned int starting_cluster_number)
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


unsigned char* FAT32_INFO::readFileWithFAT(HANDLE hdisk, FAT_TABLE* FAT, unsigned int starting_cluster_number)
{
    unsigned int file_length = this->checkFileLengthInFAT(FAT, starting_cluster_number);
    //std::cout << file_length << std::endl;

    if (file_length == 0)
        return nullptr;

    unsigned char* file_data = new unsigned char[file_length * this->boot_sector->getClusterSize()];
    unsigned int cluster_number = starting_cluster_number;

    for (int i = 0; i < file_length; i++)
    {
        readDisk(hdisk, this->boot_sector->getClusterPosition(cluster_number), file_data + i * this->boot_sector->getClusterSize(), this->boot_sector->getClusterSize());
        cluster_number = FAT->getNextFileClusterNumber(cluster_number);
    }

    return file_data;
}

unsigned char* FAT32_INFO::readDeletedFile(HANDLE hdisk, FAT_TABLE* FAT, unsigned int starting_cluster_number)
{
    return nullptr;
}

unsigned char* FAT32_INFO::readFile(HANDLE hdisk, FAT_TABLE* FAT, unsigned int starting_cluster_number, bool isDeleted)
{
    if (isDeleted)
        return this->readDeletedFile(hdisk, FAT, starting_cluster_number);
    else
        return this->readFileWithFAT(hdisk, FAT, starting_cluster_number);
}

bool FAT32_INFO::checkIfFileIsADirectory(HANDLE hdisk, FAT_TABLE* FAT, unsigned int starting_cluster_number)
{
    if (!FAT->isValidClusterNumber(starting_cluster_number))
        return false;

    if (starting_cluster_number == this->boot_sector->getRootClusterNumber())
        return true;

    unsigned char* file_data = new unsigned char[this->boot_sector->getClusterSize()];
    readDisk(hdisk, this->boot_sector->getClusterPosition(starting_cluster_number), file_data, this->boot_sector->getClusterSize());
    FILE_ENTRY* entry = new FILE_ENTRY(file_data);
    if (entry->name[0] == '.' && entry->isFolder())
        return true;

    return false;
}

bool FAT32_INFO::readDirEntries(HANDLE hdisk, FAT_TABLE* FAT, unsigned int starting_cluster_number, bool isDeleted) //FILE_ENTRY* zamiast starting_cluster_number i isDeleted
{
    if (!this->checkIfFileIsADirectory(hdisk, FAT, starting_cluster_number))
        return false;

    unsigned char* directory_data = this->readFile(hdisk, FAT, starting_cluster_number, isDeleted);
    if (directory_data == nullptr)
        return false;

    unsigned int dir_length = this->checkFileLengthInFAT(FAT, starting_cluster_number);
    unsigned int number_of_entries = (dir_length * this->boot_sector->getClusterSize()) / 32;

    for(int i = 0; i<number_of_entries; i++)
    {
        FILE_ENTRY* entry = new FILE_ENTRY(directory_data + this->calculateFileEntryPosition(i));
        if (entry->name[0] == 0) //koniec wpis�w
            break;

        this->files_and_dirs.push_back(entry);
        if (entry->isFolder() && entry->name[0] != '.')
            readDirEntries(hdisk, FAT, entry->starting_cluster, entry->isDeleted());

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

