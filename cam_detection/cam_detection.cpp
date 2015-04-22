/*
 * Author: thirdstormofcythraul@outlook.com
 */
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

#include <cwebcam.h>
#pragma comment(lib, "hacking construction kit.lib")

int _tmain(int argc, wchar_t* argv[])
{
	if (argc != 6) {
		printf("try to capture image only if a motion is detected\n");
		printf("usage: %s threshold img_modif_percent pixels name_diff count\n", argv[0]);
		return 0;
	}
	int threshold = _tstoi(argv[1]); 
	int img_modif_percent = _tstoi(argv[2]); 
	int pixels = _tstoi(argv[3]); 
	int count = _tstoi(argv[5]); 

#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	if (request_webcam_start(640, 480, 1) == 0){
		for (int i = 0; i < count; i++){
			Sleep(1000);// wait for initialisation ?
			request_webcam_motion_detection(_T("image.jpg"), 100, threshold, img_modif_percent, pixels, argv[4]);
		}
	}
	request_webcam_stop();
	freeGlobals();
	return 0;
}

