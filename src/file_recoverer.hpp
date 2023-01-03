#define _CRT_SECURE_NO_WARNINGS
#include "Windows.h"
#include "FAT32_info.hpp"
#include "wchar_t_converter.hpp"


class FILE_RECOVERER
{
public:
	FAT32_INFO* fat32_info;
	wchar_t* path_to_recover;
	wchar_t* path_to_save;
	unsigned int data_carving_saves;

	unsigned char png_signature[9] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, '\0'};
	unsigned char png_iend[13] = { 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82, '\0' };

	//unsigned char bmp_signature[3] = { 0x42, 0x4D, '\0' };

	FILE_RECOVERER(const char* path_to_recover, const char* path_to_save);
	void recoverFiles();
	void recoverFilesDataCarving(unsigned int number_of_clusters_to_check);

private:
	HANDLE createEmptyFile(wchar_t* file_full_name);
	HANDLE openDisk(wchar_t* path);
	bool saveRecoveredFile(unsigned char* file_data, FILE_ENTRY* file_entry);
	bool saveRecoveredFile(unsigned char* file_data, unsigned int data_size, unsigned char* ext);
	bool saveFile(unsigned char* file_data, unsigned int data_size, wchar_t* file_path);
	bool checkIfFileExtensionValid(unsigned char* ext);
	wchar_t* createFilePath(wchar_t* path_to_save, wchar_t* file_name);
	wchar_t* createFileName(FILE_ENTRY* file_entry);
	wchar_t* createFileName(unsigned char* ext);
};