#include "wchar_t_converter.hpp"



WCHAR_T_CONVERTER::WCHAR_T_CONVERTER()
{

}


wchar_t* WCHAR_T_CONVERTER::convert(const char* text)
{
    unsigned int text_len = std::strlen(text);
    wchar_t* converted_text = new wchar_t[text_len + 1];
    std::fill(converted_text, converted_text + text_len + 1, 0);
    std::mbstowcs(converted_text, text, text_len);
    return converted_text;
}


wchar_t* WCHAR_T_CONVERTER::concatenate(wchar_t* arr1, wchar_t* arr2, unsigned int len1, unsigned int len2)
{
    wchar_t* file_full_name = new wchar_t[len1 + len2 + 1];
    std::fill(file_full_name, file_full_name + len1 + len2 + 1, 0);

    std::copy(arr1, arr1 + len1, file_full_name);
    std::copy(arr2, arr2 + len2, file_full_name + len1);

    return file_full_name;
}

wchar_t* WCHAR_T_CONVERTER::concatenate(wchar_t* arr1, wchar_t* arr2)
{
    unsigned int len1 = std::wcslen(arr1);
    unsigned int len2 = std::wcslen(arr2);

    return WCHAR_T_CONVERTER::concatenate(arr1, arr2, len1, len2);
}

void WCHAR_T_CONVERTER::print(wchar_t* wide_text)
{
    for (int i = 0; i < std::wcslen(wide_text); i++)
        std::cout << (char)wide_text[i];
    std::cout << std::endl;
}