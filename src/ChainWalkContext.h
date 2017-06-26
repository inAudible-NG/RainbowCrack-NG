/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#ifndef _CHAINWALKCONTEXT_H
#define _CHAINWALKCONTEXT_H

#include "HashRoutine.h"
#include "Public.h"

class CChainWalkContext
{
public:
	CChainWalkContext();
	virtual ~CChainWalkContext();

private:
	static string m_sHashRoutineName;
	static HASHROUTINE m_pHashRoutine;							// Configuration
	static int m_nHashLen;										// Configuration

	static unsigned char m_PlainCharset[256];					// Configuration
	static int m_nPlainCharsetLen;								// Configuration
	static int m_nPlainLenMin;									// Configuration
	static int m_nPlainLenMax;									// Configuration
	static string m_sPlainCharsetName;
	static string m_sPlainCharsetContent;
	static uint64 m_nPlainSpaceUpToX[MAX_PLAIN_LEN + 1];		// Performance consideration
	static uint64 m_nPlainSpaceTotal;							// Performance consideration

	static int m_nRainbowTableIndex;							// Configuration
	static uint64 m_nReduceOffset;								// Performance consideration

	// Context
	uint64 m_nIndex;
	unsigned char m_Plain[MAX_PLAIN_LEN];
	int m_nPlainLen;
	unsigned char m_Hash[MAX_HASH_LEN];

private:
	static bool LoadCharset(string sName);

public:
	static bool SetHashRoutine(string sHashRoutineName);												// Configuration
	static bool SetPlainCharset(string sCharsetName, int nPlainLenMin, int nPlainLenMax);				// Configuration
	static bool SetRainbowTableIndex(int nRainbowTableIndex);											// Configuration
	static bool SetupWithPathName(string sPathName, int& nRainbowChainLen, int& nRainbowChainCount);	// Wrapper
	static string GetHashRoutineName();
	static int GetHashLen();
	static string GetPlainCharsetName();
	static string GetPlainCharsetContent();
	static int GetPlainLenMin();
	static int GetPlainLenMax();
	static uint64 GetPlainSpaceTotal();
	static int GetRainbowTableIndex();
	static void Dump();

	void GenerateRandomIndex();
	void SetIndex(uint64 nIndex);
	void SetHash(unsigned char* pHash);		// The length should be m_nHashLen

	void IndexToPlain();
	void PlainToHash();
	void HashToIndex(int nPos);

	uint64 GetIndex();
	string GetPlain();
	string GetBinary();
	string GetPlainBinary();
	string GetHash();
	bool CheckHash(unsigned char* pHash);	// The length should be m_nHashLen
};

#endif
