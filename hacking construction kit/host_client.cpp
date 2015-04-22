
#include "host_client.h"

#include <iostream>
#include <assert.h>

#include "macro.h"

#include "memory_debug.h"


using namespace std;

HostClient::HostClient():
	Host()
{
	m_readMutex = CreateMutex(NULL, FALSE, NULL);
	m_writeMutex = CreateMutex(NULL, FALSE, NULL);

}

HostClient::HostClient(SOCKET socket):
    Host(socket)
{
	m_readMutex = CreateMutex(NULL, FALSE, NULL);
	m_writeMutex = CreateMutex(NULL, FALSE, NULL);
}

HostClient::~HostClient(){
    if (m_readMutex)
        CloseHandle(m_readMutex);
    if (m_writeMutex)
        CloseHandle(m_writeMutex);
}


void HostClient::takeReadMutex(){
    WaitForSingleObject(m_readMutex, INFINITE);
}

void HostClient::takeWriteMutex(){
    WaitForSingleObject(m_writeMutex, INFINITE);
}

bool HostClient::connectToHostname(const char *hostname, int port, int seconds){
    struct hostent *host;
    struct sockaddr_in addr;

    if ((host = gethostbyname(hostname)) == NULL ) {
        MYPRINTF("HostClient: connectToHostname: Error: %s\n", hostname);
        return false;
    }

    ZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(long*)(host->h_addr);

	int result = SOCKET_ERROR;
	while(result == SOCKET_ERROR){
		result = connect(m_socket, (struct sockaddr*)&addr, sizeof(addr));
		if (result == SOCKET_ERROR) {
			int wsaError = WSAGetLastError();
			if (wsaError == WSAEWOULDBLOCK){
				int select = selectWriteSocket(seconds);
				if (select <= 0){
					if (select == 0){
						MYPRINTF("HostClient: connectToHostname: timeout\n");
					}
					return false;
				} 
			} else if (wsaError == WSAEISCONN || wsaError == WSAEALREADY || wsaError == WSAEINVAL){
				return true;
			} else {
				printWsaError(wsaError);
				return false;
			}
		}
	}

    return true;
}

