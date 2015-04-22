/*
 * Author: thirdstormofcythraul@outlook.com
 */
#include "crypted_global.h"

#include <stdio.h>

#include "base64.h"
#include "rc4.h"
#include "macro.h"

#include "memory_debug.h"

#define BUFFER_STRING_SIZE 100

using namespace std;

CryptedGlobal const CryptedGlobal::m_instance = CryptedGlobal();



CryptedGlobal::CryptedGlobal(){
}

CryptedGlobal::~CryptedGlobal(){

}

const CryptedGlobal &CryptedGlobal::get(){
	return m_instance;
}

const wchar_t *CryptedGlobal::decodeNewString(const char *str){
	
	size_t length;	
	Base64 base64encoder;

	MYPRINTF("decode string: %s\n", str);

	char *base64 = (char *)base64encoder.decode(str, strlen(str), &length);

	char buffer[BUFFER_STRING_SIZE];
	char *key2 = KEY2;
	char *key1 = KEY1;
	sprintf_s(buffer, BUFFER_STRING_SIZE, "%s%s", key1, key2);

	RC4 rc4(buffer);
	rc4.encode(base64, length);

	string s;
	s.append(base64, length);
	delete[] base64;

	std::wstring wide = tosW(s);

	wchar_t *newString = new wchar_t[length * 2 + 2];
	// TODO: base64 is not a string
	wcsncpy_s(newString, length * 2 + 2, wide.c_str(), length * 2);
	newString[length] = L'\0';	

	MYPRINTF("string decoded: %w\n", newString);

	return newString;

}

