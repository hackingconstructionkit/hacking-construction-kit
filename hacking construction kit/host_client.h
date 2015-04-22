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

// Client socket
class HostClient: public Host {
public:

	HostClient();

    HostClient(SOCKET socket);

    ~HostClient();

	// Ex: connectToHostname("10.0.0.1", 80, 60)
    bool connectToHostname(const char *hostname, int port, int timeout);

protected:

    void takeReadMutex();

    void takeWriteMutex();

    HANDLE m_readMutex;
    HANDLE m_writeMutex;

    DISALLOW_COPY_AND_ASSIGN(HostClient);

};
