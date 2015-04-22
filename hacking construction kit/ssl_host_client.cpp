
#include "ssl_host_client.h"

#include <iostream>
#include <assert.h>

#include <openssl/ssl.h> // SSL and SSL_CTX for SSL connections
#include <openssl/x509.h>

#include "macro.h"

#include "memory_debug.h"
#include "openssl_init.h"

#pragma comment(lib, "ssleay32.lib")
#pragma comment(lib, "libeay32.lib")

using namespace std;

static OpensslInit init;

SslHostClient::SslHostClient():
	m_ssl(0),
	m_lastActivity(time(0)),
	verifyPeer(false),
	certificatBuffer(0),
	caBuffer(0),
	shutdownWaitForCloseNotify(false)
{
}

SslHostClient::SslHostClient(SOCKET socket, SSL* ssl):
	HostClient(socket),
	m_ssl(ssl),
	m_lastActivity(time(0)),
	verifyPeer(false),
	certificatBuffer(0),
	caBuffer(0),
	shutdownWaitForCloseNotify(false)
{
	assert(ssl != 0);
}

SslHostClient::~SslHostClient()
{
	if (m_ssl != 0){
		SSL_shutdown(m_ssl);
		SSL_free(m_ssl);
		m_ssl = 0;
	}

}

bool SslHostClient::isValid() const
{
	return Host::isValid() && m_ssl != 0;
}

bool SslHostClient::sslConnectToHostname(const char *hostname, int port, int seconds)
{
	m_lastActivity = time(0);

	if (!connectToHostname(hostname, port, seconds)){
		return false;
	}

	SSL_CTX* ctx = initCTX();
	if (!ctx){
		return false;
	}


	if (certificatBuffer != 0 && !readCertificatsFromBuffer(ctx)){
		SSL_CTX_free(ctx);
		return false;
	}
	
	if (caBuffer != 0 && !readCAFromBuffer(ctx)){
		SSL_CTX_free(ctx);
		return false;
	}
	
	m_ssl = SSL_new(ctx);      /* create new SSL connection state */
	if (!m_ssl){
		SSL_CTX_free(ctx);
		return false;
	}

	if (SSL_set_fd(m_ssl, m_socket) == 0){
		MYPRINTF("SslHostClient: error on SSL_set_fd\n");
		SSL_free(m_ssl);
		SSL_CTX_free(ctx);
		return false;
	}

	bool result = sslConnect(seconds);

	STACK_OF(X509_NAME) *canames = SSL_get_client_CA_list(m_ssl);
	for (int i = 0; i < sk_X509_NAME_num(canames); i++) {
	
		X509_NAME *ex = sk_X509_NAME_value(canames, i);
		char buffer[1000];
		X509_NAME_oneline(ex, buffer, COUNTOF(buffer));
		MYPRINTF("X509_NAME_oneline %s\n", buffer);

	}

	if (result){
		if (verifyPeer && SSL_get_peer_certificate(m_ssl)){
			long verify = SSL_get_verify_result(m_ssl);
			if (verify != X509_V_OK){
				MYPRINTF("Certificate invalid %u\n", verify);
				result = false;
			}
		}
	} else {
		MYPRINTF("SslHostClient: error on sslConnect\n");
		SSL_free(m_ssl);
		m_ssl = 0;
	}

	SSL_CTX_free(ctx);
	return result;
}

bool SslHostClient::sslConnect(int seconds)
{
	int result = 0;
	while(result != 1){
		result = SSL_connect(m_ssl);

		if (result == 0){
			printSslError(result, SSL_get_error(m_ssl, result));
			return false;

		} else if (result < 0){
			int err = SSL_get_error(m_ssl, result);
			if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE){
				printSslError(result, err);
				return false;

			}

			int selectResult = selectReadOrWrite(err, seconds);
			
			if (selectResult < 0){
				MYPRINTF("SslHostClient: error on select\n");
				return false;

			} else if (selectResult == 0){
				MYPRINTF("SslHostClient: timeout on select\n");
				return false;

			}

		}
	}
	return true;

}

bool SslHostClient::readCAFromBuffer(SSL_CTX *ctx, const char *caBuffer)
{
	X509 *cert = NULL;
	BIO *cbio;

	cbio = BIO_new_mem_buf((void*)caBuffer, -1);
	cert = PEM_read_bio_X509(cbio, NULL, 0, NULL);
	if (!cert){
		MYPRINTF("SslHostClient: PEM_read_bio_X509 error\n");
		return false;
	}

	X509_STORE *store = SSL_CTX_get_cert_store(ctx);
	if (!store){
		MYPRINTF("SslHostClient:SSL_CTX_get_cert_store error\n");
		return false;
	}

	if (X509_STORE_add_cert(store, cert) != 1){
		MYPRINTF("SslHostClient:X509_STORE_add_cert error\n");
		return false;
	}
	return true;
}

bool SslHostClient::readCAFromBuffer(SSL_CTX *ctx)
{
	return readCAFromBuffer(ctx, caBuffer);
}

