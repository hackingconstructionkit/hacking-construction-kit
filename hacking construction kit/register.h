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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>

#include "tstring.h"

// Manage windows register
class Register {
public:
	// create a key, value in registry data base
	bool createStringKey(HKEY hKeyParam, const wchar_t *path, const wchar_t *key, const wchar_t *value);
	bool createDwordKey(HKEY hKeyParam, const wchar_t *path, const wchar_t *key, int value);

	// get a key from register data base
	std::wstring getKey(HKEY key, const wchar_t *path, const wchar_t *mykey);

	// get a key from register data base
	int getKeyAsInt(HKEY key, const wchar_t *path, const wchar_t *mykey, int defaultValue);

};