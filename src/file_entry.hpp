#include<string>
#include "helper_functions.hpp"

class FILE_ENTRY
{
public:
    unsigned char name[12];
    unsigned int starting_cluster;
    unsigned int size;
    unsigned char dir_atrribute;
    FILE_ENTRY(unsigned char* data);
    std::string toString();
    bool isFolder();
    bool isDeleted();
    unsigned char* getFileExtension();
    unsigned char* getFileName();
    bool setStartingCluster(unsigned char* data);
};