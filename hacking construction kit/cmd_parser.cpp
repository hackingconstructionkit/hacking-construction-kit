#include "cmd_parser.h"

#include <sstream>
#include <algorithm>

#include "base64.h"

#include "memory_debug.h"

using namespace std;

std::vector<std::string> CmdParser::splitString(const std::string &str, char delimiter){
	vector<string> res;
	int start = 0;
	for(string::size_type i = 0; i < str.size(); ++i) {
		if (delimiter == str[i]){
			if (i > 0){
				res.push_back(str.substr(start, i - start));
			}
			start = i + 1;
		} else if (i == str.size() - 1){
			res.push_back(str.substr(start, i - start + 1));
		}
	}
	return res;
}

std::vector<std::wstring> CmdParser::splitString(const std::wstring &str, wchar_t delimiter){
	vector<wstring> res;
	int start = 0;
	for(wstring::size_type i = 0; i < str.size(); ++i) {
		if (delimiter == str[i]){
			if (i > 0){
				res.push_back(str.substr(start, i - start));
			}
			start = i + 1;
		} else if (i == str.size() - 1){
			res.push_back(str.substr(start, i - start + 1));
		}
	}
	return res;
}

std::wstring CmdParser::strip(const std::wstring& s, const std::wstring& chars) {
	if (s.size() == 0){
		return s;
	}

	size_t begin = 0;
	size_t end = s.size() - 1;
	for(; begin < s.size(); begin++)
		if(chars.find_first_of(s[begin]) == string::npos)
			break;
	for(; end > begin; end--)
		if(chars.find_first_of(s[end]) == string::npos)
			break;
	return s.substr(begin, end-begin + 1);
}

std::wstring CmdParser::parseStringForId(const std::wstring& s){
	return parseStringForToken(s, L"id");
}

std::wstring CmdParser::parseStringForChannel(const std::wstring& s){
	return parseStringForToken(s, L"channel");
}

std::wstring CmdParser::parseStringForToken(const std::wstring& s, const std::wstring& token, wchar_t delim){
	auto splitted = splitString(s, L' ');
	for(vector<int>::size_type i = 0; i != splitted.size(); i++) {
		auto tokens = splitString(splitted[i], delim);
		if (tokens.size() == 2){
			if (tokens[0] == token){
				return stripSpaces(stripCarriageReturn(tokens[1]));
			}
		}
	}
	return L"null";
}

std::wstring CmdParser::stripCarriageReturn(const std::wstring& s){
	return strip(strip(s, L"\r"), L"\n");
}

std::wstring CmdParser::stripSpaces(const std::wstring& s){
	return strip(s, L" ");
}

std::wstring CmdParser::getValueAsString(const std::wstring& command, const std::wstring &token, bool mandatory, const std::wstring &defaultValue){
	wstring value = CmdParser::parseStringForToken(command, token);
	if (value == L"null"){
		if (mandatory){
			throw exception("token mandatory");
		}
		value = defaultValue;
	}

	return value;
}

std::wstring CmdParser::getB64ValueAsString(const std::wstring& command, const std::wstring &token, bool mandatory, const std::wstring &defaultValue){
	wstring value = CmdParser::parseStringForToken(command, token);
	if (value == L"null" || value == L""){
		if (mandatory){
			throw exception("token mandatory");
		}
		value = defaultValue;
	} else {
		Base64 b64;
		size_t length;
		wchar_t *decodedCommand = b64.decode(value.c_str(), value.size(), &length);
		if (decodedCommand == 0){
			return defaultValue;
		}
		wstring decodedCommandAsString;
		decodedCommandAsString.append(decodedCommand, length - 1);
		delete[] decodedCommand;
		value = decodedCommandAsString;
		if (value == L"null" || value == L""){
			if (mandatory){
				throw exception("token mandatory");
			}
			value = defaultValue;
		}
	}

	return value;
}

int CmdParser::getValueAsInt(const std::wstring& command, const std::wstring &token, bool mandatory, int defaultValue){
	wstring value = getValueAsString(command, token, mandatory);
	if (value == L"null"){
		return defaultValue;
	}
	if (!isInteger(value)){
		throw exception("value is not an integer");
	}
	return stoi(value);
}

bool CmdParser::isInteger(const std::wstring & s){
   if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

   wchar_t * p;
   wcstol(s.c_str(), &p, 10) ;

   return (*p == 0) ;
}