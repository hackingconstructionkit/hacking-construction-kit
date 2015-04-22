
#include "ssl_host_server.h"

#include <stdio.h>

#include <iostream>
#include <string>
#include <iterator>
#include <assert.h>

#include <WinSock2.h>

#include <openssl/bio.h> // BIO objects for I/O
#include <openssl/ssl.h> // SSL and SSL_CTX for SSL connections
#include <openssl/err.h> // Error reporting

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "ssleay32.lib")
#pragma comment(lib, "libeay32.lib")

#include "macro.h"
#include "memory_debug.h"
#include "openssl_init.h"

static OpensslInit init;

#define SSL_DH_SIZE 512

using namespace std;

SslHostServer::SslHostServer(unsigned short port):
	HostServer(port),
	certificatFilename(0),
	keyFilename(0),
	certificatBuffer(0),
	keyBuffer(0),
	certificatePassword(0),
	verifyPeer(false),
	caBuffer(0),
	useDh(false),
	useDhCallback(false),
	useRsaKey(false),
	useRsaCallback(false),
	m_dh(0){

}

SslHostServer::~SslHostServer()
{
	if (m_dh != 0){
		DH_free(m_dh);
	}
}

void SslHostServer::clientConnected(SOCKET client)
{
	SSL *ssl = createCtxAndAcceptSslConnection(client);
	if (ssl){
		clientConnected(client, ssl);
	} else {
		if (shutdown(client, SD_BOTH) == SOCKET_ERROR){
			Host::printWsaError();
		}
		if (closesocket(client) == SOCKET_ERROR){
			Host::printWsaError();
		}
	}
}

SSL *SslHostServer::createCtxAndAcceptSslConnection(SOCKET client)
{
	SSL_CTX *ctx = SSL_CTX_new(SSLv23_server_method());
	SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
	SSL *ssl = readCertificatesAndCreateSslConnection(client, ctx);
	SSL_CTX_free(ctx);
	return ssl;
}

SSL *SslHostServer::readCertificatesAndCreateSslConnection(SOCKET client, SSL_CTX *ctx)
{

	if (certificatFilename != 0 && keyFilename != 0){
		if (!readCertificatesFromFiles(ctx)){
			return 0;
		}
	} else if (certificatBuffer != 0 && keyBuffer != 0){
		if (!readCertificatsFromBuffer(ctx)){
			return 0;
		}
	} else {
		MYPRINTF("SslHostServer: no certificates or buffer\n");
		return 0;
	}
	if (caBuffer != 0){
		if (!readCAFromBuffer(ctx)){
			SSL_CTX_free(ctx);
			return 0;
		}
	}

	SSL_CTX_set_verify(ctx, verifyPeer ? SSL_VERIFY_PEER|SSL_VERIFY_CLIENT_ONCE|SSL_VERIFY_FAIL_IF_NO_PEER_CERT : SSL_VERIFY_NONE, SslHostServer::verify_callback);

	SSL *ssl = configureRsaAndCreateSslConnection(ctx, client);

	return ssl;
}

bool SslHostServer::readCertificatesFromFiles(SSL_CTX *ctx)
{

	SSL_CTX_set_default_passwd_cb(ctx, SslHostServer::password_callback);
	SSL_CTX_set_default_passwd_cb_userdata(ctx, (void *)certificatePassword);

	if (SSL_CTX_use_certificate_file(ctx, certificatFilename, SSL_FILETYPE_PEM) != 1){
		MYPRINTF("SslHostServer: Couldn't SSL_CTX_use_certificate_file!\n");
		return false;
	}

	if (SSL_CTX_use_PrivateKey_file(ctx, keyFilename, SSL_FILETYPE_PEM) != 1){
		MYPRINTF("SslHostServer: Couldn't SSL_CTX_use_PrivateKey_file!\n");
		return false;
	} 
	return true;
}

bool SslHostServer::readCertificatsFromBuffer(SSL_CTX *ctx)
{
	return SslHostClient::readCertificatsFromBuffer(ctx, certificatBuffer, keyBuffer);
}

