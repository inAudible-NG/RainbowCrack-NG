/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#ifndef _PUBLIC_H
#define _PUBLIC_H

#include <stdio.h>
#include <cstdio>
#include <cctype>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <stdint.h>
#include <csignal>
#ifndef _WIN32
	#include <unistd.h>
#endif

#include <string>
#include <vector>
#include <list>
using namespace std;

#ifdef _WIN32
	#define uint64 unsigned __int64
#else
#include <sys/types.h>
	#define uint64 u_int64_t
#endif

struct RainbowChain
{
	uint64 nIndexS;
	uint64 nIndexE;
};

#define MAX_PLAIN_LEN 256
#define MIN_HASH_LEN  8
#define MAX_HASH_LEN  256

unsigned int GetFileLen(FILE* file);
string TrimString(string s);
bool ReadLinesFromFile(string sPathName, vector<string>& vLine);
bool SeperateString(string s, string sSeperator, vector<string>& vPart);
string uint64tostr(uint64 n);
string uint64tohexstr(uint64 n);
string HexToStr(const unsigned char* pData, int nLen);
unsigned long GetAvailPhysMemorySize();
void ParseHash(string sHash, unsigned char* pHash, int& nHashLen);

void Logo();

#endif
