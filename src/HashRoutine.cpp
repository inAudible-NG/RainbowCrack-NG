/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#define no_md2_h
#include "HashRoutine.h"
#include "HashAlgorithm.h"

//////////////////////////////////////////////////////////////////////

CHashRoutine::CHashRoutine()
{
	// Notice: MIN_HASH_LEN <= nHashLen <= MAX_HASH_LEN

	AddHashRoutine("lm",   HashLM,   8);
	AddHashRoutine("ntlm", HashNTLM, 16);
	#ifndef no_md2_h
	AddHashRoutine("md2",  HashMD2,  16);
	#endif
	AddHashRoutine("md4",  HashMD4,  16);
	AddHashRoutine("md5",  HashMD5,  16);
	AddHashRoutine("sha1", HashSHA1, 20);
	AddHashRoutine("ripemd160", HashRIPEMD160, 20);
	AddHashRoutine("audible", HashAudible, 20);
}

CHashRoutine::~CHashRoutine()
{
}

void CHashRoutine::AddHashRoutine(string sHashRoutineName, HASHROUTINE pHashRoutine, int nHashLen)
{
	vHashRoutineName.push_back(sHashRoutineName);
	vHashRoutine.push_back(pHashRoutine);
	vHashLen.push_back(nHashLen);
}

string CHashRoutine::GetAllHashRoutineName()
{
	string sRet;
	int i;
	for (i = 0; i < vHashRoutineName.size(); i++)
		sRet += vHashRoutineName[i] + " ";

	return sRet;
}

void CHashRoutine::GetHashRoutine(string sHashRoutineName, HASHROUTINE& pHashRoutine, int& nHashLen)
{
	int i;
	for (i = 0; i < vHashRoutineName.size(); i++)
	{
		if (sHashRoutineName == vHashRoutineName[i])
		{
			pHashRoutine = vHashRoutine[i];
			nHashLen = vHashLen[i];
			return;
		}
	}

	pHashRoutine = NULL;
	nHashLen = 0;
}
