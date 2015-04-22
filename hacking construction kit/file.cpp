/*
 * Author: thirdstormofcythraul@outlook.com
 */
#include "file.h"

#include <stack>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>

#include "macro.h"

using namespace std;

wstring File::fileTimeToString(FILETIME *filetime){
	SYSTEMTIME st;
	wchar_t szLocalDate[255], szLocalTime[255];

	FileTimeToLocalFileTime(filetime, filetime);
	FileTimeToSystemTime(filetime, &st );
	GetDateFormat(LOCALE_INVARIANT, DATE_SHORTDATE, &st, NULL, szLocalDate, 255);
	GetTimeFormat(LOCALE_INVARIANT, 0, &st, NULL, szLocalTime, 255);

	wstring date;
	date.append(szLocalDate);
	date.append(L" ");
	date.append(szLocalTime);

	return date;
}

bool File::findFile(std::vector<std::wstring>& files, std::wstring path,
	std::wstring filename, DWORD &totalSize,
	DWORD minSize, DWORD maxSize, bool exactSearch) {
	WIN32_FIND_DATA ffd;

	stack<wstring> directories;
	directories.push(path);

	while (!directories.empty()) {
		path = directories.top();
		wstring spec = path + L"\\*";
		directories.pop();

		HANDLE hFind = FindFirstFile(spec.c_str(), &ffd);
		if (hFind == INVALID_HANDLE_VALUE)  {
			MYPRINTF("FindNextFile: INVALID_HANDLE_VALUE\n");
			continue;
		} 

		do {
			wstring tempFilename = ffd.cFileName;
			if (wcscmp(ffd.cFileName, L".") != 0 && 
				wcscmp(ffd.cFileName, L"..") != 0) {
					if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
						directories.push(path + L"\\" + ffd.cFileName);
					} else {
						if (filename.empty() || compare(tempFilename, filename, exactSearch)){
							if (minSize <= ffd.nFileSizeLow){
								if (maxSize == 0 || maxSize >= ffd.nFileSizeLow){
									totalSize += ffd.nFileSizeLow;
									files.push_back(path + L"\\" + ffd.cFileName);
									addFileInfoToStr(ffd, files);
								}
							}
						}
					}
			}
		} while (FindNextFile(hFind, &ffd) != 0);

		if (GetLastError() != ERROR_NO_MORE_FILES) {
			MYPRINTF("FindNextFile: %d\n", GetLastError());
			FindClose(hFind);
			return false;
		}

		FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;
	}

	return true;
}

void File::addFileInfoToStr(WIN32_FIND_DATA& ffd, std::vector<std::wstring>& files){
		
		double size = (ffd.nFileSizeHigh * ((double)MAXDWORD + 1)) + ffd.nFileSizeLow;
		wchar_t myStringBuffer[20];
		swprintf(myStringBuffer, COUNTOF(myStringBuffer), L"%.0f", size);
		wstring sizeAsString;
		sizeAsString.append(myStringBuffer);
		files.push_back(sizeAsString);
		files.push_back(fileTimeToString(&ffd.ftLastWriteTime));
}

bool File::compare(std::wstring str1, std::wstring str2, bool exact){
	if (exact){
		return str1 == str2;
	}
	return str1.find(str2) != std::string::npos;

}


bool File::replaceInFile(const std::wstring& filein, const std::wstring& fileout, const char *searched, unsigned int size, const char *replace){

	string replaceString(replace);
	if (replaceString.size() < size){
		replaceString.resize(size, '\0');
	} else if (replaceString.size() > size){
		return false;
	}

	ofstream ofile;
	ofile.open (fileout, ios::binary|ios::trunc);
	if (!ofile.is_open()){
		return false;
	}

	ifstream ifile;
	ifile.open(filein, ios::in|ios::binary|ios::ate);
	if (!ifile.is_open()){
		return false;
	}

	streampos length = ifile.tellg();
	
	char *buffer = new char [static_cast<unsigned int>(length)];

	MYPRINTF("Reading %u characters...\n", static_cast<unsigned int>(length));
	// read data as a block:
	ifile.seekg(0, ios::beg);
	ifile.read (buffer, length);

	const char *replaceConst = replaceString.c_str();

	bool res = false;

	long totalLimit = static_cast<unsigned int>(length);
	long total = static_cast<unsigned int>(length) - size;
	long searchedLength = strlen(searched);
	for (char *it = buffer; it < buffer + totalLimit; it++){
		if (it < buffer + total){
			if(strncmp(it, searched, searchedLength) == 0){
				ofile.write(replaceConst, size);
				it = it + size - 1;
				res = true;
			} else {
				ofile.write(it, 1);
			}
		} else {
			ofile.write(it, 1);
		}
	}

	delete [] buffer;

	return res;
}


std::wstring File::displayVolumePaths(wchar_t *volumeName) {
	DWORD  charCount = MAX_PATH + 1;
	wchar_t *names     = 0;
	wchar_t *nameIdx   = 0;
	BOOL   success   = FALSE;

	for (;;) {
		//
		//  Allocate a buffer to hold the paths.
		names = new wchar_t [charCount * sizeof(wchar_t)];
		//
		//  Obtain all of the paths
		//  for this volume.
		success = GetVolumePathNamesForVolumeName(volumeName, names, charCount, &charCount);
		if (success) {
			break;
		}
		if (GetLastError() != ERROR_MORE_DATA){
			break;
		}
		//
		//  Try again with the
		//  new suggested size.
		delete [] names;
		names = 0;
	}
	std::wstring path;
	if (success) {
		//
		//  Display the various paths.
		for (nameIdx = names; nameIdx[0] != L'\0'; nameIdx += wcslen(nameIdx) + 1) {
			//wprintf(L"  %s", nameIdx);
			path = nameIdx;
		}
		//wprintf(L"\n");
	}

	if (names != NULL) {
		delete [] names;
		names = 0;
	}

	return path;
}

