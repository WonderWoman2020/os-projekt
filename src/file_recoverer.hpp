#include "Windows.h"
#include "FAT32_info.hpp"
#include "wchar_t_converter.hpp"


class FILE_RECOVERER
{
public:
	FAT32_INFO* fat32_info;
	wchar_t* path_to_recover;
	wchar_t* path_to_save;

	FILE_RECOVERER(const char* path_to_recover, const char* path_to_save);
	void recoverFiles();
	void recoverFilesDataCarving();

private:
	HANDLE createEmptyFile(wchar_t* file_full_name);
	HANDLE openDisk(wchar_t* path);
	bool saveFile(unsigned char* file_data, FILE_ENTRY* file_entry);
	bool checkIfFileExtensionValid(unsigned char* ext);
	wchar_t* createFileName(FILE_ENTRY* file_entry);
	wchar_t* createFilePath(wchar_t* path_to_save, FILE_ENTRY* file_entry);

};