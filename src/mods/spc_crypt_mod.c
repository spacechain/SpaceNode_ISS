#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <openssl/aes.h>

#include "spc_crypt_mod.h"

#define KEYLEN		16
#define HOTPLEN		6	
#define AESBLOCKLEN	16


//
static void _decrypt(const unsigned char *inBuf, uint32_t inLen, unsigned char *outBuf, unsigned char *key) 
{
    	AES_KEY aes;
    	unsigned char aesKey[16] = {'\0'}; 
    	unsigned char aesIv[16]  = {'\0'};
	memcpy(aesKey, key, KEYLEN);
	memcpy(aesIv, key, KEYLEN);

    	AES_set_decrypt_key(aesKey,128,&aes);
    	AES_cbc_encrypt(inBuf, outBuf, inLen, &aes, aesIv, AES_DECRYPT);
	return;
}


//
int crypt_decryptData(const unsigned char *inBuf, uint32_t inLen, unsigned char *outBuf, uint32_t outLen, unsigned char *key, uint32_t keyBufLen)
{
	if((NULL == inBuf) || (inLen < 16) || (NULL == outBuf) || ((outLen % AESBLOCKLEN) != 0) || (NULL == key) || (keyBufLen < KEYLEN))
	{
		printf("crypt_decryptData():");
		printf("Got invalid input.\n");
		return -1;	
	}

	_decrypt(inBuf, inLen, outBuf, key);

	return 0;
}

#if 0
//
int main(int argn, char *argv[] ) 
{

    	return 0;
}

#endif