bool File::getDiskPath(std::vector<std::wstring> &volumes, std::vector<std::wstring> &devices, std::vector<std::wstring> &paths){
	DWORD  charCount            = 0;
	wchar_t  deviceName[MAX_PATH] = L"";
	DWORD  error                = ERROR_SUCCESS;
	HANDLE findHandle           = INVALID_HANDLE_VALUE;
	BOOL   found                = FALSE;
	size_t index                = 0;
	BOOL   success              = FALSE;
	wchar_t  volumeName[MAX_PATH] = L"";

	//
	//  Enumerate all volumes in the system.
	findHandle = FindFirstVolume(volumeName, ARRAYSIZE(volumeName));

	if (findHandle == INVALID_HANDLE_VALUE) {
		error = GetLastError();
		MYPRINTF("FindFirstVolumeW failed with error code %d\n", error);
		return false;
	}

	for (;;) {
		//
		//  Skip the \\?\ prefix and remove the trailing backslash.
		index = wcslen(volumeName) - 1;

		if (volumeName[0]     != L'\\' ||
			volumeName[1]     != L'\\' ||
			volumeName[2]     != L'?'  ||
			volumeName[3]     != L'\\' ||
			volumeName[index] != L'\\') {
				error = ERROR_BAD_PATHNAME;
				MYPRINTF("FindFirstVolumeW/FindNextVolumeW returned a bad path: %s\n", volumeName);
				break;
		}

		//
		//  QueryDosDeviceW does not allow a trailing backslash,
		//  so temporarily remove it.
		volumeName[index] = L'\0';

		charCount = QueryDosDevice(&volumeName[4], deviceName, ARRAYSIZE(deviceName)); 

		volumeName[index] = L'\\';

		if ( charCount == 0 ) {
			error = GetLastError();
			MYPRINTF("QueryDosDeviceW failed with error code %d\n", error);
			break;
		}

		devices.push_back(deviceName);
		volumes.push_back(volumeName);
		wstring path = File::displayVolumePaths(volumeName);
		if (!path.empty()){
			if (path != L"A:\\" && path != L"B:\\"){
				paths.push_back(path);
			}
		}

		//
		//  Move on to the next volume.
		success = FindNextVolume(findHandle, volumeName, ARRAYSIZE(volumeName));

		if ( !success ) {
			error = GetLastError();

			if (error != ERROR_NO_MORE_FILES) {
				MYPRINTF("FindNextVolumeW failed with error code %d\n", error);
				break;
			}

			//
			//  Finished iterating
			//  through all the volumes.
			error = ERROR_SUCCESS;
			break;
		}
	}

	FindVolumeClose(findHandle);
	findHandle = INVALID_HANDLE_VALUE;

	return true;

}

bool File::listDirectory(std::wstring path, std::vector<std::wstring> &files, std::vector<std::wstring> &directories){
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;   

	// Check that the input path plus 3 is not longer than MAX_PATH.
	// Three characters are for the "\*" plus NULL appended below.

	if (path.length() > (MAX_PATH - 3))	{
		MYPRINTF("\nDirectory path is too long.\n");
		return false;
	}

	// Prepare string for use with FindFile functions.  First, copy the
	// string to a buffer, then append '\*' to the directory name.

	path.append(L"\\*");

	// Find the first file in the directory.

	hFind = FindFirstFile(path.c_str(), &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		MYPRINTF("INVALID_HANDLE_VALUE");
		return false;
	} 

	// List all the files in the directory with some info about them.

	do {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			directories.push_back(ffd.cFileName);
		} else {
			filesize.LowPart = ffd.nFileSizeLow;
			filesize.HighPart = ffd.nFileSizeHigh;
			files.push_back(ffd.cFileName);
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) {
		MYPRINTF("ERROR_NO_MORE_FILES");
	}

	FindClose(hFind);
	return true;
}

bool File::listDirectory(std::wstring path, std::vector<std::wstring> &files){
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;   

	// Check that the input path plus 3 is not longer than MAX_PATH.
	// Three characters are for the "\*" plus NULL appended below.

	if (path.length() > (MAX_PATH - 3))	{
		MYPRINTF("\nDirectory path is too long.\n");
		return false;
	}

	// Prepare string for use with FindFile functions.  First, copy the
	// string to a buffer, then append '\*' to the directory name.

	path.append(L"\\*");

	// Find the first file in the directory.

	hFind = FindFirstFile(path.c_str(), &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		MYPRINTF("INVALID_HANDLE_VALUE");
		return false;
	} 

	// List all the files in the directory with some info about them.

	do {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			wstring dir = L">";
			dir.append(ffd.cFileName);
			files.push_back(dir);
			files.push_back(fileTimeToString(&ffd.ftLastWriteTime));			
		} else {
			files.push_back(ffd.cFileName);
			addFileInfoToStr(ffd, files);
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) {
		MYPRINTF("ERROR_NO_MORE_FILES");
	}

	FindClose(hFind);
	return true;
}