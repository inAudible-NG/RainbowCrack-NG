/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include "CrackEngine.h"

#include <time.h>

CCrackEngine::CCrackEngine()
{
	ResetStatistics();
}

CCrackEngine::~CCrackEngine()
{
}

//////////////////////////////////////////////////////////////////////

void CCrackEngine::ResetStatistics()
{
	m_fTotalDiskAccessTime               = 0.0f;
	m_fTotalCryptanalysisTime            = 0.0f;
	m_nTotalChainWalkStep                = 0;
	m_nTotalFalseAlarm                   = 0;
	m_nTotalChainWalkStepDueToFalseAlarm = 0;
}

int CCrackEngine::BinarySearch(RainbowChain* pChain, int nRainbowChainCount, uint64 nIndex)
{
	int nLow = 0;
	int nHigh = nRainbowChainCount - 1;
	while (nLow <= nHigh)
	{
		int nMid = (nLow + nHigh) / 2;
		if (nIndex == pChain[nMid].nIndexE)
			return nMid;
		else if (nIndex < pChain[nMid].nIndexE)
			nHigh = nMid - 1;
		else
			nLow = nMid + 1;
	}

	return -1;
}

void CCrackEngine::GetChainIndexRangeWithSameEndpoint(RainbowChain* pChain,
		int nRainbowChainCount,
		int nMatchingIndexE,
		int& nMatchingIndexEFrom,
		int& nMatchingIndexETo)
{
	nMatchingIndexEFrom = nMatchingIndexE;
	nMatchingIndexETo   = nMatchingIndexE;
	while (nMatchingIndexEFrom > 0)
	{
		if (pChain[nMatchingIndexEFrom - 1].nIndexE == pChain[nMatchingIndexE].nIndexE)
			nMatchingIndexEFrom--;
		else
			break;
	}
	while (nMatchingIndexETo < nRainbowChainCount - 1)
	{
		if (pChain[nMatchingIndexETo + 1].nIndexE == pChain[nMatchingIndexE].nIndexE)
			nMatchingIndexETo++;
		else
			break;
	}
}

bool CCrackEngine::CheckAlarm(RainbowChain* pChain, int nGuessedPos, unsigned char* pHash, CHashSet& hs)
{
	CChainWalkContext cwc;
	cwc.SetIndex(pChain->nIndexS);
	int nPos;
	for (nPos = 0; nPos < nGuessedPos; nPos++)
	{
		cwc.IndexToPlain();
		cwc.PlainToHash();
		cwc.HashToIndex(nPos);
	}
	cwc.IndexToPlain();
	cwc.PlainToHash();
	if (cwc.CheckHash(pHash))
	{
		printf("plaintext of %s is %s\n", cwc.GetHash().c_str(), cwc.GetPlain().c_str());
		hs.SetPlain(cwc.GetHash(), cwc.GetPlain(), cwc.GetBinary());
		return true;
	}

	return false;
}

