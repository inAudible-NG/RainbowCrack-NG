/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include <cstring>
#include "ChainWalkSet.h"

CChainWalkSet::CChainWalkSet()
{
	m_sHashRoutineName   = "";
	m_sPlainCharsetName  = "";
	m_nPlainLenMin       = 0;
	m_nPlainLenMax       = 0;
	m_nRainbowTableIndex = 0;
	m_nRainbowChainLen   = 0;
}

CChainWalkSet::~CChainWalkSet()
{
	DiscardAll();
}

void CChainWalkSet::DiscardAll()
{
	//printf("debug: discarding all walk...\n");

	list<ChainWalk>::iterator it;
	for (it = m_lChainWalk.begin(); it != m_lChainWalk.end(); it++)
		delete it->pIndexE;
	m_lChainWalk.clear();
}

uint64* CChainWalkSet::RequestWalk(unsigned char* pHash, int nHashLen,
								   string sHashRoutineName,
								   string sPlainCharsetName, int nPlainLenMin, int nPlainLenMax, 
								   int nRainbowTableIndex, 
								   int nRainbowChainLen,
								   bool& fNewlyGenerated)
{
	if (   m_sHashRoutineName   != sHashRoutineName
		|| m_sPlainCharsetName  != sPlainCharsetName
		|| m_nPlainLenMin       != nPlainLenMin
		|| m_nPlainLenMax       != nPlainLenMax
		|| m_nRainbowTableIndex != nRainbowTableIndex
		|| m_nRainbowChainLen   != nRainbowChainLen)
	{
		DiscardAll();

		m_sHashRoutineName   = sHashRoutineName;
		m_sPlainCharsetName  = sPlainCharsetName;
		m_nPlainLenMin       = nPlainLenMin;
		m_nPlainLenMax       = nPlainLenMax;
		m_nRainbowTableIndex = nRainbowTableIndex;
		m_nRainbowChainLen   = nRainbowChainLen;

		ChainWalk cw;
		memcpy(cw.Hash, pHash, nHashLen);
		cw.pIndexE = new uint64[nRainbowChainLen - 1];
		m_lChainWalk.push_back(cw);

		fNewlyGenerated = true;
		return cw.pIndexE;
	}

	list<ChainWalk>::iterator it;
	for (it = m_lChainWalk.begin(); it != m_lChainWalk.end(); it++)
	{
		if (memcmp(it->Hash, pHash, nHashLen) == 0)
		{
			fNewlyGenerated = false;
			return it->pIndexE;
		}
	}

	ChainWalk cw;
	memcpy(cw.Hash, pHash, nHashLen);
	cw.pIndexE = new uint64[nRainbowChainLen - 1];
	m_lChainWalk.push_back(cw);

	fNewlyGenerated = true;
	return cw.pIndexE;
}

void CChainWalkSet::DiscardWalk(uint64* pIndexE)
{
	list<ChainWalk>::iterator it;
	for (it = m_lChainWalk.begin(); it != m_lChainWalk.end(); it++)
	{
		if (it->pIndexE == pIndexE)
		{
			delete it->pIndexE;
			m_lChainWalk.erase(it);
			return;
		}
	}

	printf("debug: pIndexE not found\n");
}
