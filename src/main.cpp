#include<iostream>
#include<Windows.h>

#include<string>

using namespace std;


int convertBytesToInt(unsigned char* bytes, int len);
std::string bytesToFormattedString(unsigned char* bytes, int len);

class BPB_INFO
{
public:
    unsigned short bytes_per_sector;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char media_descriptor;
    unsigned int all_sectors_on_volume;
    unsigned int sectors_per_fat_table;
    unsigned short fs_version;
    unsigned int root_first_cluster;
    unsigned short fs_info_sector;


    BPB_INFO(unsigned char data[512])
    {
        this->bytes_per_sector = this->calculateField(data, 11, 2);
        this->sectors_per_cluster = this->calculateField(data, 13, 1);
        this->reserved_sectors = this->calculateField(data, 14, 1);
        this->media_descriptor = this->calculateField(data, 21, 1);
        this->all_sectors_on_volume = this->calculateField(data, 0x20, 4);
        this->sectors_per_fat_table = this->calculateField(data, 0x24, 4);
        this->fs_version = this->calculateField(data, 0x2A, 2);
        this->root_first_cluster = this->calculateField(data, 0x2C, 4);
        this->fs_info_sector = this->calculateField(data, 0x30, 2);
    }

    unsigned int calculateField(unsigned char data[512], short offset, short size)
    {
        unsigned char* buffer = new unsigned char[size];
        std::copy(data + offset, data + offset + size, buffer);
        return convertBytesToInt(buffer, size);
    }

    std::string toString()
    {
        return "** BPB INFO START **\n\n"
            "Bytes per sector: " + std::to_string(this->bytes_per_sector) + "\n"
            + "Sectors per cluster: " + std::to_string(this->sectors_per_cluster) + "\n"
            + "Reserved sectors: " + std::to_string(this->reserved_sectors) + "\n"
            + "Media descriptor: " + std::to_string(this->media_descriptor) + "\n"
            + "All sectors on volume: " + std::to_string(this->all_sectors_on_volume) + "\n"
            + "Sectors per FAT table: " + std::to_string(this->sectors_per_fat_table) + "\n"
            + "File system version: " + std::to_string(this->fs_version) + "\n"
            + "Root first cluster: " + std::to_string(this->root_first_cluster) + "\n"
            + "File system info sector: " + std::to_string(this->fs_info_sector) + "\n"
            "\n** BPB INFO END **\n";
    }

};

class BOOT_SECTOR_INFO
{
public:
    unsigned char sanity_check_code[2];
    BPB_INFO* bpb;
    unsigned char oem_id[9];

    BOOT_SECTOR_INFO(unsigned char data[512])
    {
        this->bpb = new BPB_INFO(data);
        std::copy(data + 3, data + 11, this->oem_id);
        std::copy(data + 510, data + 512, this->sanity_check_code);
    }

    std::string toString()
    {
        return "--- BOOT SECTOR INFO START ---\n\n"
            "OEM ID: " + std::string((char*)this->oem_id) + "\n"
            +"\n"+ bpb->toString()+"\n"
            + "Sanity check code: " + bytesToFormattedString(this->sanity_check_code, 2)
            +"\n--- BOOT SECTOR INFO END ---\n";
    }

};

class FAT32_INFO
{
public:
    BOOT_SECTOR_INFO boot_sector;
};


int convertBytesToInt(unsigned char* bytes, int len)
{
    int result = 0;
    for (int i = 0; i < len; i++)
        result = result | (bytes[i] << (i * 8));

    return result;
}

std::string hexValueOfFourBits(unsigned char halfByte)
{
    string s = "";
    if (halfByte >= 10)
        s = s + char('A' + halfByte - 10);
    else
        s = std::to_string(halfByte);
    return s;
}

std::string bytesToFormattedString(unsigned char* bytes, int len)
{
    std::string text = "";
    for (int i = 0; i < len; i++)
    {
        unsigned char byte = bytes[i];
        unsigned char lowerHalfOfByte = byte & 0x0F;
        unsigned char upperHalfOfByte = (byte & 0xF0) >> 4;
        //cout << (short)upperHalfOfByte << endl;
        //cout << (short)lowerHalfOfByte << endl;
        text = text + "0x" + hexValueOfFourBits(upperHalfOfByte) + hexValueOfFourBits(lowerHalfOfByte) + " ";
    }
    text = text + "\n";
    return text;
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

    /*unsigned char buffer[4];
    std::copy(buf + 36, buf + 36+4, buffer);
    std::cout << bytesToFormattedString(buffer, 4);*/
    //std::cout << convertBytesToInt(buffer, 4) << std::endl;

    /*std::copy(buf + 510, buf + 510 + 2, buffer);
    std::cout << bytesToFormattedString(buffer, 2);*/

    /*unsigned char name[9] = {0};
    std::copy(buf + 3, buf + 11, name);
    std::cout << name << std::endl;*/

    unsigned char sector[512];
    std::copy(buf, buf + 512, sector);
    BOOT_SECTOR_INFO boot_sector_info(sector);
    std::cout << boot_sector_info.toString() << std::endl;


	return 0;
}