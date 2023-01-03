#include "Windows.h"
#include "FAT32_info.hpp"
#include "wchar_t_converter.hpp"


class FILE_RECOVERER
{
public:
	FAT32_INFO* fat32_info;
	wchar_t* path_to_recover;
	wchar_t* path_to_save;

	unsigned char png_signature[9] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, '\0'};
	unsigned char png_iend[13] = { 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82, '\0' };

	FILE_RECOVERER(const char* path_to_recover, const char* path_to_save);
	void recoverFiles();
	void recoverFilesDataCarving(unsigned int number_of_clusters_to_check);

private:
	HANDLE createEmptyFile(wchar_t* file_full_name);
	HANDLE openDisk(wchar_t* path);
	bool saveFile(unsigned char* file_data, FILE_ENTRY* file_entry);
	bool checkIfFileExtensionValid(unsigned char* ext);
	wchar_t* createFileName(FILE_ENTRY* file_entry);
	wchar_t* createFilePath(wchar_t* path_to_save, FILE_ENTRY* file_entry);

};