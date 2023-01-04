#include"helper_functions.hpp"


int convertBytesToInt(unsigned char* bytes, int len)
{
    int result = 0;
    for (int i = 0; i < len; i++)
        result = result | (bytes[i] << (i * 8));

    return result;
}

std::string hexValueOfFourBits(unsigned char halfByte)
{
    std::string s = "";
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

unsigned int readDisk(HANDLE hdisk, LARGE_INTEGER position, BYTE* buffer, unsigned int buf_size)
{
    BOOL ok = SetFilePointerEx(hdisk, position, nullptr, FILE_BEGIN);

    DWORD read;
    ok = ReadFile(hdisk, buffer, buf_size, &read, nullptr);
    return read;
}