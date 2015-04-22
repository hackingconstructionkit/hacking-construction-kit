/*
 * Hacking construction kit -- A GPL Windows security library.
 * While this library can potentially be used for nefarious purposes, 
 * it was written for educational and recreational
 * purposes only and the author does not endorse illegal use.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: thirdstormofcythraul@outlook.com
 */
#pragma once

#include "tstring.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Wininet.h>

/**
* Class Ftp
* Use this class to manage FTP connexion
*/
class Ftp {
public:
	Ftp(wchar_t *url, int port = 21, wchar_t *user = 0, wchar_t *password = 0);

	~Ftp();

	std::wstring getCurrentDirectory();

	void setCurrentDirectory(std::wstring directory);

	void createDirectory(std::wstring directory);

	void uploadFile(std::wstring local, std::wstring remote);

private:
	HINTERNET m_handle;
	HINTERNET m_h;
};