void CCrackEngine::SearchTableChunk(RainbowChain* pChain, int nRainbowChainLen, int nRainbowChainCount, CHashSet& hs)
{
	vector<string> vHash;
	hs.GetLeftHashWithLen(vHash, CChainWalkContext::GetHashLen());
	printf("searching for %lu hash%s...\n", vHash.size(),
			vHash.size() > 1 ? "es" : "");

	int nChainWalkStep = 0;
	int nFalseAlarm = 0;
	int nChainWalkStepDueToFalseAlarm = 0;

	int nHashIndex;
	for (nHashIndex = 0; nHashIndex < vHash.size(); nHashIndex++)
	{
		unsigned char TargetHash[MAX_HASH_LEN];
		int nHashLen;
		ParseHash(vHash[nHashIndex], TargetHash, nHashLen);
		if (nHashLen != CChainWalkContext::GetHashLen())
			printf("debug: nHashLen mismatch\n");

		// Rqeuest ChainWalk
		bool fNewlyGenerated;
		uint64* pStartPosIndexE = m_cws.RequestWalk(TargetHash,
													nHashLen,
													CChainWalkContext::GetHashRoutineName(),
													CChainWalkContext::GetPlainCharsetName(),
													CChainWalkContext::GetPlainLenMin(),
													CChainWalkContext::GetPlainLenMax(),
													CChainWalkContext::GetRainbowTableIndex(),
													nRainbowChainLen,
													fNewlyGenerated);
		//printf("debug: using %s walk for %s\n", fNewlyGenerated ? "newly generated" : "existing",
		//										vHash[nHashIndex].c_str());

		// Walk
		int nPos;
		for (nPos = nRainbowChainLen - 2; nPos >= 0; nPos--)
		{
			if (fNewlyGenerated)
			{
				CChainWalkContext cwc;
				cwc.SetHash(TargetHash);
				cwc.HashToIndex(nPos);
				int i;
				for (i = nPos + 1; i <= nRainbowChainLen - 2; i++)
				{
					cwc.IndexToPlain();
					cwc.PlainToHash();
					cwc.HashToIndex(i);
				}

				pStartPosIndexE[nPos] = cwc.GetIndex();
				nChainWalkStep += nRainbowChainLen - 2 - nPos;
			}
			uint64 nIndexEOfCurPos = pStartPosIndexE[nPos];

			// Search matching nIndexE
			int nMatchingIndexE = BinarySearch(pChain, nRainbowChainCount, nIndexEOfCurPos);
			if (nMatchingIndexE != -1)
			{
				int nMatchingIndexEFrom, nMatchingIndexETo;
				GetChainIndexRangeWithSameEndpoint(pChain, nRainbowChainCount,
						nMatchingIndexE,
						nMatchingIndexEFrom, nMatchingIndexETo);
				int i;
				for (i = nMatchingIndexEFrom; i <= nMatchingIndexETo; i++)
				{
					if (CheckAlarm(pChain + i, nPos, TargetHash, hs))
					{
						//printf("debug: discarding walk for %s\n", vHash[nHashIndex].c_str());
						m_cws.DiscardWalk(pStartPosIndexE);
						goto NEXT_HASH;
					}
					else
					{
						nChainWalkStepDueToFalseAlarm += nPos + 1;
						nFalseAlarm++;
					}
				}
			}
		}
NEXT_HASH:;
	}

	//printf("debug: chain walk step: %d\n", nChainWalkStep);
	//printf("debug: false alarm: %d\n", nFalseAlarm);
	//printf("debug: chain walk step due to false alarm: %d\n", nChainWalkStepDueToFalseAlarm);

	m_nTotalChainWalkStep += nChainWalkStep;
	m_nTotalFalseAlarm += nFalseAlarm;
	m_nTotalChainWalkStepDueToFalseAlarm += nChainWalkStepDueToFalseAlarm;
}

