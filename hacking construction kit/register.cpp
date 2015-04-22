/*
 * Author: thirdstormofcythraul@outlook.com
 */
#include "register.h"

#include <cstdio>
#include <locale.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Winreg.h>
#pragma comment(lib, "Advapi32.lib")

#include "print.h"
#include "macro.h"

#include "memory_debug.h"

bool Register::createStringKey(HKEY hKeyParam, const wchar_t *path, const wchar_t *key, const wchar_t *value) {

	HKEY hKey;
	DWORD dwDisp = 0;
	LPDWORD lpdwDisp = &dwDisp;
	LONG iSuccess;


	iSuccess = RegCreateKeyEx(hKeyParam, path, 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, lpdwDisp);

	if(iSuccess == ERROR_SUCCESS)
	{
		iSuccess = RegSetValueEx (hKey, key, 0L, REG_SZ,(CONST BYTE*) value, wcslen(value) * sizeof(wchar_t) + 1);
		if (iSuccess != ERROR_SUCCESS){
			MYPRINTF( "RegSetValueEx failed with error: %d\n", iSuccess);
			return false;
		}
	} else {
		MYPRINTF( "RegOpenKeyEx failed with error: %d\n", iSuccess);
		return false;
	}
	return true;
}

bool Register::createDwordKey(HKEY hKeyParam, const wchar_t *path, const wchar_t *key, int value) {

	HKEY hKey;
	DWORD dwDisp = 0;
	LPDWORD lpdwDisp = &dwDisp;
	LONG iSuccess;


	iSuccess = RegCreateKeyEx(hKeyParam, path, 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, lpdwDisp);

	if(iSuccess == ERROR_SUCCESS)
	{
		iSuccess = RegSetValueEx (hKey, key, 0L, REG_DWORD, (const BYTE*)&value, sizeof(value));
		if (iSuccess != ERROR_SUCCESS){
			MYPRINTF( "RegSetValueEx failed with error: %d\n", iSuccess);
			return false;
		}
	} else {
		MYPRINTF( "RegOpenKeyEx failed with error: %d\n", iSuccess);
		return false;
	}
	return true;
}

std::wstring Register::getKey(HKEY hkey, const wchar_t *path, const wchar_t *mykey) {
	unsigned long type = REG_SZ, size = 1024;
	wchar_t res[1024] = TEXT("");
	HKEY key;


	if (RegOpenKeyEx(hkey, path, NULL, KEY_READ, &key) == ERROR_SUCCESS) {
		RegQueryValueEx(key, mykey, NULL, &type, (LPBYTE)&res[0], &size);
		RegCloseKey(key);
	}

	//A std:string  using the char* constructor.
	std::wstring ss(res);
	return ss;
}

int Register::getKeyAsInt(HKEY key, const wchar_t *path, const wchar_t *mykey, int defaultValue){
	std::wstring index = this->getKey(key, path, mykey);
	int value = _wtoi(index.c_str());
	if (value == 0){
		value = defaultValue;
	}
	return value;
}