bool SslHostClient::readCertificatsFromBuffer(SSL_CTX *ctx, const char *certificatBuffer, const char *keyBuffer)
{
	X509 *cert = NULL;
	RSA *rsa = NULL;
	BIO *cbio, *kbio;

	cbio = BIO_new_mem_buf((void*)certificatBuffer, -1);
	cert = PEM_read_bio_X509(cbio, NULL, 0, NULL);
	if (!cert){
		MYPRINTF("SslHostClient: PEM_read_bio_X509 error\n");
		return false;
	}
	if (SSL_CTX_use_certificate(ctx, cert) != 1){
		MYPRINTF("SslHostClient: SSL_CTX_use_certificate error\n");
		X509_free(cert);
		return false;
	}
	X509_free(cert);
	kbio = BIO_new_mem_buf((void*)keyBuffer, -1);
	rsa = PEM_read_bio_RSAPrivateKey(kbio, NULL, 0, NULL);
	if (!rsa){
		MYPRINTF("SslHostClient: PEM_read_bio_RSAPrivateKey error\n");
		return false;
	}
	if (SSL_CTX_use_RSAPrivateKey(ctx, rsa) != 1){
		MYPRINTF("SslHostClient: SSL_CTX_use_RSAPrivateKey error\n");
		RSA_free(rsa);
		return false;
	}
	return true;
}

bool SslHostClient::readCertificatsFromBuffer(SSL_CTX *ctx)
{
	return readCertificatsFromBuffer(ctx, certificatBuffer, keyBuffer);
}

int SslHostClient::selectReadOrWrite(int err, int seconds)
{
	if (err == SSL_ERROR_WANT_READ){
		return selectReadSocket(seconds);
	}
	return selectWriteSocket(seconds);

}

SSL_CTX* SslHostClient::initCTX(void)
{   	
	const SSL_METHOD *method = SSLv23_client_method();  /* Create new client-method instance */
	SSL_CTX *ctx = SSL_CTX_new(method);   /* Create new context */
	SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
	if ( ctx == 0 ) {
		MYPRINTF("SslHostClient: SSL_CTX_new: %s\n", stderr);
		return 0;
	}

	if (SSL_CTX_set_cipher_list(ctx, "HIGH:!DSS:!aNULL") == 0){
		SSL_CTX_free(ctx);
		return 0;
	}
	SSL_CTX_set_verify(ctx, verifyPeer ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, 0);
	return ctx;
}

std::wstring SslHostClient::readStringFromSocket(int seconds)
{	
	takeReadMutex();

	wstring stringResult = readDataToStringFromSocket(seconds);

	ReleaseMutex(m_readMutex);

	return stringResult;
}

std::wstring SslHostClient::readDataToStringFromSocket(int seconds)
{
	if (!isValid()){
		MYPRINTF("SslHostClient: readDataToStringFromSocket: not valid\n");
		return L"error";
	}

	wchar_t buffer[SSL_BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));

	wstring stringResult;

	do {
		int result = SSL_read(m_ssl, buffer, COUNTOF(buffer));
		if (result > 0){
			stringResult = stringResult.append(buffer, result / sizeof(wchar_t));
		} else {
			int sslError = SSL_get_error(m_ssl, result);
			if (sslError == SSL_ERROR_ZERO_RETURN){
				stop();
				return stringResult;
			}
			if (sslError != SSL_ERROR_WANT_READ && sslError != SSL_ERROR_WANT_WRITE){
				printSslError(result, sslError);
				return L"error";
			}

			if (!stringResult.empty()){
				return stringResult;
			}

			int selectResult = selectReadOrWrite(sslError, seconds);
			if (selectResult == 0){
				return L"timeout";
			} else if (selectResult < 0) {
				MYPRINTF("SslHostClient: readDataToStringFromSocket: select error\n");
				return L"error";
			}
		}

	} while(true);

	return stringResult;
}

std::wstring SslHostClient::readDataToStringFromSocket(int seconds, SOCKET socket, SSL *ssl)
{
	wchar_t buffer[SSL_BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));

	wstring stringResult;

	do {
		int result = SSL_read(ssl, buffer, COUNTOF(buffer));
		if (result > 0){
			stringResult = stringResult.append(buffer, result / sizeof(wchar_t));
		} else {
			int sslError = SSL_get_error(ssl, result);
			if (sslError == SSL_ERROR_ZERO_RETURN){
				SSL_shutdown(ssl);
				return stringResult;
			}
			if (sslError != SSL_ERROR_WANT_READ && sslError != SSL_ERROR_WANT_WRITE){
				printSslError(result, sslError);
				return L"error";
			}

			if (!stringResult.empty()){
				return stringResult;
			}

			int selectResult;
			if (sslError == SSL_ERROR_WANT_READ){
				selectResult = Host::selectReadSocket(seconds, socket);
			} else {
				selectResult = Host::selectWriteSocket(seconds, socket);
			}
			if (selectResult == 0){
				return L"timeout";
			} else if (selectResult < 0) {
				MYPRINTF("SslHostClient: readDataToStringFromSocket: select error\n");
				return L"error";
			}
		}

	} while(true);

	return stringResult;
}

