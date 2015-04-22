/*
 * Author: thirdstormofcythraul@outlook.com
 */
#include "openssl_init.h"

#include <Ws2tcpip.h>

#include <openssl/bio.h> // BIO objects for I/O
#include <openssl/ssl.h> // SSL and SSL_CTX for SSL connections
#include <openssl/err.h> // Error reporting

#pragma comment(lib, "ws2_32")

bool OpensslInit::isInit = false;

OpensslInit::OpensslInit(){
	if (!isInit){
		isInit = true;
		CRYPTO_malloc_init(); // Initialize malloc, free, etc for OpenSSL's use
		SSL_library_init(); // Initialize OpenSSL's SSL libraries
		SSL_load_error_strings(); // Load SSL error strings
		ERR_load_BIO_strings(); // Load BIO error strings
		OpenSSL_add_all_algorithms(); // Load all available encryption algorithms
	}
}

OpensslInit::~OpensslInit(){
	if (!isInit){
		ERR_free_strings();
		EVP_cleanup();
	}
}