/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#ifndef _CHAINWALKSET_H
#define _CHAINWALKSET_H

#include "Public.h"

struct ChainWalk
{
	unsigned char Hash[MAX_HASH_LEN];
	//int nHashLen;		// Implied
	uint64* pIndexE;	// mapStartPosIndexE, Len = nRainbowChainLen - 1
};

class CChainWalkSet
{
public:
	CChainWalkSet();
	virtual ~CChainWalkSet();

private:
	string m_sHashRoutineName;		// Discard all if not match
	string m_sPlainCharsetName;		// Discard all if not match
	int    m_nPlainLenMin;			// Discard all if not match
	int    m_nPlainLenMax;			// Discard all if not match
	int    m_nRainbowTableIndex;	// Discard all if not match
	int    m_nRainbowChainLen;		// Discard all if not match
	list<ChainWalk> m_lChainWalk;

private:
	void DiscardAll();

public:
	uint64* RequestWalk(unsigned char* pHash, int nHashLen,
						string sHashRoutineName,
						string sPlainCharsetName, int nPlainLenMin, int nPlainLenMax,
						int nRainbowTableIndex,
						int nRainbowChainLen,
						bool& fNewlyGenerated);
	void DiscardWalk(uint64* pIndexE);
};

#endif
