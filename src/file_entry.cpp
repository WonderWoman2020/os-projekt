#include "file_entry.hpp"


//class FILE_ENTRY
FILE_ENTRY::FILE_ENTRY(unsigned char* data)
{
    std::fill(this->name, this->name + 12, 0);
    std::copy(data, data + 11, this->name);
    std::copy(data + 11, data + 12, &this->dir_atrribute);
    this->setStartingCluster(data);
    unsigned char buffer[4];
    std::copy(data + 28, data + 32, buffer);
    this->size = convertBytesToInt(buffer, 4);
}

bool FILE_ENTRY::isFolder()
{
    if (this->dir_atrribute == 0x10)
        return true;
    else
        return false;
}

bool FILE_ENTRY::isDeleted()
{
    if (this->name[0] == 0xE5)
        return true;
    else
        return false;
}

bool FILE_ENTRY::setStartingCluster(unsigned char* data)
{
    unsigned char buffer[4];
    std::copy(data + 26, data + 28, buffer);
    std::copy(data + 20, data + 22, buffer + 2);
    this->starting_cluster = convertBytesToInt(buffer, 4);
    return true;
}


std::string FILE_ENTRY::toString()
{
    return "= FILE ENTRY START =\n"
        "Name: " + std::string((char*)this->name) + "\n"
        + "Is deleted: " + (this->isDeleted() ? "true" : "false") + "\n"
        + "Is folder: " + (this->isFolder() ? "true" : "false") + "\n"
        + "Starting cluster: " + std::to_string(this->starting_cluster) + "\n"
        + "Size: " + std::to_string(this->size) + "\n"
        + "\n= FILE ENTRY END =\n";
}