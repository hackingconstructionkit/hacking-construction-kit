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

#include <vector>
#include <string>

#include <WinSock2.h>

#include "ssl_host_client.h"
#include "host_server.h"
#include "macro.h"

// Create a ssl server
// You should extends this class and overwrite virtual method clientConnected
class SslHostServer: public HostServer {

public:

	SslHostServer(unsigned short port);
	virtual ~SslHostServer();

	virtual std::wstring toString() = 0;

	// verify peer certificate
	bool verifyPeer;
	// Certificate filename and provate key
	const char *certificatFilename;
	const char *keyFilename;
	// Certificate as buffer
	const char *certificatBuffer;
	const char *keyBuffer;
	// For certficate file
	const char *certificatePassword;
	// CA buffer
	const char *caBuffer;
	// SSH options
	bool useDh;
	bool useDhCallback;
	bool useRsaKey;
	bool useRsaCallback;

protected:
	// Called when a client is connected
	virtual void clientConnected(SOCKET client, SSL *ssl) = 0;

	virtual void clientConnected(SOCKET client);

private:

	SSL *readCertificatesAndCreateSslConnection(SOCKET client, SSL_CTX *ctx);

	DH *setupDh();

	SSL *createCtxAndAcceptSslConnection(SOCKET client);

	bool doHandShake(SOCKET socket, SSL *ssl);

	void printCipherList(const SSL *ssl);

	bool readCertificatesFromFiles(SSL_CTX *ctx);

	bool readCertificatsFromBuffer(SSL_CTX *ctx);

	bool readCAFromBuffer(SSL_CTX *ctx);

	SSL* configureRsaAndCreateSslConnection(SSL_CTX *ctx, SOCKET client);

	static DH *getDH512();

	static DH *getDH1024();

	static DH *getDH2048();

	static DH *getDH4096();

	static DH *tmp_dh_callback(SSL *ssl,int is_export, int keylength);

	static RSA *tmp_rsa_callback(SSL *ssl,int is_export, int keylength);

	static int password_callback(char* buffer, int num, int rwflag, void* userdata);

	static int verify_callback(int ok, X509_STORE_CTX* store);

	DH *m_dh;

	DISALLOW_COPY_AND_ASSIGN(SslHostServer);


};