void CCrackEngine::SearchRainbowTable(string sPathName, CHashSet& hs)
{
	// FileName
#ifdef _WIN32
	int nIndex = sPathName.find_last_of('\\');
#else
	int nIndex = sPathName.find_last_of('/');
#endif
	string sFileName;
	if (nIndex != -1)
		sFileName = sPathName.substr(nIndex + 1);
	else
		sFileName = sPathName;

	// Info
	printf("%s:\n", sFileName.c_str());

	// Setup
	int nRainbowChainLen, nRainbowChainCount;
	if (!CChainWalkContext::SetupWithPathName(sPathName, nRainbowChainLen, nRainbowChainCount))
		return;

	// Already finished?
	if (!hs.AnyHashLeftWithLen(CChainWalkContext::GetHashLen()))
	{
		printf("this table contains hashes with length %d only\n", CChainWalkContext::GetHashLen());
		return;
	}

	// Open
	FILE* file = fopen(sPathName.c_str(), "rb");
	if (file != NULL)
	{
		// File length check
		unsigned int nFileLen = GetFileLen(file);
		if (nFileLen % 16 != 0 || nRainbowChainCount * 16 != nFileLen)
			printf("file length mismatch\n");
		else
		{
			static CMemoryPool mp;
			unsigned int nAllocatedSize;
			RainbowChain* pChain = (RainbowChain*)mp.Allocate(nFileLen, nAllocatedSize);
			if (pChain != NULL)
			{
				nAllocatedSize = nAllocatedSize / 16 * 16;		// Round to 16-byte boundary

				fseek(file, 0, SEEK_SET);
				bool fVerified = false;
				while (true)	// Chunk read loop
				{
					if (ftell(file) == nFileLen)
						break;

					// Load table chunk
					//printf("reading...\n");
					clock_t t1 = clock();
					unsigned int nDataRead = fread(pChain, 1, nAllocatedSize, file);
					clock_t t2 = clock();
					float fTime = 1.0f * (t2 - t1) / CLOCKS_PER_SEC;
					printf("%u bytes read, disk access time: %.2f s\n", nDataRead, fTime);
					m_fTotalDiskAccessTime += fTime;

					int nRainbowChainCountRead = nDataRead / 16;

					// Verify table chunk
					if (!fVerified)
					{
						printf("verifying the file...\n");

						// Chain length test
						int nIndexToVerify = nRainbowChainCountRead / 2;
						CChainWalkContext cwc;
						cwc.SetIndex(pChain[nIndexToVerify].nIndexS);
						int nPos;
						for (nPos = 0; nPos < nRainbowChainLen - 1; nPos++)
						{
							cwc.IndexToPlain();
							cwc.PlainToHash();
							cwc.HashToIndex(nPos);
						}
						if (cwc.GetIndex() != pChain[nIndexToVerify].nIndexE)
						{
							printf("rainbow chain length verify fail\n");
							break;
						}

						// Chain sort test
						int i;
						for (i = 0; i < nRainbowChainCountRead - 1; i++)
						{
							if (pChain[i].nIndexE > pChain[i + 1].nIndexE)
								break;
						}
						if (i != nRainbowChainCountRead - 1)
						{
							printf("this file is not sorted\n");
							break;
						}

						fVerified = true;
					}

					// Search table chunk
					t1 = clock();
					SearchTableChunk(pChain, nRainbowChainLen, nRainbowChainCountRead, hs);
					t2 = clock();
					fTime = 1.0f * (t2 - t1) / CLOCKS_PER_SEC;
					printf("cryptanalysis time: %.2f s\n", fTime);
					m_fTotalCryptanalysisTime += fTime;

					// Already finished?
					if (!hs.AnyHashLeftWithLen(CChainWalkContext::GetHashLen()))
						break;
				}
			}
			else
				printf("memory allocation fail\n");
		}

		fclose(file);
	}
	else
		printf("can't open file\n");
}

void CCrackEngine::Run(vector<string> vPathName, CHashSet& hs)
{
	// Reset statistics
	ResetStatistics();

	// Sort vPathName (CChainWalkSet need it)
	int i, j;
	for (i = 0; i < vPathName.size() - 1; i++)
		for (j = 0; j < vPathName.size() - i - 1; j++)
		{
			if (vPathName[j] > vPathName[j + 1])
			{
				string sTemp;
				sTemp = vPathName[j];
				vPathName[j] = vPathName[j + 1];
				vPathName[j + 1] = sTemp;
			}
		}

	// Run
	for (i = 0; i < vPathName.size() && hs.AnyhashLeft(); i++)
	{
		SearchRainbowTable(vPathName[i], hs);
		printf("\n");
	}
}

float CCrackEngine::GetStatTotalDiskAccessTime()
{
	return m_fTotalDiskAccessTime;
}

float CCrackEngine::GetStatTotalCryptanalysisTime()
{
	return m_fTotalCryptanalysisTime;
}

int CCrackEngine::GetStatTotalChainWalkStep()
{
	return m_nTotalChainWalkStep;
}

int CCrackEngine::GetStatTotalFalseAlarm()
{
	return m_nTotalFalseAlarm;
}

int CCrackEngine::GetStatTotalChainWalkStepDueToFalseAlarm()
{
	return m_nTotalChainWalkStepDueToFalseAlarm;
}
