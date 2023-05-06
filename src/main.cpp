#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<Windows.h>

#include<string>

#include <fstream>

#include"helper_functions.hpp"
#include "file_recoverer.hpp"

using namespace std;


int main()
{
    unsigned char input[512] = { 0 };
    unsigned char disk_name = 0;
    bool input_valid = false;
    while (!input_valid)
    {
        while (!std::isalpha(input[0]))
        {
            std::cout << "Z jakiego dysku FAT32 chcesz odzyskac dane? (Wpisz litere nazwy dysku, np. E): ";
            cin >> input;

            if (!std::isalpha(input[0]))
            {
                std::cout << "\nPodano bledne dane w miejsce nazwy dysku. Wpisz nazwe dysku jeszcze raz\n"
                    "(Wskazowka: Upewnij sie, ze wpisujesz tylko litere bedaca nazwa dysku, bez zadnych dodatkowych znakow)" << std::endl;
            }
        }
        disk_name = std::toupper(input[0]);
        std::cout << "Wybrany dysk: " << disk_name << std::endl;

        bool path_valid = false;

        while (!path_valid)
        {
            std::cout << "\nGdzie chcesz zapisac odzyskane dane? (Podaj sciezke, np. D:\\odzyskane): ";
            cin >> (input + 1);
            if (disk_name == std::toupper(input[1]))
            {
                std::cout << "\nUwaga! Na dysku, z ktorego chcesz odzyskac dane, nie powinines nic zapisywac, poki nie przywrocisz z niego danych.\n"
                    "Mogloby to spowodowac bezpowrotna utrate czesci plikow do odzyskania, poniewaz moglby one zostac nadpisane przez nowo zapisane dane.\n"
                    "Wybierz inny dysk do zapisywania odzyskiwanych danych." << std::endl;
                continue;
            }
            
            if (input[2] == ':' && input[3] == '\\')
                path_valid = true;
            else
                std::cout << "\nPodano bledne dane w miejsce sciezki do zapisu. Podaj sciezke jeszcze raz\n"
                "(Wskazowka: Upewnij sie, ze wpisujesz sciezke (minimum to np. D:\\), a nie tylko nazwe dysku, na ktorym chcesz zapisac dane)" << std::endl;
        }

        std::cout << "Wybrana sciezka zapisu: " << (input + 1) << std::endl;
        input_valid = true;
    }

    std::string temp_path_to_recover = std::string("\\\\.\\") + std::string((char*)&disk_name, 1) + std::string(":");
    unsigned char* path_to_recover = new unsigned char[10];
    std::fill(path_to_recover, path_to_recover + 10, 0);
    std::strcpy((char*)path_to_recover, temp_path_to_recover.c_str());
    
    //std::cout << path_to_recover << std::endl;

    unsigned char* path_to_save = new unsigned char[2048];
    std::fill(path_to_save, path_to_save + 2048, 0);

    int j = 0;
    for (int i = 1; i < 512; i++)
    {
        path_to_save[j] = input[i];
        j++;
        if (input[i] == '\\')
        {
            for (int a = 0; a < 3; a++)
            {
                path_to_save[j] = '\\';
                j++;
            }
        }
    }

    //std::cout << path_to_save << std::endl;

    std::cout << "\nPokazac informacje odczytana z systemu plikow FAT32? (T/n): ";
    std::cin >> input;

    bool show_fat32_info = false;
    if (input[0] == 'T')
        show_fat32_info = true;

    std::cout << "\nIle MB dysku przeszukac uzywajac techniki 'data carving'? (Wpisz liczbe calkowita): ";
    std::fill(input, input + 512, 0);
    cin >> input;
    unsigned int data_carving_limit = std::atoi((const char*)input);
    std::cout << data_carving_limit << std::endl;

    std::cout << "\nJaki ma byc maksymalny rozmiar (w KB) plikow szukanych uzywajac techniki 'data carving'? (Wpisz liczbe calkowita): ";
    std::fill(input, input + 512, 0);
    cin >> input;
    unsigned int carving_file_size_limit = std::atoi((const char*)input);
    std::cout << carving_file_size_limit << std::endl;
    unsigned int file_size_limit_bytes = carving_file_size_limit * 1024;
    std::cout << file_size_limit_bytes << std::endl;

    FILE_RECOVERER recoverer((const char*)path_to_recover, (const char*)path_to_save);

    unsigned int data_carving_cluster_number_limit = data_carving_limit * 1024 * 1024 / recoverer.fat32_info->boot_sector->getClusterSize();
    std::cout << "Zostanie przeszukanych " << data_carving_cluster_number_limit << " klastrow" << std::endl;

    if (show_fat32_info)
    {
        std::cout << std::endl;
        std::cout << recoverer.fat32_info->toString() << std::endl;
        recoverer.fat32_info->showFilesEntries();
        /*FILE_ENTRY* entry = nullptr;
        for (int i = 0; i < recoverer.fat32_info->files_and_dirs.size(); i++)
        {
            entry = recoverer.fat32_info->files_and_dirs.at(i);
            if (entry != nullptr)
            {
                auto ext = entry->getFileExtension();
                if (ext == nullptr)
                    continue;
                auto file_name = recoverer.createFileName(entry);
                if(file_name != nullptr)
                    WCHAR_T_CONVERTER::print(file_name);
            }
        }*/
    }

    std::cout << "\nOdzyskiwanie plikow z uzyciem informacji z systemu plikow: " << std::endl;
    recoverer.recoverFiles();
    std::cout << "\nOdzyskiwanie plikow przez 'data carving': " << std::endl;
    recoverer.recoverFilesDataCarving(data_carving_cluster_number_limit, file_size_limit_bytes);

    std::cout << "\nOdzyskiwanie plikow zakonczono. Aby wyjsc, nacisnij enter" << std::endl;
    std::cin.get();
    std::cin.get();

	return 0;
}