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

#include <string>
#include <vector>

#include <WinSock2.h>

#include <openssl/err.h> // Error reporting

#include "host_client.h"
#include "macro.h"

#define SSL_BUFFER_SIZE 16 * 1024

/*
This class is used to connect to a ssl server, and send/receive data.

ex :
	int timeout = 10;
	SslHostClient client;
	client.connectToHostname("www.google.com", 80, timeout);
	client.selectReadSocket(timeout);
	wcout << client.readStringFromSocket(timeout);
*/
class SslHostClient: public HostClient {
	
public:
	static bool readCertificatsFromBuffer(SSL_CTX *ctx, const char *certificatBuffer, const char *keyBuffer);
	static bool readCAFromBuffer(SSL_CTX *ctx, const char *caBuffer);
	static void printSslError(int res, int sslGetError, int wsaError = 0);
	
	SslHostClient();

	SslHostClient(SOCKET socket, SSL* ssl);

	~SslHostClient();

	// Connect to hostname:port
	bool sslConnectToHostname(const char *hostname, int port, int timeout);

	virtual bool isValid() const;

	// Read wstring
	std::wstring readStringFromSocket(int seconds);
	// Read buffer
	int readFromSocket(char *buffer, int size, int seconds);	
	// Send to server
    bool sendToSocket(const std::wstring& s);
	bool sendToSocket(const wchar_t *buffer, int length);
	bool sendToSocket(const char *buffer, int length, int seconds = 10);
    bool sendToSocket(const std::vector<char>& buffer);

	void stop();
	
	long long getLastActivity() const {
		return m_lastActivity;
	}
	// Verify server certificate
	// Need certificate, key and CA
	bool verifyPeer;
	// C string for certificate, ca and key
	// in PEM format
	// certificate and key are used to authenticate the client
	const char *certificatBuffer;
	const char *keyBuffer;
	// used to authenticate the server
	const char *caBuffer;
	// Don't wait for close notify on shutdown
	bool shutdownWaitForCloseNotify;

protected:
	static std::wstring readDataToStringFromSocket(int seconds, SOCKET socket, SSL *ssl);

	std::wstring readDataToStringFromSocket(int seconds);

	bool readCertificatsFromBuffer(SSL_CTX *ctx);
	bool readCAFromBuffer(SSL_CTX *ctx);

	bool sslConnect(int seconds);

	int readDataFromSocket(char *buffer, int size, int seconds);

	bool sendDataToSocket(const char *buffer, int length, int seconds = 20);

	int selectReadOrWrite(int err, int seconds);

	SSL_CTX* initCTX(void);
    
	long long m_lastActivity;

	SSL* m_ssl;
	
	DISALLOW_COPY_AND_ASSIGN(SslHostClient);
};
