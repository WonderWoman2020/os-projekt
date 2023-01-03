#include<iostream>
#include<Windows.h>

#include<string>

#include <fstream>

//#include"FAT32_info.hpp"
#include"helper_functions.hpp"
//#include "wchar_t_converter.hpp"
#include "file_recoverer.hpp"

using namespace std;


/*HANDLE createEmptyFile(wchar_t* file_full_name)
{
    HANDLE hNewFile = CreateFile(file_full_name,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);

    return hNewFile;
}*/


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

    CloseHandle(hdisk);

    /*for (int i = 0; i < 100; i++)
    {
        std::cout << fat32_info.getFAT(1)->getNextFileClusterNumber(i) << std::endl;
    }*/


    FILE_RECOVERER recoverer("\\\\.\\E:", "D:\\\\odzysk");

    recoverer.recoverFiles();


	return 0;
}