/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#ifndef _CRACKENGINE_H
#define _CRACKENGINE_H

#include "Public.h"
#include "HashSet.h"
#include "ChainWalkContext.h"
#include "MemoryPool.h"
#include "ChainWalkSet.h"

class CCrackEngine
{
public:
	CCrackEngine();
	virtual ~CCrackEngine();

private:
	CChainWalkSet m_cws;

	// Statistics
	float m_fTotalDiskAccessTime;
	float m_fTotalCryptanalysisTime;
	int m_nTotalChainWalkStep;
	int m_nTotalFalseAlarm;
	int m_nTotalChainWalkStepDueToFalseAlarm;

private:
	void ResetStatistics();
	int BinarySearch(RainbowChain* pChain, int nRainbowChainCount, uint64 nIndex);
	void GetChainIndexRangeWithSameEndpoint(RainbowChain* pChain,
										    int nRainbowChainCount,
										    int nChainIndex,
										    int& nChainIndexFrom,
										    int& nChainIndexTo);
	bool CheckAlarm(RainbowChain* pChain, int nGuessedPos, unsigned char* pHash, CHashSet& hs);
	void SearchTableChunk(RainbowChain* pChain, int nRainbowChainLen, int nRainbowChainCount, CHashSet& hs);
	void SearchRainbowTable(string sPathName, CHashSet& hs);

public:
	void Run(vector<string> vPathName, CHashSet& hs);
	float GetStatTotalDiskAccessTime();
	float GetStatTotalCryptanalysisTime();
	int   GetStatTotalChainWalkStep();
	int   GetStatTotalFalseAlarm();
	int   GetStatTotalChainWalkStepDueToFalseAlarm();
};

#endif
