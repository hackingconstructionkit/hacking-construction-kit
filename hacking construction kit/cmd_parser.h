#pragma once

#include <string>
#include <vector>

class CmdParser {
public:

	static std::vector<std::wstring> splitString(const std::wstring &s, wchar_t delim);

	static std::vector<std::string> CmdParser::splitString(const std::string &str, char delimiter);

	static std::wstring parseStringForId(const std::wstring& s);

	static std::wstring parseStringForToken(const std::wstring& s, const std::wstring& token, wchar_t delim = L':');

	static std::wstring parseStringForChannel(const std::wstring& s);

	static std::wstring strip(const std::wstring& s, const std::wstring& chars = L" ");

	static std::wstring stripCarriageReturn(const std::wstring& s);

	static std::wstring stripSpaces(const std::wstring& s);

	static bool isInteger(const std::wstring & s);

	static std::wstring getValueAsString(const std::wstring& command, const std::wstring &token, bool mandatory = true, const std::wstring &defaultValue = L"null");

	static int getValueAsInt(const std::wstring& command, const std::wstring &token, bool mandatory = true, int defaultValue = 0);

	static std::wstring getB64ValueAsString(const std::wstring& command, const std::wstring &token, bool mandatory = true, const std::wstring &defaultValue = L"null");


};