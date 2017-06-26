/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#include <algorithm>
#include "ChainWalkContext.h"

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		Logo();

		printf("usage: rtdump rainbow_table_pathname rainbow_chain_index\n");
		return 0;
	}
	string sPathName = argv[1];
	int nRainbowChainIndex = atoi(argv[2]);

	int nRainbowChainLen, nRainbowChainCount;
	if (!CChainWalkContext::SetupWithPathName(sPathName, nRainbowChainLen, nRainbowChainCount))
		return 0;
	if (nRainbowChainIndex < 0 || nRainbowChainIndex > nRainbowChainCount - 1)
	{
		printf("valid rainbow chain index range: 0 - %d\n", nRainbowChainCount - 1);
		return 0;
	}

	// Open file
	FILE* file = fopen(sPathName.c_str(), "rb");
	if (file == NULL)
	{
		printf("failed to open %s\n", sPathName.c_str());
		return 0;
	}

	// Dump
	if (GetFileLen(file) != nRainbowChainCount * 16)
		printf("rainbow table size check fail\n");
	else
	{
		// Read required chain
		RainbowChain chain;
		fseek(file, nRainbowChainIndex * 16, SEEK_SET);
		fread(&chain, 1, 16, file);

		// Dump required chain
		CChainWalkContext cwc;
		cwc.SetIndex(chain.nIndexS);
		int nPos;
		for (nPos = 0; nPos < nRainbowChainLen - 1; nPos++)
		{
			cwc.IndexToPlain();
			cwc.PlainToHash();
			printf("#%-4d  %s  %s  %s\n", nPos,
										  uint64tohexstr(cwc.GetIndex()).c_str(),
										  cwc.GetPlainBinary().c_str(),
										  cwc.GetHash().c_str());
			cwc.HashToIndex(nPos);
		}
		printf("#%-4d  %s\n", nPos, uint64tohexstr(cwc.GetIndex()).c_str());
		if (cwc.GetIndex() != chain.nIndexE)
			printf("\nwarning: rainbow chain integrity check fail!\n");
	}

	// Close file
	fclose(file);

	return 0;
}