bool SslHostServer::readCAFromBuffer(SSL_CTX *ctx)
{
	return SslHostClient::readCAFromBuffer(ctx, caBuffer);
}

SSL* SslHostServer::configureRsaAndCreateSslConnection(SSL_CTX *ctx, SOCKET client)
{
	if (useDh){
		DH *dh = setupDh();
		if (dh == 0){
			MYPRINTF("SslHostServer: Couldn't create DH parameters\n");
			return 0;
		}
		if (SSL_CTX_set_tmp_dh(ctx, dh) == 0) {
			MYPRINTF("SslHostServer: Couldn't set DH parameters\n");
			return 0;
		}
	}

	if (useDhCallback){
		SSL_CTX_set_tmp_dh_callback(ctx, SslHostServer::tmp_dh_callback);
	}

	if (SSL_CTX_need_tmp_RSA(ctx) == 1){
		//MYPRINTF("SslHostServer: need tmp RSA\n");
	}

	if (useRsaKey){
		RSA *rsa = RSA_generate_key(1024, RSA_F4, NULL, NULL);
		if (rsa == 0){
			MYPRINTF("SslHostServer: RSA_generate_key failed\n");
			return 0;
		}

		if (SSL_CTX_set_tmp_rsa(ctx, rsa) == 0) {
			MYPRINTF("SslHostServer: Couldn't set RSA key!\n");
		}
		RSA_free(rsa);
	}

	if (useRsaCallback){
		SSL_CTX_set_tmp_rsa_callback(ctx, SslHostServer::tmp_rsa_callback);
	}

	SSL_CTX_set_options(ctx, SSL_OP_ALL);

	if (SSL_CTX_set_cipher_list(ctx, "HIGH:!DSS:!aNULL") == 0){
		MYPRINTF("Unable to set cipher list\n");
		return 0;
	}

	BIO *sslclient = BIO_new_socket(client, BIO_NOCLOSE);
	if (sslclient == 0){
		MYPRINTF("BIO_new_socket error\n");
		return 0;
	}

	SSL *ssl = SSL_new(ctx);
	if (ssl == 0){
		MYPRINTF("SSL_new error\n");
		return 0;
	}

	SSL_set_bio(ssl, sslclient, sslclient);

	if (!doHandShake(client, ssl)){
		SSL_free(ssl);
		return 0;
	}

	if (SSL_get_verify_result(ssl) != X509_V_OK) {
		MYPRINTF("Certificate failed verification!\n");
	}

	return ssl;
}

void SslHostServer::printCipherList(const SSL *ssl)
{
	int priority = 0;
	const char *cipher = SSL_get_cipher_list(ssl, priority);
	while (cipher != 0) {
		priority++;
		MYPRINTF("cypher list: %s\n", cipher);
		cipher = SSL_get_cipher_list(ssl, priority);		
	}
}

int SslHostServer::password_callback(char* buffer, int num, int rwflag, void* userdata) 
{
	if (num < ((int)strlen((const char *)userdata) + 1)) {
		return(0);
	}
	strcpy_s(buffer, num, (const char *)userdata);
	return strlen((const char *)userdata);
}

int SslHostServer::verify_callback(int ok, X509_STORE_CTX* store) 
{
	char data[255];
	//MYPRINTF("verify_callback: %d\n", ok);
	if (!ok) {
		X509* cert = X509_STORE_CTX_get_current_cert(store);
		int depth = X509_STORE_CTX_get_error_depth(store);
		int err = X509_STORE_CTX_get_error(store);

		MYPRINTF("Error with certificate at depth: %d!\n", depth);
		X509_NAME_oneline(X509_get_issuer_name(cert), data, 255);
		MYPRINTF("\tIssuer: %s\n", data);
		X509_NAME_oneline(X509_get_subject_name(cert), data, 255);
		MYPRINTF("\tSubject: %s\n", data);
		MYPRINTF("\tError %d: %s\n", err, X509_verify_cert_error_string(err));
	}

	return ok;
}


