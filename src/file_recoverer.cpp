#include "file_recoverer.hpp"


FILE_RECOVERER::FILE_RECOVERER(const char* path_to_recover, const char* path_to_save)
{
    this->path_to_recover = WCHAR_T_CONVERTER::convert((const char*)path_to_recover);
    this->path_to_save = WCHAR_T_CONVERTER::convert((const char*)path_to_save);
    HANDLE hdisk = this->openDisk(this->path_to_recover);
    if (hdisk == INVALID_HANDLE_VALUE)
        this->fat32_info = nullptr;
    else
    {
        this->fat32_info = new FAT32_INFO(hdisk);
        CloseHandle(hdisk);
    }
    this->data_carving_saves = 0;
}

void FILE_RECOVERER::recoverFiles()
{
    if (this->fat32_info == nullptr)
        return;

    HANDLE hdisk = this->openDisk(this->path_to_recover);

    if (hdisk == INVALID_HANDLE_VALUE)
        return;

    CreateDirectory(this->path_to_save, nullptr);

    for (int i = 0; i < this->fat32_info->files_and_dirs.size(); i++)
    {
        FILE_ENTRY* file_entry = this->fat32_info->files_and_dirs.at(i);
        if (file_entry->isFolder())
            continue;

        unsigned char* file_data = this->fat32_info->readFile(hdisk, this->fat32_info->getFAT(1), file_entry);

        if (file_data == nullptr)
            continue;

        if (!this->checkIfFileExtensionValid(file_entry->getFileExtension()))
            continue;

        this->saveRecoveredFile(file_data, file_entry);
        delete[] file_data;

    }

    CloseHandle(hdisk);
}

void FILE_RECOVERER::recoverFilesDataCarving(unsigned int number_of_clusters_to_check)
{
    if (this->fat32_info == nullptr)
        return;

    HANDLE hdisk = this->openDisk(this->path_to_recover);

    if (hdisk == INVALID_HANDLE_VALUE)
        return;

    unsigned int number_of_clusters = this->fat32_info->boot_sector->getFatSize() / 4;

    if (number_of_clusters_to_check < number_of_clusters)
        number_of_clusters = number_of_clusters_to_check;

    std::string iend_png_string = std::string((const char*)this->png_iend, 12);
    unsigned char* data_for_checking = new unsigned char[this->fat32_info->boot_sector->getClusterSize()];
    for (int i = 2; i < number_of_clusters; i++)
    {
        if (!this->checkIfFileStart(i, this->png_signature))
            continue;

        unsigned int starting_cluster = i;
        std::cout << "Znaleziono poczatek png, klaster " << starting_cluster << std::endl;
        unsigned int checked_size = this->findFileEndingOffset(starting_cluster, this->png_iend, 12);
        if (checked_size == 0)
            continue;

        unsigned char* file_data = this->readContinuousMemoryBlock(starting_cluster, checked_size);
        if (file_data == nullptr)
            continue;

        std::cout << "Ile bajtow png: " << checked_size << std::endl;
        this->saveRecoveredFile(file_data, checked_size, (unsigned char*)"png");
        delete[] file_data;
    }
    CloseHandle(hdisk);
}

HANDLE FILE_RECOVERER::createEmptyFile(wchar_t* file_full_name)
{
    HANDLE hNewFile = CreateFile(file_full_name,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);

    return hNewFile;
}

HANDLE FILE_RECOVERER::openDisk(wchar_t* path)
{
    HANDLE hdisk = CreateFile(path,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0, NULL);

    return hdisk;
}

bool FILE_RECOVERER::saveRecoveredFile(unsigned char* file_data, FILE_ENTRY* file_entry)
{
    if (file_data == nullptr || file_entry == nullptr)
        return false;

    wchar_t* file_name = this->createFileName(file_entry);
    wchar_t* file_full_name_to_save = this->createFilePath(this->path_to_save, file_name);
    return this->saveFile(file_data, file_entry->size, file_full_name_to_save);
}

