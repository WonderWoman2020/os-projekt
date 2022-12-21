#include<iostream>
#include<Windows.h>

#include<string>

#include"FAT32_info.hpp"
#include"helper_functions.hpp"

using namespace std;




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


    /*unsigned char sector[512];
    std::copy(buf, buf + 512, sector);
    BOOT_SECTOR_INFO boot_sector_info(sector);
    std::cout << boot_sector_info.toString() << std::endl;*/

    FAT32_INFO fat32_info(hdisk);
    std::cout << fat32_info.toString() << std::endl;

    fat32_info.showFilesEntries();
    

	return 0;
}