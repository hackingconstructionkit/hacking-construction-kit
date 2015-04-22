#include "tstring.h"
#include <sstream>

#include "macro.h"

#if defined _UNICODE || defined UNICODE
	std::tstring tosW(std::string s){
		std::wstringstream ws;
		ws << s.c_str();
		std::wstring sLogLevel = ws.str();
		return sLogLevel;
	}
	std::wstring lToW(unsigned long i){
		wchar_t buf[1024];
		std::swprintf(buf, COUNTOF(buf), L"%u", i);
		return buf;
	}
	std::string tosS(std::tstring w){
		std::string s;
		s.assign(w.begin(), w.end());
		return s;
	}
	char *wToc(const wchar_t *wstr){
		char* ascii = new char[wcslen(wstr) + 1];
		size_t outSize;
		wcstombs_s(&outSize, ascii, wcslen(wstr) + 1, wstr, _TRUNCATE);
		return ascii;
	}

#else
	std::tstring tosW(std::string s){
		return s;
	}
#endif