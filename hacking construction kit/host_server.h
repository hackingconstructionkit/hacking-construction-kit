/*
 * Hacking construction kit -- A GPL Windows security library.
 * While this library can potentially be used for nefarious purposes, 
 * it was written for educational and recreational
 * purposes only and the author does not endorse illegal use.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: thirdstormofcythraul@outlook.com
 */
#pragma once

#include "host.h"

// Server socket
class HostServer: public Host {

public:

    HostServer(unsigned short port);

    virtual ~HostServer();

	// Start listening
    virtual void startServer();

    void stop();

    virtual std::wstring toString() = 0;

    volatile bool m_running;

protected:
	// Called when a new client is connected
    virtual void clientConnected(SOCKET client) = 0;

    bool bindSocket();

    bool listenSocket();

    unsigned short m_port;

    DISALLOW_COPY_AND_ASSIGN(HostServer);


};
