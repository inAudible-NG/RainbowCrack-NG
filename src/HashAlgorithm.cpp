/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#include "HashAlgorithm.h"

#include "Public.h"

#include <openssl/des.h>
#define no_md2_h
#ifndef no_md2_h
#include <openssl/md2.h>
#endif
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#ifdef _WIN32
	#pragma comment(lib, "libcrypto.lib")
#endif

void setup_des_key(unsigned char key_56[], DES_key_schedule &ks)
{
	DES_cblock key;

	key[0] = key_56[0];
	key[1] = (key_56[0] << 7) | (key_56[1] >> 1);
	key[2] = (key_56[1] << 6) | (key_56[2] >> 2);
	key[3] = (key_56[2] << 5) | (key_56[3] >> 3);
	key[4] = (key_56[3] << 4) | (key_56[4] >> 4);
	key[5] = (key_56[4] << 3) | (key_56[5] >> 5);
	key[6] = (key_56[5] << 2) | (key_56[6] >> 6);
	key[7] = (key_56[6] << 1);

	//des_set_odd_parity(&key);
	DES_set_key(&key, &ks);
}

void HashLM(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	/*
	unsigned char data[7] = {0};
	memcpy(data, pPlain, nPlainLen > 7 ? 7 : nPlainLen);
	*/

	int i;
	for (i = nPlainLen; i < 7; i++)
		pPlain[i] = 0;

	static unsigned char magic[] = {0x4B, 0x47, 0x53, 0x21, 0x40, 0x23, 0x24, 0x25};
	DES_key_schedule ks;
	//setup_des_key(data, ks);
	setup_des_key(pPlain, ks);
	DES_ecb_encrypt((DES_cblock*)magic, (DES_cblock*)pHash, &ks, DES_ENCRYPT);
}

void HashNTLM(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	unsigned char UnicodePlain[MAX_PLAIN_LEN * 2];
	int i;
	for (i = 0; i < nPlainLen; i++)
	{
		UnicodePlain[i * 2] = pPlain[i];
		UnicodePlain[i * 2 + 1] = 0x00;
	}

	MD4(UnicodePlain, nPlainLen * 2, pHash);
}

#ifndef no_md2_h
void HashMD2(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	MD2(pPlain, nPlainLen, pHash);
}
#endif

void HashMD4(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	MD4(pPlain, nPlainLen, pHash);
}

void HashMD5(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	MD5(pPlain, nPlainLen, pHash);
}

void HashSHA1(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	SHA1(pPlain, nPlainLen, pHash);
}

void HashRIPEMD160(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	RIPEMD160(pPlain, nPlainLen, pHash);
}

void HashAudible(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	unsigned char fixed_key[] = { 0x77, 0x21, 0x4d, 0x4b, 0x19, 0x6a, 0x87, 0xcd, 0x52, 0x00, 0x45, 0xfd, 0x20, 0xa5, 0x1d, 0x67 };

	unsigned char hash1[20] = {0};
	unsigned char hash2[20] = {0};
	SHA_CTX ctx;

	// fixed_key + "guess"
	SHA1_Init(&ctx);
	SHA1_Update(&ctx, fixed_key, 16);
	SHA1_Update(&ctx, pPlain, nPlainLen);
	SHA1_Final(hash1, &ctx);

	// fixed_key + previous hash + guess
	SHA1_Init(&ctx);
	SHA1_Update(&ctx, fixed_key, 16);
	SHA1_Update(&ctx, hash1, 20);
	SHA1_Update(&ctx, pPlain, nPlainLen);
	SHA1_Final(hash2, &ctx);

	// final checksum calculation
	SHA1_Init(&ctx);
	SHA1_Update(&ctx, hash1, 16); // only 16 bytes!
	SHA1_Update(&ctx, hash2, 16);
	SHA1_Final(pHash, &ctx);
}