DH *SslHostServer::setupDh()
{
	if (m_dh != 0){
		return m_dh;
	}
	DH *dh = DH_new();
	if (!dh){
		MYPRINTF("DH_new: failed");
		return 0;
	}

	if (!DH_generate_parameters_ex(dh, SSL_DH_SIZE, DH_GENERATOR_2, 0)){
		MYPRINTF("DH_generate_parameters_ex: failed");
		DH_free(dh);
		return 0;
	}

	int codes = 0;
	if (!DH_check(dh, &codes)){
		MYPRINTF("DH_check: failed");
		DH_free(dh);
		return 0;
	}

	if (!DH_generate_key(dh)){
		MYPRINTF("DH_generate_key: failed");
		DH_free(dh);
		return 0;
	}
	m_dh = dh;
	return m_dh;
}


bool SslHostServer::doHandShake(SOCKET socket, SSL *ssl)
{
	int result;
	int error;
	do {
		result =  Host::selectReadSocket(10, socket);
		if (result <= 0){
			MYPRINTF("SslHostServer: unable to select read\n");
			Host::printWsaError();
			return false;
		}

		result = SSL_accept(ssl);
		if (result == 1){
			return true;
		}
		error = SSL_get_error(ssl, result);
	} while (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE);
	MYPRINTF("SslHostServer: doHandShake failed: %d\n", error);
	SslHostClient::printSslError(result, error);
	return false;
}