int SslHostClient::readFromSocket(char *buffer, int size, int seconds)
{	
	takeReadMutex();

	int result = readDataFromSocket(buffer, size, seconds);

	ReleaseMutex(m_readMutex);

	return result;
}

int SslHostClient::readDataFromSocket(char *buffer, int size, int seconds)
{
	if (!isValid()){
		MYPRINTF("SslHostClient: readDataFromSocket: not valid\n");
		return 0;
	}
	m_lastActivity = time(0);

	int result = 0;
	while(result == 0) {
		result = SSL_read(m_ssl, buffer, size);
		MYPRINTF("SslHostClient: readDataFromSocket: size read: %d\n", result);
		if (result > 0){
			return result;
		}

		int sslError = SSL_get_error(m_ssl, result);
		if (sslError == SSL_ERROR_ZERO_RETURN){
			stop();
			return result;
		}
		if (sslError != SSL_ERROR_WANT_READ && sslError != SSL_ERROR_WANT_WRITE){
			printSslError(result, sslError);
			return result;
		}

		int selectResult = selectReadOrWrite(sslError, seconds);
		if (selectResult == 0){
			MYPRINTF("SslHostClient: readDataFromSocket: Unable to read (socket timeout)\n");
			return result;
		} else if (selectResult < 0) {
			MYPRINTF("SslHostClient: readDataFromSocket: select error\n");
			return result;
		}
		result = 0;

	}
	return result;
}

bool SslHostClient::sendToSocket(const std::wstring& s)
{
	return sendToSocket((const char *)s.c_str(), s.size() * 2);
}

bool SslHostClient::sendToSocket(const wchar_t *buffer, int length)
{
	return sendToSocket((const char *)buffer, length * 2);
}

bool SslHostClient::sendToSocket(const char *buffer, int length, int seconds)
{
	takeWriteMutex();

	bool result = sendDataToSocket(buffer, length, seconds);

	ReleaseMutex(m_writeMutex);

	return result;
}

bool SslHostClient::sendToSocket(const std::vector<char>& vectorBuffer)
{
	bool result;
	if (vectorBuffer.size() > 0){
		result = sendToSocket(&vectorBuffer[0], vectorBuffer.size());
	} else {
		result = sendToSocket(L"");
	}

	return result;
}

bool SslHostClient::sendDataToSocket(const char *buffer, int length, int seconds)
{
	if (!isValid()){
		MYPRINTF("SslHostClient: sendDataToSocket: not valid\n");
		return false;
	}

	m_lastActivity = time(0);

	int err = 0;

	do {
		int result = SSL_write(m_ssl, buffer, length);
		if (result == length){
			return true;
		}

		if (result > 0){
			MYPRINTF("SslHostClient: unable to write all data on socket\n");
			return false;
		}

		if (result == 0){
			MYPRINTF("SslHostClient: unable to write on socket\n");
			return false;
		} 

		err = SSL_get_error(m_ssl, result);
		if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE){
			printSslError(result, err);
			MYPRINTF("SslHostClient: error on write: %d\n", err);
			return false;
		}

		int selectResult = selectReadOrWrite(err, seconds);
		if (selectResult <= 0){
			if (selectResult < 0){
				MYPRINTF("SslHostClient: sendDataToSocket: error on select\n");
			} else {
				MYPRINTF("SslHostClient: sendDataToSocket: select timeout\n");
			}
			return false;
		}

	} while(true);

}

void SslHostClient::stop()
{
	takeReadMutex();
	takeWriteMutex();

	if (m_ssl != 0){
		int result = 0;
		while(result != 1){
			result = SSL_shutdown(m_ssl);
			if (!shutdownWaitForCloseNotify && result == 0){
				result = 1;
			}
			if (result == -1){

				int err = SSL_get_error(m_ssl, result);

				if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE){
					MYPRINTF("SslHostClient: error on SSL_shutdown: %d\n", err);
					result = 1;
				} else {

					int selectResult = selectReadOrWrite(err, 2);

					if (selectResult < 0){
						MYPRINTF("SslHostClient: stop: error on select\n");
						result = 1;
					} else if (selectResult == 0){
						result = 1;
					}
				}
			}
		}
		SSL_free(m_ssl);
		m_ssl = 0;
	}
	Host::stop();

	ReleaseMutex(m_writeMutex);
	ReleaseMutex(m_readMutex);
}

void SslHostClient::printSslError(int result, int sslGetError, int wsaError)
{
	MYPRINTF("SSL: SSL_get_error : %d\n", sslGetError);
	MYPRINTF("SSL: function returned %d\n", result);
	if (sslGetError == SSL_ERROR_SYSCALL || sslGetError == SSL_ERROR_SSL){
		long error = ERR_get_error();
		if (error == 0 && wsaError != 0){
			Host::printWsaError(wsaError);
		}
		while(error != 0){
			char errorStr[65535];
			ERR_error_string_n(error, errorStr, 65535);
			MYPRINTF("SSL: ERR_get_error : %s\n", errorStr);
			error = ERR_get_error();
		}
	}
}


