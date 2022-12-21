#include<iostream>
#include<Windows.h>

#include<string>

using namespace std;

int convertBytesToInt(unsigned char* bytes, int len);

/*class PARTITION_INFO
{
public:
    int number_of_sectors;
    int LBA_begin_address_val;
    unsigned char FS_type_code;

    PARTITION_INFO(unsigned char partition[16])
    {
        unsigned char buffer[4] = {0};
        std::copy(partition+12, partition+16, buffer);
        this->number_of_sectors = convertBytesToInt(buffer, 4);
        std::copy(partition + 8, partition + 12, buffer);
        this->LBA_begin_address_val = convertBytesToInt(buffer, 4);
        this->FS_type_code = partition[4];
    }

    std::string toString()
    {
        return "Number of sectors: " + std::to_string(this->number_of_sectors) + "\n"
            + "LBA begin addres: " + std::to_string(this->LBA_begin_address_val) + "\n"
            + "File system type code: " + std::to_string(this->FS_type_code) + "\n";
    }

};

class MBR_INFO
{
public:
    unsigned char boot_code[446];
    PARTITION_INFO partitions[4];
    unsigned char sanity_check_code[2];

};

class FAT32_INFO
{
public:
    MBR_INFO mbr;
};*/


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
    std::cout << ok << std::endl << (short)buf[510]<< ", " << (short)buf[511] << std::endl;
    //cin >> ok;

    unsigned char buffer[4];
    std::copy(buf + 36, buf + 36+4, buffer);
    std::cout << bytesToFormattedString(buffer, 4);
    //std::cout << convertBytesToInt(buffer, 4) << std::endl;

    std::copy(buf + 510, buf + 510+2, buffer);
    std::cout << bytesToFormattedString(buffer, 2);

	return 0;
}