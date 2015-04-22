
#include "ssl_host_server.h"

#include <stdio.h>

#include <iostream>
#include <string>
#include <iterator>

#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

#include "macro.h"
#include "memory_debug.h"

using namespace std;

HostServer::HostServer(unsigned short port):
	Host(),
    m_port(port),
    m_running(false){

}

HostServer::~HostServer(){

}

void HostServer::startServer(){

    m_running = true;

    if (!bindSocket()){
        return;
    }

    if (!listenSocket()){
        return;
    }

    // Set up to accept connections

    sockaddr_in clientsockaddrin;
    int len = sizeof(clientsockaddrin);
    MYPRINTF("HostServer: Server ready to accept connections!\n");

    while (m_running) {
        int res =  selectReadSocket(10);

        if (res < 0){
            MYPRINTF("HostServer: select function failed with error: %d\n", WSAGetLastError());
        } else if (res > 0){
            // Block until a connection is ready
            SOCKET client = accept(m_socket, (sockaddr*) &clientsockaddrin, &len);
            if (client == INVALID_SOCKET) {
                int error = WSAGetLastError();
                if (error != WSAEWOULDBLOCK){
                    MYPRINTF("HostServer: Error on accept %d\n", error);
                }
            } else {
                MYPRINTF("HostServer: Connection recieved from %s on %d\n", inet_ntoa(clientsockaddrin.sin_addr), m_port);
                clientConnected(client);
            }
        }
    }


}

bool HostServer::bindSocket(){
    sockaddr_in sockaddrin;
    // Internet socket
    sockaddrin.sin_family = AF_INET;
    // Accept any IP
    sockaddrin.sin_addr.s_addr = INADDR_ANY;
    sockaddrin.sin_port =  htons(m_port);

    // Now bind to the port
    int ret = bind(m_socket, (sockaddr*) &(sockaddrin), sizeof(sockaddrin));
    if (ret != 0) {
        MYPRINTF("SslHostServer: error binding to port!\n");
        return false;
    }
    return true;
}

bool HostServer::listenSocket(){
    // Start listening for connections
    // Second param is max number of connections
    int ret = listen(m_socket, 50);
    if (ret != 0) {
        MYPRINTF("SslHostServer: Error listening for connections!\n");
        return false;
    }
    return true;
}

void HostServer::stop(){
    m_running = false;
}
