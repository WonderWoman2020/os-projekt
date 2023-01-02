#define _CRT_SECURE_NO_WARNINGS
#include<string>
#include<iostream>


class WCHAR_T_CONVERTER
{
public:

	WCHAR_T_CONVERTER();
	static wchar_t* convert(const char* text);
	static wchar_t* concatenate(wchar_t* arr1, wchar_t* arr2, unsigned int len1, unsigned int len2);
	static wchar_t* concatenate(wchar_t* arr1, wchar_t* arr2);

	static void print(wchar_t* wide_text);
};