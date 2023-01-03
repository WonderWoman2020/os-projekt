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

        this->saveFile(file_data, file_entry);
        delete[] file_data;

    }

    CloseHandle(hdisk);
}

void FILE_RECOVERER::recoverFilesDataCarving()
{
    if (this->fat32_info == nullptr)
        return;

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

bool FILE_RECOVERER::saveFile(unsigned char* file_data, FILE_ENTRY* file_entry)
{
    if (file_data == nullptr || file_entry == nullptr)
        return false;

    wchar_t* file_full_name_to_save = this->createFilePath(this->path_to_save, file_entry);
    HANDLE hNewFileToSave = this->createEmptyFile(file_full_name_to_save);
    if (hNewFileToSave == INVALID_HANDLE_VALUE)
        return false;

    DWORD bytesWritten;
    WriteFile(hNewFileToSave, file_data, file_entry->size, &bytesWritten, nullptr);
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

wchar_t* FILE_RECOVERER::createFilePath(wchar_t* path_to_save, FILE_ENTRY* file_entry)
{
    wchar_t* backslash = WCHAR_T_CONVERTER::convert("\\\\");
    wchar_t* file_name_to_save = this->createFileName(file_entry);
    wchar_t* file_full_name_to_save = WCHAR_T_CONVERTER::concatenate(this->path_to_save, backslash);
    file_full_name_to_save = WCHAR_T_CONVERTER::concatenate(file_full_name_to_save, file_name_to_save);
    WCHAR_T_CONVERTER::print(file_name_to_save);
    WCHAR_T_CONVERTER::print(file_full_name_to_save);

    return file_full_name_to_save;
}