DH* SslHostServer::getDH512()
{
	static unsigned char dh512_p[] =
	{
		0xF5,0x2A,0xFF,0x3C,0xE1,0xB1,0x29,0x40,0x18,0x11,0x8D,0x7C,
		0x84,0xA7,0x0A,0x72,0xD6,0x86,0xC4,0x03,0x19,0xC8,0x07,0x29,
		0x7A,0xCA,0x95,0x0C,0xD9,0x96,0x9F,0xAB,0xD0,0x0A,0x50,0x9B,
		0x02,0x46,0xD3,0x08,0x3D,0x66,0xA4,0x5D,0x41,0x9F,0x9C,0x7C,
		0xBD,0x89,0x4B,0x22,0x19,0x26,0xBA,0xAB,0xA2,0x5E,0xC3,0x55,
		0xE9,0x2A,0x05,0x5F,
	};
	static unsigned char dh512_g[] =
	{
		0x02,
	};
	DH* dh = DH_new();

	if( !dh )
		return 0;

	dh->p = BN_bin2bn( dh512_p, sizeof( dh512_p ), 0 );
	dh->g = BN_bin2bn( dh512_g, sizeof( dh512_g ), 0 );
	if( ( dh->p == 0 ) || ( dh->g == 0 ) )
	{
		DH_free( dh );
		return 0;
	}

	return dh;
}
DH* SslHostServer::getDH1024()
{
	static unsigned char dh1024_p[]={
		0xF4,0x88,0xFD,0x58,0x4E,0x49,0xDB,0xCD,0x20,0xB4,0x9D,0xE4,
		0x91,0x07,0x36,0x6B,0x33,0x6C,0x38,0x0D,0x45,0x1D,0x0F,0x7C,
		0x88,0xB3,0x1C,0x7C,0x5B,0x2D,0x8E,0xF6,0xF3,0xC9,0x23,0xC0,
		0x43,0xF0,0xA5,0x5B,0x18,0x8D,0x8E,0xBB,0x55,0x8C,0xB8,0x5D,
		0x38,0xD3,0x34,0xFD,0x7C,0x17,0x57,0x43,0xA3,0x1D,0x18,0x6C,
		0xDE,0x33,0x21,0x2C,0xB5,0x2A,0xFF,0x3C,0xE1,0xB1,0x29,0x40,
		0x18,0x11,0x8D,0x7C,0x84,0xA7,0x0A,0x72,0xD6,0x86,0xC4,0x03,
		0x19,0xC8,0x07,0x29,0x7A,0xCA,0x95,0x0C,0xD9,0x96,0x9F,0xAB,
		0xD0,0x0A,0x50,0x9B,0x02,0x46,0xD3,0x08,0x3D,0x66,0xA4,0x5D,
		0x41,0x9F,0x9C,0x7C,0xBD,0x89,0x4B,0x22,0x19,0x26,0xBA,0xAB,
		0xA2,0x5E,0xC3,0x55,0xE9,0x2F,0x78,0xC7,
	};
	static unsigned char dh1024_g[]={
		0x02,
	};
	DH* dh = DH_new();

	if( !dh )
		return 0;

	dh->p = BN_bin2bn( dh1024_p, sizeof( dh1024_p ), 0 );
	dh->g = BN_bin2bn( dh1024_g, sizeof( dh1024_g ), 0 );
	if( ( dh->p == 0 ) || ( dh->g == 0 ) )
	{
		DH_free( dh );
		return 0;
	}

	return dh;
}
DH* SslHostServer::getDH2048()
{
	static unsigned char dh2048_p[]={
		0xF6,0x42,0x57,0xB7,0x08,0x7F,0x08,0x17,0x72,0xA2,0xBA,0xD6,
		0xA9,0x42,0xF3,0x05,0xE8,0xF9,0x53,0x11,0x39,0x4F,0xB6,0xF1,
		0x6E,0xB9,0x4B,0x38,0x20,0xDA,0x01,0xA7,0x56,0xA3,0x14,0xE9,
		0x8F,0x40,0x55,0xF3,0xD0,0x07,0xC6,0xCB,0x43,0xA9,0x94,0xAD,
		0xF7,0x4C,0x64,0x86,0x49,0xF8,0x0C,0x83,0xBD,0x65,0xE9,0x17,
		0xD4,0xA1,0xD3,0x50,0xF8,0xF5,0x59,0x5F,0xDC,0x76,0x52,0x4F,
		0x3D,0x3D,0x8D,0xDB,0xCE,0x99,0xE1,0x57,0x92,0x59,0xCD,0xFD,
		0xB8,0xAE,0x74,0x4F,0xC5,0xFC,0x76,0xBC,0x83,0xC5,0x47,0x30,
		0x61,0xCE,0x7C,0xC9,0x66,0xFF,0x15,0xF9,0xBB,0xFD,0x91,0x5E,
		0xC7,0x01,0xAA,0xD3,0x5B,0x9E,0x8D,0xA0,0xA5,0x72,0x3A,0xD4,
		0x1A,0xF0,0xBF,0x46,0x00,0x58,0x2B,0xE5,0xF4,0x88,0xFD,0x58,
		0x4E,0x49,0xDB,0xCD,0x20,0xB4,0x9D,0xE4,0x91,0x07,0x36,0x6B,
		0x33,0x6C,0x38,0x0D,0x45,0x1D,0x0F,0x7C,0x88,0xB3,0x1C,0x7C,
		0x5B,0x2D,0x8E,0xF6,0xF3,0xC9,0x23,0xC0,0x43,0xF0,0xA5,0x5B,
		0x18,0x8D,0x8E,0xBB,0x55,0x8C,0xB8,0x5D,0x38,0xD3,0x34,0xFD,
		0x7C,0x17,0x57,0x43,0xA3,0x1D,0x18,0x6C,0xDE,0x33,0x21,0x2C,
		0xB5,0x2A,0xFF,0x3C,0xE1,0xB1,0x29,0x40,0x18,0x11,0x8D,0x7C,
		0x84,0xA7,0x0A,0x72,0xD6,0x86,0xC4,0x03,0x19,0xC8,0x07,0x29,
		0x7A,0xCA,0x95,0x0C,0xD9,0x96,0x9F,0xAB,0xD0,0x0A,0x50,0x9B,
		0x02,0x46,0xD3,0x08,0x3D,0x66,0xA4,0x5D,0x41,0x9F,0x9C,0x7C,
		0xBD,0x89,0x4B,0x22,0x19,0x26,0xBA,0xAB,0xA2,0x5E,0xC3,0x55,
		0xE9,0x32,0x0B,0x3B,
	};
	static unsigned char dh2048_g[]={
		0x02,
	};
	DH* dh = DH_new();

	if( !dh )
		return 0;

	dh->p = BN_bin2bn( dh2048_p, sizeof( dh2048_p ), 0 );
	dh->g = BN_bin2bn( dh2048_g, sizeof( dh2048_g ), 0 );
	if( ( dh->p == 0 ) || ( dh->g == 0 ) )
	{
		DH_free( dh );
		return 0;
	}

	return dh;
}

