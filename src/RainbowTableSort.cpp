/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#include <cstdlib>
#include "Public.h"

#define ASSUMED_MIN_MEMORY 32 * 1024 * 1024

/////////////////////////////////////////////////////////////////////////////

/*
int QuickSortCompare(const void* pElem1, const void* pElem2)
{
	uint64 n1 = ((RainbowChain*)pElem1)->nIndexE;
	uint64 n2 = ((RainbowChain*)pElem2)->nIndexE;

	if (n1 < n2)
		return -1;
	else if (n1 == n2)
		return 0;
	else
		return 1;
}

void QuickSort(RainbowChain* pChain, int nRainbowChainCount)
{
	qsort(pChain, nRainbowChainCount, 16, QuickSortCompare);	// so slow!
}
*/

/////////////////////////////////////////////////////////////////////////////

int QuickSortPartition(RainbowChain* pChain, int nLow, int nHigh)
{
	int nRandomIndex = nLow + ((unsigned int)rand() * (RAND_MAX + 1LL) + (unsigned int)rand()) % (nHigh - nLow + 1);
	RainbowChain TempChain;
	TempChain = pChain[nLow];
	pChain[nLow] = pChain[nRandomIndex];
	pChain[nRandomIndex] = TempChain;

	TempChain = pChain[nLow];
	uint64 nPivotKey = pChain[nLow].nIndexE;
	while (nLow < nHigh)
	{
		while (nLow < nHigh && pChain[nHigh].nIndexE >= nPivotKey)
			nHigh--;
		pChain[nLow] = pChain[nHigh];
		while (nLow < nHigh && pChain[nLow].nIndexE <= nPivotKey)
			nLow++;
		pChain[nHigh] = pChain[nLow];
	}
	pChain[nLow] = TempChain;
	return nLow;
}

void QuickSort(RainbowChain* pChain, int nLow, int nHigh)
{
	if (nLow < nHigh)
	{
		int nPivotLoc = QuickSortPartition(pChain, nLow, nHigh);
		QuickSort(pChain, nLow, nPivotLoc - 1);
		QuickSort(pChain, nPivotLoc + 1, nHigh);
	}
}

/////////////////////////////////////////////////////////////////////////////

#define SORTED_SEGMENT_MAX_CHAIN_IN_MEMORY 1024

class CSortedSegment
{
public:
	CSortedSegment(FILE* file, unsigned int nFileChainOffset, int nFileChainCount)
	{
		m_nChainCount     = 0;
		m_nNextChainIndex = 0;

		m_file             = file;
		m_nFileChainOffset = nFileChainOffset;
		m_nFileChainCount  = nFileChainCount;
	}

private:
	RainbowChain m_Chain[SORTED_SEGMENT_MAX_CHAIN_IN_MEMORY];
	int m_nChainCount;
	int m_nNextChainIndex;

	FILE* m_file;
	unsigned int m_nFileChainOffset;
	int m_nFileChainCount;

public:
	RainbowChain* GetNextChain()	// Don't call this if no chain left
	{
		if (m_nChainCount == m_nNextChainIndex)
		{
			int nChainCountToRead;
			if (m_nFileChainCount < SORTED_SEGMENT_MAX_CHAIN_IN_MEMORY)
				nChainCountToRead = m_nFileChainCount;
			else
				nChainCountToRead = SORTED_SEGMENT_MAX_CHAIN_IN_MEMORY;

			//printf("reading... (offset = %u, chain count = %d)\n", m_nFileChainOffset, nChainCountToRead);
			fseek(m_file, m_nFileChainOffset, SEEK_SET);
			fread(m_Chain, 1, sizeof(RainbowChain) * nChainCountToRead, m_file);
			m_nChainCount       = nChainCountToRead;
			m_nNextChainIndex   = 0;
			m_nFileChainOffset += sizeof(RainbowChain) * nChainCountToRead;
			m_nFileChainCount  -= nChainCountToRead;
		}

		return &m_Chain[m_nNextChainIndex];
	}

	bool RemoveTopChain()	// return whether already empty
	{
		m_nNextChainIndex++;

		if (m_nChainCount == m_nNextChainIndex)
			if (m_nFileChainCount == 0)
				return true;

		return false;
	}
};

FILE* CreateTemporaryFile(string sPathName, unsigned int nLen)
{
	FILE* tempfile = fopen(sPathName.c_str(), "w+b");
	if (tempfile == NULL)
	{
		printf("can't create temporary file %s\n", sPathName.c_str());
		return NULL;
	}

	// Set proper length - this is not a good method
	fseek(tempfile, nLen - 4, SEEK_SET);
	int x;
	fwrite(&x, 1, 4, tempfile);
	if (GetFileLen(tempfile) != nLen)
	{
		printf("not enough temporary disk space, %u bytes required\n", nLen);
		fclose(tempfile);
		remove(sPathName.c_str());
		return NULL;
	}

	return tempfile;
}