bool FILE_RECOVERER::saveRecoveredFile(unsigned char* file_data, unsigned int data_size, unsigned char* ext)
{
    if (file_data == nullptr || ext == nullptr)
        return false;

    wchar_t* file_name = this->createFileName(ext);
    wchar_t* file_full_name_to_save = this->createFilePath(this->path_to_save, file_name);
    return this->saveFile(file_data, data_size, file_full_name_to_save);
}

bool FILE_RECOVERER::saveFile(unsigned char* file_data, unsigned int data_size, wchar_t* file_path)
{
    HANDLE hNewFileToSave = this->createEmptyFile(file_path);
    if (hNewFileToSave == INVALID_HANDLE_VALUE)
        return false;

    DWORD bytesWritten;
    WriteFile(hNewFileToSave, file_data, data_size, &bytesWritten, nullptr);
    CloseHandle(hNewFileToSave);

    return true;
}

bool FILE_RECOVERER::checkIfFileExtensionValid(unsigned char* ext)
{
    if (ext == nullptr)
        return false;

    if (std::strlen((const char*)ext) == 0 || ext[0] == ' ')
        return false;

    return true;
}

wchar_t* FILE_RECOVERER::createFileName(FILE_ENTRY* file_entry)
{
    unsigned char* name = file_entry->getFileName();
    unsigned char* ext = file_entry->getFileExtension();
    if (name[0] == 0xE5)
        name[0] = 'A';

    unsigned int len_name_to_save = std::find(name, name + 11, ' ') - name;
    wchar_t* wide_name = WCHAR_T_CONVERTER::convert((const char*)name);
    wchar_t* wide_ext = WCHAR_T_CONVERTER::convert((const char*)ext);
    wchar_t* dot = WCHAR_T_CONVERTER::convert(".");
    wide_ext = WCHAR_T_CONVERTER::concatenate(dot, wide_ext);
    wchar_t* file_name = WCHAR_T_CONVERTER::concatenate(wide_name, wide_ext, len_name_to_save, std::wcslen(wide_ext));

    return file_name;
}

wchar_t* FILE_RECOVERER::createFilePath(wchar_t* path_to_save, wchar_t* file_name)
{
    wchar_t* backslash = WCHAR_T_CONVERTER::convert("\\\\");
    //wchar_t* file_name_to_save = this->createFileName(file_entry);
    wchar_t* file_full_name_to_save = WCHAR_T_CONVERTER::concatenate(this->path_to_save, backslash);
    file_full_name_to_save = WCHAR_T_CONVERTER::concatenate(file_full_name_to_save, file_name);
    //WCHAR_T_CONVERTER::print(file_name);
    WCHAR_T_CONVERTER::print(file_full_name_to_save);

    return file_full_name_to_save;
}

wchar_t* FILE_RECOVERER::createFileName(unsigned char* ext)
{
    wchar_t* generated_name = WCHAR_T_CONVERTER::convert("DATA_CARVING_");
    std::string temp_number = std::to_string(this->data_carving_saves);
    unsigned char* number = new unsigned char[11];
    std::fill(number, number + 11, 0);
    std::strcpy((char*)number, temp_number.c_str());
    generated_name = WCHAR_T_CONVERTER::concatenate(generated_name, WCHAR_T_CONVERTER::convert((const char*)number));
    wchar_t* dot = WCHAR_T_CONVERTER::convert(".");
    wchar_t* wide_ext = WCHAR_T_CONVERTER::convert((const char*)ext);
    generated_name = WCHAR_T_CONVERTER::concatenate(generated_name, dot);
    generated_name = WCHAR_T_CONVERTER::concatenate(generated_name, wide_ext);
    this->data_carving_saves++;
    return generated_name;
}


