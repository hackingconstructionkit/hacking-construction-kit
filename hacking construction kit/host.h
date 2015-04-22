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

#include <WinSock2.h>

#include "macro.h"

#define SELECT_TIMEOUT 10

// TCP connection
class Host {
public:
	// Select read/write on socket
    static int selectReadSocket(int seconds, SOCKET socket);
	static int selectWriteSocket(int seconds, SOCKET socket);

	static void printWsaError();

	static void printWsaError(int error);
	
	Host();
	// Create an host from socket
	Host(SOCKET socket);

	virtual ~Host();

	virtual bool isValid() const;

	// Set blocking/non blocking mode
	void setNonBlockingMode();
	void setBlockingMode();
	void setMode(unsigned long mode);

	// Select read/write on socket
	int selectReadSocket(int seconds);

	int selectWriteSocket(int seconds);

	virtual void stop();

		

protected:
	SOCKET m_socket;

private:
	
	DISALLOW_COPY_AND_ASSIGN(Host);
};
