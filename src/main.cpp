#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<Windows.h>

#include<string>

#include <fstream>

#include"FAT32_info.hpp"
#include"helper_functions.hpp"

using namespace std;

wchar_t* convertCHAR2WCHAR_T(const char* text)
{
    unsigned int text_len = std::strlen(text);
    wchar_t* converted_text = new wchar_t[text_len + 1];
    std::fill(converted_text, converted_text + text_len + 1, 0);
    std::mbstowcs(converted_text, text, text_len);
    return converted_text;
}

wchar_t* concatenateArraysWCHAR_T(wchar_t* arr1, wchar_t* arr2)
{
    unsigned int len_path = std::wcslen(arr1);
    unsigned int len_name = std::wcslen(arr2);

    wchar_t* file_full_name = new wchar_t[len_path + len_name + 1];
    std::fill(file_full_name, file_full_name + len_path + len_name + 1, 0);

    std::copy(arr1, arr1 + len_path, file_full_name);
    std::copy(arr2, arr2 + len_name, file_full_name + len_path);

    return file_full_name;
}

void printArrayWCHAR_T(wchar_t* arr)
{
    for (int i = 0; i < std::wcslen(arr); i++)
        std::cout << (char)arr[i];
    std::cout << std::endl;
}

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
    unsigned char extension[4] = { 0 };
    unsigned int len_name = std::strlen((const char*)file_name);
    std::copy(file_name + len_name - 3, file_name + len_name, extension);

    for(int i=0; i<std::strlen((const char*)extension); i++)
        extension[i] = std::tolower(extension[i]);

    return extension;
}

unsigned char* getFileName(unsigned char* file_name)
{
    unsigned char name[9] = { 0 };
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



    wchar_t* file_path = convertCHAR2WCHAR_T("D:\\\\odzysk");
    CreateDirectory(file_path, nullptr);

    wchar_t* file_name = convertCHAR2WCHAR_T("\\\\1.txt");
    wchar_t* file_full_name = concatenateArraysWCHAR_T(file_path, file_name);

    printArrayWCHAR_T(file_path);
    printArrayWCHAR_T(file_name);
    printArrayWCHAR_T(file_full_name);

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
                std::cout << file_entry->toString() << std::endl;
                delete[] file_data;
            }
        }
    }


    CloseHandle(hdisk);
    CloseHandle(hNewFile);

	return 0;
}