bool FILE_RECOVERER::checkIfFileStart(unsigned int cluster_number, unsigned char* start_signature)
{
    HANDLE hdisk = this->openDisk(this->path_to_recover);
    if (hdisk == INVALID_HANDLE_VALUE)
        return false;

    unsigned char* data_for_checking = new unsigned char[this->fat32_info->boot_sector->getClusterSize()];
    unsigned char* possible_signature = new unsigned char[9];
    std::fill(possible_signature, possible_signature + 9, 0);
    readDisk(hdisk, this->fat32_info->boot_sector->getClusterPosition(cluster_number), data_for_checking, this->fat32_info->boot_sector->getClusterSize());
    std::copy(data_for_checking, data_for_checking + 8, possible_signature);
    delete[] data_for_checking;
    if (std::strcmp((const char*)start_signature, (const char*)possible_signature) == 0)
    {
        delete[] possible_signature;
        return true;
    }

    delete[] possible_signature;
    return false;
}

unsigned int FILE_RECOVERER::findFileEndingOffset(unsigned int cluster_number, unsigned char* end_signature, unsigned int len_end_signature)
{
    HANDLE hdisk = this->openDisk(this->path_to_recover);
    if (hdisk == INVALID_HANDLE_VALUE)
        return 0;

    std::string iend_png_string = std::string((const char*)end_signature, len_end_signature);
    unsigned int ending_cluster = cluster_number;
    bool iend_found = false;
    unsigned int checked_size = 0;
    unsigned int file_size_limit = 2 << 20;
    //std::cout << "File size limit: " << file_size_limit << std::endl;
    unsigned char* data_for_checking = new unsigned char[this->fat32_info->boot_sector->getClusterSize()];
    readDisk(hdisk, this->fat32_info->boot_sector->getClusterPosition(cluster_number), data_for_checking, this->fat32_info->boot_sector->getClusterSize());
    while (!iend_found && checked_size < file_size_limit)
    {
        std::string data = std::string((const char*)data_for_checking, this->fat32_info->boot_sector->getClusterSize());
        auto index = data.find(iend_png_string);
        if (index != std::string::npos)
        {
            std::cout << "Znaleziono koniec png, klaster " << ending_cluster << ", pozycja: " << index << std::endl;
            iend_found = true;
            checked_size = checked_size + (unsigned int)index + len_end_signature;
            return checked_size;
        }
        checked_size = checked_size + this->fat32_info->boot_sector->getClusterSize();
        ending_cluster++;
        readDisk(hdisk, this->fat32_info->boot_sector->getClusterPosition(ending_cluster), data_for_checking, this->fat32_info->boot_sector->getClusterSize());
    }
    delete[] data_for_checking;

    if (iend_found)
        return checked_size;
    else
        return 0;
}

unsigned char* FILE_RECOVERER::readContinuousMemoryBlock(unsigned int cluster_number, unsigned int size)
{
    HANDLE hdisk = this->openDisk(this->path_to_recover);
    if (hdisk == INVALID_HANDLE_VALUE)
        return nullptr;

    unsigned int length_in_clusters = std::ceil((double)size / (double)this->fat32_info->boot_sector->getClusterSize());
    unsigned char* file_data = new unsigned char[size];
    std::fill(file_data, file_data + size, 0);
    for (int a = 0; a < length_in_clusters; a++)
    {
        if (a == length_in_clusters - 1)
        {
            unsigned int remaining_bytes = size - (length_in_clusters - 1) * this->fat32_info->boot_sector->getClusterSize();

            unsigned char* buffer = new unsigned char[this->fat32_info->boot_sector->getClusterSize()];
            std::fill(buffer, buffer + this->fat32_info->boot_sector->getClusterSize(), 0);

            int read = readDisk(hdisk, this->fat32_info->boot_sector->getClusterPosition(cluster_number + a), buffer, this->fat32_info->boot_sector->getClusterSize());
            std::copy(buffer, buffer + remaining_bytes, file_data + a * this->fat32_info->boot_sector->getClusterSize());
            //std::cout << "Bajty odczytane koñcówka: " << read << ", a by³o: " << remaining_bytes << std::endl;
        }
        else
        {
            readDisk(hdisk, this->fat32_info->boot_sector->getClusterPosition(cluster_number + a),
                file_data + a * this->fat32_info->boot_sector->getClusterSize(), this->fat32_info->boot_sector->getClusterSize());
        }
    }

    return file_data;
}