DH* SslHostServer::getDH4096()
{
	static unsigned char dh4096_p[]={
		0xFA,0x14,0x72,0x52,0xC1,0x4D,0xE1,0x5A,0x49,0xD4,0xEF,0x09,
		0x2D,0xC0,0xA8,0xFD,0x55,0xAB,0xD7,0xD9,0x37,0x04,0x28,0x09,
		0xE2,0xE9,0x3E,0x77,0xE2,0xA1,0x7A,0x18,0xDD,0x46,0xA3,0x43,
		0x37,0x23,0x90,0x97,0xF3,0x0E,0xC9,0x03,0x50,0x7D,0x65,0xCF,
		0x78,0x62,0xA6,0x3A,0x62,0x22,0x83,0xA1,0x2F,0xFE,0x79,0xBA,
		0x35,0xFF,0x59,0xD8,0x1D,0x61,0xDD,0x1E,0x21,0x13,0x17,0xFE,
		0xCD,0x38,0x87,0x9E,0xF5,0x4F,0x79,0x10,0x61,0x8D,0xD4,0x22,
		0xF3,0x5A,0xED,0x5D,0xEA,0x21,0xE9,0x33,0x6B,0x48,0x12,0x0A,
		0x20,0x77,0xD4,0x25,0x60,0x61,0xDE,0xF6,0xB4,0x4F,0x1C,0x63,
		0x40,0x8B,0x3A,0x21,0x93,0x8B,0x79,0x53,0x51,0x2C,0xCA,0xB3,
		0x7B,0x29,0x56,0xA8,0xC7,0xF8,0xF4,0x7B,0x08,0x5E,0xA6,0xDC,
		0xA2,0x45,0x12,0x56,0xDD,0x41,0x92,0xF2,0xDD,0x5B,0x8F,0x23,
		0xF0,0xF3,0xEF,0xE4,0x3B,0x0A,0x44,0xDD,0xED,0x96,0x84,0xF1,
		0xA8,0x32,0x46,0xA3,0xDB,0x4A,0xBE,0x3D,0x45,0xBA,0x4E,0xF8,
		0x03,0xE5,0xDD,0x6B,0x59,0x0D,0x84,0x1E,0xCA,0x16,0x5A,0x8C,
		0xC8,0xDF,0x7C,0x54,0x44,0xC4,0x27,0xA7,0x3B,0x2A,0x97,0xCE,
		0xA3,0x7D,0x26,0x9C,0xAD,0xF4,0xC2,0xAC,0x37,0x4B,0xC3,0xAD,
		0x68,0x84,0x7F,0x99,0xA6,0x17,0xEF,0x6B,0x46,0x3A,0x7A,0x36,
		0x7A,0x11,0x43,0x92,0xAD,0xE9,0x9C,0xFB,0x44,0x6C,0x3D,0x82,
		0x49,0xCC,0x5C,0x6A,0x52,0x42,0xF8,0x42,0xFB,0x44,0xF9,0x39,
		0x73,0xFB,0x60,0x79,0x3B,0xC2,0x9E,0x0B,0xDC,0xD4,0xA6,0x67,
		0xF7,0x66,0x3F,0xFC,0x42,0x3B,0x1B,0xDB,0x4F,0x66,0xDC,0xA5,
		0x8F,0x66,0xF9,0xEA,0xC1,0xED,0x31,0xFB,0x48,0xA1,0x82,0x7D,
		0xF8,0xE0,0xCC,0xB1,0xC7,0x03,0xE4,0xF8,0xB3,0xFE,0xB7,0xA3,
		0x13,0x73,0xA6,0x7B,0xC1,0x0E,0x39,0xC7,0x94,0x48,0x26,0x00,
		0x85,0x79,0xFC,0x6F,0x7A,0xAF,0xC5,0x52,0x35,0x75,0xD7,0x75,
		0xA4,0x40,0xFA,0x14,0x74,0x61,0x16,0xF2,0xEB,0x67,0x11,0x6F,
		0x04,0x43,0x3D,0x11,0x14,0x4C,0xA7,0x94,0x2A,0x39,0xA1,0xC9,
		0x90,0xCF,0x83,0xC6,0xFF,0x02,0x8F,0xA3,0x2A,0xAC,0x26,0xDF,
		0x0B,0x8B,0xBE,0x64,0x4A,0xF1,0xA1,0xDC,0xEE,0xBA,0xC8,0x03,
		0x82,0xF6,0x62,0x2C,0x5D,0xB6,0xBB,0x13,0x19,0x6E,0x86,0xC5,
		0x5B,0x2B,0x5E,0x3A,0xF3,0xB3,0x28,0x6B,0x70,0x71,0x3A,0x8E,
		0xFF,0x5C,0x15,0xE6,0x02,0xA4,0xCE,0xED,0x59,0x56,0xCC,0x15,
		0x51,0x07,0x79,0x1A,0x0F,0x25,0x26,0x27,0x30,0xA9,0x15,0xB2,
		0xC8,0xD4,0x5C,0xCC,0x30,0xE8,0x1B,0xD8,0xD5,0x0F,0x19,0xA8,
		0x80,0xA4,0xC7,0x01,0xAA,0x8B,0xBA,0x53,0xBB,0x47,0xC2,0x1F,
		0x6B,0x54,0xB0,0x17,0x60,0xED,0x79,0x21,0x95,0xB6,0x05,0x84,
		0x37,0xC8,0x03,0xA4,0xDD,0xD1,0x06,0x69,0x8F,0x4C,0x39,0xE0,
		0xC8,0x5D,0x83,0x1D,0xBE,0x6A,0x9A,0x99,0xF3,0x9F,0x0B,0x45,
		0x29,0xD4,0xCB,0x29,0x66,0xEE,0x1E,0x7E,0x3D,0xD7,0x13,0x4E,
		0xDB,0x90,0x90,0x58,0xCB,0x5E,0x9B,0xCD,0x2E,0x2B,0x0F,0xA9,
		0x4E,0x78,0xAC,0x05,0x11,0x7F,0xE3,0x9E,0x27,0xD4,0x99,0xE1,
		0xB9,0xBD,0x78,0xE1,0x84,0x41,0xA0,0xDF,
	};
	static unsigned char dh4096_g[]={
		0x02,
	};
	DH* dh = DH_new();

	if( !dh )
		return 0;

	dh->p = BN_bin2bn( dh4096_p, sizeof( dh4096_p ), 0 );
	dh->g = BN_bin2bn( dh4096_g, sizeof( dh4096_g ), 0 );
	if( ( dh->p == 0 ) || ( dh->g == 0 ) )
	{
		DH_free( dh );
		return 0;
	}

	return dh;
}

DH* SslHostServer::tmp_dh_callback(SSL*, int is_export, int keylength)
{
	MYPRINTF("tmp_dh_callback: keylength: %d\n", keylength)
		switch( keylength )	{
		case 512:
			return getDH512();
			break;
		case 1024:
			return getDH1024();
			break;
		case 2048:
			return getDH2048();
			break;
		case 4096:
			return getDH4096();
			break;
		default:
			MYPRINTF("unsupported DH param length requested");
			return 0;
			break;
	}
}

RSA* SslHostServer::tmp_rsa_callback(SSL*, int is_export, int keylength)
{
	MYPRINTF("tmp_rsa_callback: keylength: %d\n", keylength);
	return RSA_generate_key( keylength, RSA_F4, 0, 0 );
}