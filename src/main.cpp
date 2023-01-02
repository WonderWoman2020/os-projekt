#include<iostream>
#include<Windows.h>

#include<string>

#include <fstream>

#include"FAT32_info.hpp"
#include"helper_functions.hpp"
#include "wchar_t_converter.hpp"

using namespace std;


HANDLE createEmptyFile(wchar_t* file_full_name)
{
    HANDLE hNewFile = CreateFile(file_full_name,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);

    return hNewFile;
}

unsigned char* getFileExtension(unsigned char* file_name) //dodaæ do FILE_ENTRY, w zale¿noœci czy dir czy nie (lub brak rozszerzenia), zwraca rozszerzenie lub nullptr
{
    unsigned char* extension = new unsigned char[4];
    std::fill(extension, extension + 4, 0);

    //unsigned int len_name = std::strlen((const char*)file_name);
    std::copy(file_name + 11 - 3, file_name + 11, extension);

    for(int i=0; i<std::strlen((const char*)extension); i++)
        extension[i] = std::tolower(extension[i]);

    return extension;
}

unsigned char* getFileName(unsigned char* file_name)
{
    unsigned char* name = new unsigned char[9];
    std::fill(name, name + 9, 0);
    std::copy(file_name, file_name + 8, name);
    return name;
}


int main()
{

    HANDLE hdisk = CreateFile(L"\\\\.\\E:",
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0, NULL);
    if (hdisk == INVALID_HANDLE_VALUE) {
        int err = GetLastError();
        // report error...
        return -err;
    }

    LARGE_INTEGER position = { 0 };
    BOOL ok = SetFilePointerEx(hdisk, position, nullptr, FILE_BEGIN);
    //assert(ok);

    BYTE buf[65536];
    DWORD read;
    ok = ReadFile(hdisk, buf, 65536, &read, nullptr);
    //assert(ok);
    //std::cout << ok << std::endl << (short)buf[510]<< ", " << (short)buf[511] << std::endl;
    //cin >> ok;

    FAT32_INFO fat32_info(hdisk);
    std::cout << fat32_info.toString() << std::endl;

    fat32_info.showFilesEntries();

    /*for (int i = 0; i < 100; i++)
    {
        std::cout << fat32_info.getFAT(1)->getNextFileClusterNumber(i) << std::endl;
    }*/



    wchar_t* file_path = WCHAR_T_CONVERTER::convert("D:\\\\odzysk"); // WA¯NE!
    CreateDirectory(file_path, nullptr);

    wchar_t* file_name = WCHAR_T_CONVERTER::convert("\\\\1.txt");
    wchar_t* file_full_name = WCHAR_T_CONVERTER::concatenate(file_path, file_name);

    WCHAR_T_CONVERTER::print(file_path);
    WCHAR_T_CONVERTER::print(file_name);
    WCHAR_T_CONVERTER::print(file_full_name);


    HANDLE hNewFile = createEmptyFile(file_full_name);

    if (hNewFile == INVALID_HANDLE_VALUE) {
        int err = GetLastError();
        // report error...
        return -err;
    }


    for (int i = 0; i < fat32_info.files_and_dirs.size(); i++)
    {
        FILE_ENTRY* file_entry = fat32_info.files_and_dirs.at(i);
        if (!file_entry->isFolder())
        {
            unsigned char* file_data = fat32_info.readFile(hdisk, fat32_info.getFAT(1), file_entry);
            if (file_data != nullptr)
            {
                //std::cout << file_entry->toString() << std::endl;
                unsigned char* name = getFileName(file_entry->name);
                unsigned char* ext = getFileExtension(file_entry->name);
                if (name[0] == 0xE5)
                    name[0] = 'A';
                std::cout << name << std::endl;
                std::cout << ext << std::endl;

                if (std::strlen((const char*)ext) != 0 && ext[0] != ' ') // spacje s¹ w rozszerzeniu .gitignore, trzeba do niego poprawiæ albo go ignorowaæ
                {
                    std::cout << "Mo¿na zapisywaæ " << std::strlen((const char*)ext) << std::endl;
                    wchar_t* backslash = WCHAR_T_CONVERTER::convert("\\\\");
                    wchar_t* file_name_to_save = WCHAR_T_CONVERTER::convert((const char*)name);
                    wchar_t* file_full_name_to_save = WCHAR_T_CONVERTER::concatenate(file_path, backslash);
                    unsigned int len_name_to_save = std::find(file_name_to_save, file_name_to_save+11, ' ') - file_name_to_save;
                    file_full_name_to_save = WCHAR_T_CONVERTER::concatenate(file_full_name_to_save, file_name_to_save, std::wcslen(file_full_name_to_save), len_name_to_save);
                    file_full_name_to_save = WCHAR_T_CONVERTER::concatenate(file_full_name_to_save, WCHAR_T_CONVERTER::convert("."));
                    file_full_name_to_save = WCHAR_T_CONVERTER::concatenate(file_full_name_to_save, WCHAR_T_CONVERTER::convert((const char*)ext));
                    WCHAR_T_CONVERTER::print(file_name_to_save);
                    WCHAR_T_CONVERTER::print(file_full_name_to_save);

                    HANDLE hNewFileToSave = createEmptyFile(file_full_name_to_save);

                    if (hNewFileToSave == INVALID_HANDLE_VALUE) {
                        int err = GetLastError();
                        // report error...
                        return -err;
                    }

                    DWORD bytesWritten;
                    WriteFile(hNewFileToSave, file_data, file_entry->size, &bytesWritten, nullptr);

                    CloseHandle(hNewFileToSave);

                }

                delete[] file_data;
            }
        }
    }

    //std::cout << fat32_info.getFAT(1)->getNextFileClusterNumber(14) << std::endl;

    CloseHandle(hdisk);
    CloseHandle(hNewFile);

	return 0;
}