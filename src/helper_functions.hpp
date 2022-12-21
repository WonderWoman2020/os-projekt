#include<string>
#include<Windows.h>


int convertBytesToInt(unsigned char* bytes, int len);
std::string hexValueOfFourBits(unsigned char halfByte);
std::string bytesToFormattedString(unsigned char* bytes, int len);

unsigned int readDisk(HANDLE hdisk, LARGE_INTEGER position, BYTE* buffer, unsigned int buf_size);