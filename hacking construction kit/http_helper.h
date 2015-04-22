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
#include <windows.h>
#include <WinInet.h>

typedef LPVOID HINTERNET;

class HttpHelper {
public:
	// Send HTTP get and retrieve page as array of LPSTR.
	// uri must not have http://
	// ex: google.com
	LPSTR* get(wchar_t *uri);

	// download from uri and save as filename
	void download(const wchar_t *uri, const wchar_t *filename);

	// send HTTP get, and read response
	bool sendGet(const wchar_t *server, const wchar_t *uri, char **response, int &responseSize);

	// send HTTP post, and read response
	bool sendPost(const wchar_t *server, wchar_t *uri, wchar_t *post, char **response, int &responseSize);

	/**
	* @brief uploadFile upload a file with POST
	* @param server like google.com or 127.0.0.1
	* @param resource like /upload.php
	* @param LocalFile source file name
	* @param remoteFile remote file name
	* @param response a pointer to the response page. you need to free this pointer.
	* @param responseSize size of response page
	* @return
	*/
	bool uploadFile(const wchar_t *server, const wchar_t *resource, const wchar_t *localFile, const wchar_t *remoteFile, char **response, int &responseSize);
	bool uploadFile(const wchar_t *uri, const wchar_t *localFile, const wchar_t *remoteFile, char **response, int &responseSize);

	/**
	* @brief fileToBuffer read a file to a buffer
	* @param file source file name
	* @param buffer a pointer to char*. you need to free the buffer after.
	* @param size
	* @return
	*/
	bool fileToBuffer(const wchar_t *file, char **buffer, int &size);

	/**
	* @brief uploadBuffer upload a buffer to server with POST request
	* @param server like 127.0.0.1
	* @param resource like /upload
	* @param buffer buffer to upload
	* @param size size of buffer
	* @param remoteFile remote file name
	* @param response you need to free this after
	* @param responseSize size of response
	* @return
	*/
	bool uploadBuffer(const wchar_t *server, const wchar_t *resource, const char* bufferToSend, int sizeToSend, const wchar_t *remoteFile, char **response, int &responseSize);

private:
	bool parseUri(const wchar_t *uri, wchar_t **url, wchar_t **resource);

	LPSTR* parseResponse(HINTERNET hRequest);

	LPSTR* callGet(const wchar_t *uri, const wchar_t *filename);

	bool writeToFile(HINTERNET hRequest, const wchar_t *filename);
	
	bool readInternetFile(HINTERNET hRequest, char **response, int &responseSize);

	bool createHeader(INTERNET_BUFFERS &internetBuffers, int size, const wchar_t *filename);

};

