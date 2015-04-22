
#include "host.h"

#include <assert.h>

#include "wsa_init.h"

#pragma comment(lib, "ws2_32.lib")

static WsaInit init;

Host::Host():
	m_socket(socket(AF_INET, SOCK_STREAM, 0))
{
	assert(m_socket != INVALID_SOCKET);

	setNonBlockingMode();
}

Host::Host(SOCKET socket):
	m_socket(socket){
	assert(socket != 0);

	setNonBlockingMode();
}

Host::~Host(){
	if (m_socket != 0){
		shutdown(m_socket, SD_BOTH);
		closesocket(m_socket);
		m_socket = 0;
	}
}

bool Host::isValid() const{
	return m_socket != 0;
}

void Host::stop(){
	if (m_socket != 0) {
		shutdown(m_socket, SD_BOTH);
		closesocket(m_socket);
		m_socket = socket(AF_INET, SOCK_STREAM, 0);
		assert(m_socket != INVALID_SOCKET);
		setNonBlockingMode();
	}
}

void Host::setBlockingMode(){
	setMode(0);
}

void Host::setNonBlockingMode(){
	setMode(1);
}

void Host::setMode(unsigned long mode){
	u_long iMode = mode;
	if (ioctlsocket(m_socket, FIONBIO, &iMode) == SOCKET_ERROR){
		MYPRINTF("Unable to set ioctl\n");
		Host::printWsaError();
	}
}

int Host::selectReadSocket(int seconds, SOCKET socket){
	struct timeval timeout;
	struct fd_set fds;
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(socket, &fds);
	// Possible return values:
	// -1: error occurred
	// 0: timed out
	// > 0: data ready to be read
	return select(0, &fds, 0, 0, &timeout);
}

int Host::selectWriteSocket(int seconds, SOCKET socket){
	struct timeval timeout;
	struct fd_set fds;
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(socket, &fds);
	// Possible return values:
	// -1: error occurred
	// 0: timed out
	// > 0: data ready to be read
	return select(0, 0, &fds, 0, &timeout);
}

int Host::selectReadSocket(int seconds){
    return Host::selectReadSocket(seconds, m_socket);
}

int Host::selectWriteSocket(int seconds){
	struct timeval timeout;
	struct fd_set fds;
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(m_socket, &fds);
	// Possible return values:
	// -1: error occurred
	// 0: timed out
	// > 0: data ready to be read
	return select(0, 0, &fds, 0, &timeout);
}

void Host::printWsaError(){
	int error = WSAGetLastError();
	MYPRINTF("SSL: WSA: %d\n", error);
}

void Host::printWsaError(int error){
	MYPRINTF("SSL: WSA: %d\n", error);
}


