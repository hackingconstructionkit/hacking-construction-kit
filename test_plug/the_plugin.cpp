/*
 * Author: thirdstormofcythraul@outlook.com
 */
#include "the_plugin.h"

#include <Windows.h>
#include <tchar.h>

ThePlugin::ThePlugin(): m_stopped(false){

}

ThePlugin::~ThePlugin(){

}

void ThePlugin::start() {
	printf("started\n");
	while(!m_stopped){
		HANDLE hFile = CreateFile(_T("\\windows\\temp\\test_plug.log"), GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, 0, 0);
		if (hFile == INVALID_HANDLE_VALUE){
			printf("unable to create file: %d", GetLastError());
		} else {
			SYSTEMTIME st;
			DWORD dwLen;

			SetFilePointer(hFile, 0, 0, FILE_END);

			char buff[256]; 
			strcpy_s(buff, "timer:  ");
			GetLocalTime(&st);
			GetDateFormatA(0, 0, &st,"dd/MM/yyyy", buff + strlen(buff), 12);
			strcat_s(buff," à ");
			GetTimeFormatA(0, 0, &st, "HH:mm:ss", buff + strlen(buff), 12);
			strcat_s(buff, "\r\n");

			WriteFile(hFile, buff, strlen(buff), &dwLen, 0);
			CloseHandle(hFile);
		}
		Sleep(1000);
	}
	printf("stopped\n");
}

void ThePlugin::initialize(){}

void ThePlugin::destroy(){}

void ThePlugin::stop(){
	this->m_stopped = true;
}