bool PrepareSortedSegment(list<CSortedSegment>& lSS, FILE* file, FILE* tempfile)
{
	unsigned int nAvailPhys = GetAvailPhysMemorySize();
	if (nAvailPhys < ASSUMED_MIN_MEMORY)
		nAvailPhys = ASSUMED_MIN_MEMORY;
	nAvailPhys = nAvailPhys / 16 * 16;

	// Allocate memory
	unsigned char* pMem = new unsigned char[nAvailPhys];
	if (pMem == NULL)
	{
		printf("memory allocation fail\n");
		return false;
	}

	// Run
	unsigned int nFileLen = GetFileLen(file);
	fseek(file, 0, SEEK_SET);
	fseek(tempfile, 0, SEEK_SET);
	int i;
	for (i = 0; ; i++)
	{
		if (ftell(file) == nFileLen)
			break;

		printf("reading segment #%d...\n", i);
		unsigned int nRead = fread(pMem, 1, nAvailPhys, file);

		printf("sorting segment #%d...\n", i);
		QuickSort((RainbowChain*)pMem, 0, nRead / 16 - 1);

		CSortedSegment ss(tempfile, ftell(tempfile), nRead / 16);
		lSS.push_back(ss);

		printf("writing sorted segment #%d...\n", i);
		fwrite(pMem, 1, nRead, tempfile);
	}

	// Free memory
	delete pMem;

	return true;
}

void MergeSortedSegment(list<CSortedSegment>& lSS, FILE* file)
{
	printf("merging sorted segments...\n");

	fseek(file, 0, SEEK_SET);
	while (!lSS.empty())
	{
		list<CSortedSegment>::iterator MinIt;
		uint64 nMinIndexE;
		list<CSortedSegment>::iterator it;
		for (it = lSS.begin(); it != lSS.end(); it++)
		{
			if (it == lSS.begin())
			{
				MinIt = it;
				nMinIndexE = ((*it).GetNextChain())->nIndexE;
			}
			else
			{
				if (((*it).GetNextChain())->nIndexE < nMinIndexE)
				{
					MinIt = it;
					nMinIndexE = ((*it).GetNextChain())->nIndexE;
				}
			}
		}

		fwrite((*MinIt).GetNextChain(), 1, 16, file);

		if ((*MinIt).RemoveTopChain())
			lSS.erase(MinIt);
	}
}

void ExternalSort(FILE* file, string sTemporaryFilePathName)
{
	FILE* tempfile = CreateTemporaryFile(sTemporaryFilePathName, GetFileLen(file));
	if (tempfile != NULL)
	{
		list<CSortedSegment> lSS;
		if (PrepareSortedSegment(lSS, file, tempfile))
			MergeSortedSegment(lSS, file);
		fclose(tempfile);
		remove(sTemporaryFilePathName.c_str());
	}
}

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		Logo();

		printf("usage: rtsort rainbow_table_pathname\n");
		return 0;
	}
	string sPathName = argv[1];

	// Open file
	FILE* file = fopen(sPathName.c_str(), "r+b");
	if (file == NULL)
	{
		printf("failed to open %s\n", sPathName.c_str());
		return 0;
	}

	// Sort
	unsigned int nFileLen = GetFileLen(file);
	if (nFileLen % 16 != 0)
		printf("rainbow table size check fail\n");
	else
	{
		// Available physical memory
		unsigned int nAvailPhys = GetAvailPhysMemorySize();
		printf("available physical memory: %u bytes\n", nAvailPhys);

		// QuickSort or ExternalSort
		if (nAvailPhys >= nFileLen || nFileLen <= ASSUMED_MIN_MEMORY)
		{
			int nRainbowChainCount = nFileLen / 16;
			RainbowChain* pChain = (RainbowChain*)new unsigned char[nFileLen];
			if (pChain != NULL)
			{
				// Load file
				printf("loading rainbow table...\n");
				fseek(file, 0, SEEK_SET);
				if (fread(pChain, 1, nFileLen, file) != nFileLen)
				{
					printf("disk read fail\n");
					goto ABORT;
				}

				// Sort file
				printf("sorting rainbow table...\n");
				QuickSort(pChain, 0, nRainbowChainCount - 1);

				// Write file
				printf("writing sorted rainbow table...\n");
				fseek(file, 0, SEEK_SET);
				fwrite(pChain, 1, nFileLen, file);

				delete[] pChain;
			}
			else
				printf("memory allocation fail\n");
		}
		else
		{
			// External sort when memory is low
			ExternalSort(file, sPathName + ".tmp");
		}
	}
ABORT:

	// Close file
	fclose(file);

	return 0;
}
