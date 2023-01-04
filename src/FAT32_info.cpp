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
    //std::cout << fat_entry_val << std::endl;
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
    this->readDirEntries(hdisk, this->getFAT(1), nullptr);
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
    //std::cout <<"Sprawdzany klaster: " << starting_cluster_number << std::endl;
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


unsigned char* FAT32_INFO::readFileWithFAT(HANDLE hdisk, FAT_TABLE* FAT, FILE_ENTRY* file_entry)
{
    unsigned int starting_cluster = (file_entry == nullptr ? this->boot_sector->getRootClusterNumber() : file_entry->starting_cluster);

    if (file_entry != nullptr) //nowe
    {
        if (!file_entry->isFolder() && (file_entry->size == 0 || file_entry->size == 0xFFFFFFFF))
            return nullptr;
    }

    unsigned int file_length = this->checkFileLengthInFAT(FAT, starting_cluster);
    //std::cout << file_length << std::endl;

    if (file_length == 0)
        return nullptr;

    unsigned char* file_data = new unsigned char[file_length * this->boot_sector->getClusterSize()];
    unsigned int cluster_number = starting_cluster;


    // TODO te¿ dodaæ ucinanie koñcówki pliku, jeœli to nie folder? w sumie wystarczy na etapie zapisywania podaæ ile bajtów chcê zapisaæ
    for (int i = 0; i < file_length; i++)
    {
        readDisk(hdisk, this->boot_sector->getClusterPosition(cluster_number), file_data + i * this->boot_sector->getClusterSize(), this->boot_sector->getClusterSize());
        cluster_number = FAT->getNextFileClusterNumber(cluster_number);
    }

    return file_data;
}

unsigned char* FAT32_INFO::readDeletedFile(HANDLE hdisk, FAT_TABLE* FAT, FILE_ENTRY* file_entry)
{
    if(file_entry == nullptr)
        return nullptr;

    if (file_entry->isFolder()) //przepe³niæ jakiœ folder i sprawdziæ jak siê zapisze? czy w kolejnym klastrze bêd¹ te¿ foldery '.' i '..'
    {
        if (!FAT->isFreeCluster(file_entry->starting_cluster))
            return nullptr;

        unsigned char* file_data = new unsigned char[this->boot_sector->getClusterSize()];
        readDisk(hdisk, this->boot_sector->getClusterPosition(file_entry->starting_cluster), file_data, this->boot_sector->getClusterSize());
        return file_data;
    }

    if (file_entry->size == 0 || file_entry->size == 0xFFFFFFFF) //nowe
        return nullptr;

    unsigned char* file_data = new unsigned char[file_entry->size+1];
    std::fill(file_data, file_data + file_entry->size + 1, 0);
    //TODO if klaster jest obecnie nie zajêty przez istniej¹cy plik w FAT - sprawdzenie rozmiaru usunietego
    // pliku jako ile niezajêtych klastrow zajmuje w granicach deklarowanego rozmiaru

    /*unsigned int file_length; //nowe
    if (file_entry->size < this->boot_sector->getClusterSize())
        file_length = 1;
    else
        file_length = std::ceil(file_entry->size / this->boot_sector->getClusterSize());*/
    unsigned int file_length = std::ceil((double) file_entry->size / (double) this->boot_sector->getClusterSize());
    //std::cout << "Ile klastrów zostanie odczytanych: " << file_length << std::endl;
    unsigned int cluster_number = file_entry->starting_cluster;
    for (int i = 0; i < file_length; i++)
    {
        if (i == (file_length - 1))
        {
            unsigned int remaining_bytes = file_entry->size - (file_length-1) * this->boot_sector->getClusterSize(); //poprawka -1 file_length

            unsigned char* buffer = new unsigned char[this->boot_sector->getClusterSize()];
            std::fill(buffer, buffer + this->boot_sector->getClusterSize(), 0);

            //int read = readDisk(hdisk, this->boot_sector->getClusterPosition(cluster_number), file_data + i * this->boot_sector->getClusterSize(), remaining_bytes); //czy nie klaster ca³y?
            int read = readDisk(hdisk, this->boot_sector->getClusterPosition(cluster_number), buffer, this->boot_sector->getClusterSize()); //czy nie klaster ca³y?
            //if (file_entry->size < 10)
            //    std::cout << buffer << std::endl;
            std::copy(buffer, buffer+remaining_bytes ,file_data + i * this->boot_sector->getClusterSize());

            //std::cout << "Bajty odczytane koñcówka: " << read << ", a by³o: " << remaining_bytes << std::endl;
        }
        else
        {
            int read = readDisk(hdisk, this->boot_sector->getClusterPosition(cluster_number), file_data + i * this->boot_sector->getClusterSize(), this->boot_sector->getClusterSize());
            cluster_number++;
            //std::cout << "Bajty odczytane: " << read << std::endl;
        }
    }

    //if (file_entry->size < 10)
    //    std::cout << file_data << std::endl;

    return file_data;
}

unsigned char* FAT32_INFO::readFile(HANDLE hdisk, FAT_TABLE* FAT, FILE_ENTRY* file_entry)
{
    bool isDeleted = (file_entry == nullptr ? false : file_entry->isDeleted());

    if (isDeleted)
        return this->readDeletedFile(hdisk, FAT, file_entry);
    else
        return this->readFileWithFAT(hdisk, FAT, file_entry);
}

bool FAT32_INFO::checkIfFileIsAValidDirectory(HANDLE hdisk, FAT_TABLE* FAT, unsigned int starting_cluster_number)
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

bool FAT32_INFO::readDirEntries(HANDLE hdisk, FAT_TABLE* FAT, FILE_ENTRY * dir_entry) //FILE_ENTRY* zamiast starting_cluster_number i isDeleted
{
    unsigned int starting_cluster = (dir_entry == nullptr ? this->boot_sector->getRootClusterNumber() : dir_entry->starting_cluster);
    bool isDeleted = (dir_entry == nullptr ? false : dir_entry->isDeleted());

    if (!this->checkIfFileIsAValidDirectory(hdisk, FAT, starting_cluster))
        return false;

    //std::cout << "Klaster " << starting_cluster << " jest uznany za folder i czytany" << std::endl;

    unsigned char* directory_data = this->readFile(hdisk, FAT, dir_entry);
    if (directory_data == nullptr)
        return false;

    unsigned int dir_length; //nowe
    if (!isDeleted)
        dir_length = this->checkFileLengthInFAT(FAT, starting_cluster);
    else
        dir_length = 1;
    //unsigned int dir_length = this->checkFileLengthInFAT(FAT, starting_cluster);
    unsigned int number_of_entries = (dir_length * this->boot_sector->getClusterSize()) / 32;

    for(int i = 0; i<number_of_entries; i++)
    {
        FILE_ENTRY* entry = new FILE_ENTRY(directory_data + this->calculateFileEntryPosition(i));
        if (entry->name[0] == 0) //koniec wpisów
            break;

        this->files_and_dirs.push_back(entry);

        if (entry->isFolder() && entry->name[0] != '.')
            readDirEntries(hdisk, FAT, entry);
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

