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
        if (!file_entry->isFolder())
        {
            unsigned char* file_data = this->fat32_info->readFile(hdisk, this->fat32_info->getFAT(1), file_entry);
            if (file_data != nullptr)
            {
                //std::cout << file_entry->toString() << std::endl;
                unsigned char* name = file_entry->getFileName();//getFileName(file_entry->name);
                unsigned char* ext = file_entry->getFileExtension();//getFileExtension(file_entry->name);
                if (name[0] == 0xE5)
                    name[0] = 'A';
                std::cout << name << std::endl;
                std::cout << ext << std::endl;

                if (std::strlen((const char*)ext) != 0 && ext[0] != ' ') // spacje s¹ w rozszerzeniu .gitignore, trzeba do niego poprawiæ albo go ignorowaæ
                {
                    std::cout << "Mo¿na zapisywaæ " << std::strlen((const char*)ext) << std::endl;
                    wchar_t* backslash = WCHAR_T_CONVERTER::convert("\\\\");
                    wchar_t* file_name_to_save = WCHAR_T_CONVERTER::convert((const char*)name);
                    wchar_t* file_full_name_to_save = WCHAR_T_CONVERTER::concatenate(this->path_to_save, backslash);
                    unsigned int len_name_to_save = std::find(file_name_to_save, file_name_to_save + 11, ' ') - file_name_to_save;
                    file_full_name_to_save = WCHAR_T_CONVERTER::concatenate(file_full_name_to_save, file_name_to_save, std::wcslen(file_full_name_to_save), len_name_to_save);
                    file_full_name_to_save = WCHAR_T_CONVERTER::concatenate(file_full_name_to_save, WCHAR_T_CONVERTER::convert("."));
                    file_full_name_to_save = WCHAR_T_CONVERTER::concatenate(file_full_name_to_save, WCHAR_T_CONVERTER::convert((const char*)ext));
                    WCHAR_T_CONVERTER::print(file_name_to_save);
                    WCHAR_T_CONVERTER::print(file_full_name_to_save);

                    HANDLE hNewFileToSave = this->createEmptyFile(file_full_name_to_save);

                    /*if (hNewFileToSave == INVALID_HANDLE_VALUE) {
                        int err = GetLastError();
                        // report error...
                        return -err;
                    }*/
                    if (hNewFileToSave == INVALID_HANDLE_VALUE)
                        return;

                    DWORD bytesWritten;
                    WriteFile(hNewFileToSave, file_data, file_entry->size, &bytesWritten, nullptr);

                    CloseHandle(hNewFileToSave);

                }

                delete[] file_data;
            }
        }
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