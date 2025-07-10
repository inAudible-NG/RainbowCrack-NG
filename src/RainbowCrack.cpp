/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include "CrackEngine.h"

#ifdef _WIN32
	#include <io.h>
	#include <sys/stat.h>  // _A_SUBDIR
#else
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
#endif

#include <cstring>
#include <openssl/md4.h>
#ifdef _WIN32
	#pragma comment(lib, "libcrypto.lib")
#endif

//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
void GetTableList(const std::string& sWildCharPathName, std::vector<std::string>& vPathName)
{
    vPathName.clear();

    // Извлечение пути к папке (до последнего '\')
    std::string sPath;
    size_t n = sWildCharPathName.find_last_of("\\/");
    if (n != std::string::npos)
        sPath = sWildCharPathName.substr(0, n + 1);
    else
        sPath = "";  // fallback, если пути нет

    _finddata_t fd;
    intptr_t handle = _findfirst(sWildCharPathName.c_str(), &fd);

    if (handle == -1L) {
        printf("[GetTableList] No files found for pattern: %s\n", sWildCharPathName.c_str());
        return;
    }

    do {
        // Пропускаем папки и служебные имена
        if (!(fd.attrib & _A_SUBDIR) && strcmp(fd.name, ".") != 0 && strcmp(fd.name, "..") != 0)
        {
            std::string sPathName = sPath + fd.name;
            vPathName.push_back(sPathName);
        }
    } while (_findnext(handle, &fd) == 0);

    _findclose(handle);
}
#else
void GetTableList(int argc, char* argv[], vector<string>& vPathName)
{
	printf("GetTableList NOT WIN32 Begin\n");
	vPathName.clear();

	int i;
	for (i = 1; i <= argc - 3; i++)
	{
		string sPathName = argv[i];

		struct stat buf;
		if (lstat(sPathName.c_str(), &buf) == 0)
		{
			if (S_ISREG(buf.st_mode))
				vPathName.push_back(sPathName);
		}
	}
}
#endif

bool NormalizeHash(string& sHash)
{
	string sNormalizedHash = sHash;

	if (   sNormalizedHash.size() % 2 != 0
		|| sNormalizedHash.size() < MIN_HASH_LEN * 2
		|| sNormalizedHash.size() > MAX_HASH_LEN * 2)
		return false;

	// Make lower
	int i;
	for (i = 0; i < sNormalizedHash.size(); i++)
	{
		if (sNormalizedHash[i] >= 'A' && sNormalizedHash[i] <= 'F')
			sNormalizedHash[i] = sNormalizedHash[i] - 'A' + 'a';
	}

	// Character check
	for (i = 0; i < sNormalizedHash.size(); i++)
	{
		if (   (sNormalizedHash[i] < 'a' || sNormalizedHash[i] > 'f')
			&& (sNormalizedHash[i] < '0' || sNormalizedHash[i] > '9'))
			return false;
	}

	sHash = sNormalizedHash;
	return true;
}

void LoadLMHashFromPwdumpFile(string sPathName, vector<string>& vUserName, vector<string>& vLMHash, vector<string>& vNTLMHash)
{
	vector<string> vLine;
	if (ReadLinesFromFile(sPathName, vLine))
	{
		int i;
		for (i = 0; i < vLine.size(); i++)
		{
			vector<string> vPart;
			if (SeperateString(vLine[i], "::::", vPart))
			{
				string sUserName = vPart[0];
				string sLMHash   = vPart[2];
				string sNTLMHash = vPart[3];

				if (sLMHash.size() == 32 && sNTLMHash.size() == 32)
				{
					if (NormalizeHash(sLMHash) && NormalizeHash(sNTLMHash))
					{
						vUserName.push_back(sUserName);
						vLMHash.push_back(sLMHash);
						vNTLMHash.push_back(sNTLMHash);
					}
					else
						printf("invalid lm/ntlm hash %s:%s\n", sLMHash.c_str(), sNTLMHash.c_str());
				}
			}
		}
	}
	else
		printf("can't open %s\n", sPathName.c_str());
}

bool NTLMPasswordSeek(unsigned char* pLMPassword, int nLMPasswordLen, int nLMPasswordNext,
		unsigned char* pNTLMHash, string& sNTLMPassword)
{
	if (nLMPasswordNext == nLMPasswordLen)
	{
		unsigned char md[16];
		MD4(pLMPassword, nLMPasswordLen * 2, md);
		if (memcmp(md, pNTLMHash, 16) == 0)
		{
			sNTLMPassword = "";
			int i;
			for (i = 0; i < nLMPasswordLen; i++)
				sNTLMPassword += char(pLMPassword[i * 2]);
			return true;
		}
		else
			return false;
	}

	if (NTLMPasswordSeek(pLMPassword, nLMPasswordLen, nLMPasswordNext + 1, pNTLMHash, sNTLMPassword))
		return true;

	if (   pLMPassword[nLMPasswordNext * 2] >= 'A'
		&& pLMPassword[nLMPasswordNext * 2] <= 'Z')
	{
		pLMPassword[nLMPasswordNext * 2] = pLMPassword[nLMPasswordNext * 2] - 'A' + 'a';
		if (NTLMPasswordSeek(pLMPassword, nLMPasswordLen, nLMPasswordNext + 1, pNTLMHash, sNTLMPassword))
			return true;
		pLMPassword[nLMPasswordNext * 2] = pLMPassword[nLMPasswordNext * 2] - 'a' + 'A';
	}

	return false;
}

bool LMPasswordCorrectCase(string sLMPassword, unsigned char* pNTLMHash, string& sNTLMPassword)
{
	if (sLMPassword.size() == 0)
	{
		sNTLMPassword = "";
		return true;
	}

	unsigned char* pLMPassword = new unsigned char[sLMPassword.size() * 2];
	int i;
	for (i = 0; i < sLMPassword.size(); i++)
	{
		pLMPassword[i * 2    ] = sLMPassword[i];
		pLMPassword[i * 2 + 1] = 0x00;
	}
	bool fRet = NTLMPasswordSeek(pLMPassword, sLMPassword.size(), 0, pNTLMHash, sNTLMPassword);
	delete pLMPassword;

	return fRet;
}

void Usage()
{
	Logo();

	printf("usage: rcrack rainbow_table_pathname -h hash\n");
	printf("       rcrack rainbow_table_pathname -l hash_list_file\n");
	printf("       rcrack rainbow_table_pathname -f pwdump_file\n");
	printf("rainbow_table_pathname: pathname of the rainbow table(s), wildchar(*, ?) supported\n");
	printf("-h hash:                use raw hash as input\n");
	printf("-l hash_list_file:      use hash list file as input, each hash in a line\n");
	printf("-f pwdump_file:         use pwdump file as input, this will handle lanmanager hash only\n");
	printf("\n");
	printf("example: rcrack *.rt -h 5d41402abc4b2a76b9719d911017c592\n");
	printf("         rcrack *.rt -l hash.txt\n");
	printf("         rcrack *.rt -f hash.txt\n");
}

int main(int argc, char* argv[])
{
	/// printf("Begin\n");
#ifdef _WIN32
	/// printf("IFDEF WIN32 ST\n");
	if (argc != 4)
	{
		Usage();
		/// printf("Error U");
		return 0;
	}
	string sWildCharPathName = argv[1];
	string sInputType        = argv[2];
	string sInput            = argv[3];
	/// printf("IFDEF WIN32 SET STRINGS\n");

	// vPathName
	vector<string> vPathName;
	/// printf("IFDEF WIN32 SET vPathName\n");
	/// printf("GetTableList path raw: [%s]\n", sWildCharPathName.c_str());
	/// for (size_t i = 0; i < sWildCharPathName.length(); ++i)
	///     printf("%02X ", (unsigned char)sWildCharPathName[i]);
	/// printf("\n");
	GetTableList(sWildCharPathName, vPathName);
	//GetTableList(sWildCharPathName, vPathName);
	/// printf("IFDEF WIN32 COMP\n");
#else
	/// printf("IFDEF NOT WIN32 ST\n");
	if (argc < 4)
	{
		Usage();
		/// printf("Error U\n");
		return 0;
	}
	string sInputType        = argv[argc - 2];
	string sInput            = argv[argc - 1];

	// vPathName
	vector<string> vPathName;
	GetTableList(argc, argv, vPathName);
	/// printf("IFDEF NOT WIN32 COMP\n");
#endif
	/// printf("IFDEF WIN32 AFT\n");
	if (vPathName.size() == 0)
	{
		printf("no rainbow table found\n");
		return 0;
	}

	// fCrackerType, vHash, vUserName, vLMHash
	bool fCrackerType;			// true: hash cracker, false: lm cracker
	vector<string> vHash;		// hash cracker
	vector<string> vUserName;	// lm cracker
	vector<string> vLMHash;		// lm cracker
	vector<string> vNTLMHash;	// lm cracker
	if (sInputType == "-h")
	{
		fCrackerType = true;

		string sHash = sInput;
		if (NormalizeHash(sHash))
			vHash.push_back(sHash);
		else
			printf("invalid hash: %s\n", sHash.c_str());
	}
	else if (sInputType == "-l")
	{
		fCrackerType = true;

		string sPathName = sInput;
		vector<string> vLine;
		if (ReadLinesFromFile(sPathName, vLine))
		{
			int i;
			for (i = 0; i < vLine.size(); i++)
			{
				string sHash = vLine[i];
				if (NormalizeHash(sHash))
					vHash.push_back(sHash);
				else
					printf("invalid hash: %s\n", sHash.c_str());
			}
		}
		else
			printf("can't open %s\n", sPathName.c_str());
	}
	else if (sInputType == "-f")
	{
		fCrackerType = false;

		string sPathName = sInput;
		LoadLMHashFromPwdumpFile(sPathName, vUserName, vLMHash, vNTLMHash);
	}
	else
	{
		Usage();
		printf("Error U");
		return 0;
	}

	if (fCrackerType && vHash.size() == 0){
		printf("Error 1");
		return 0;
	}
	if (!fCrackerType && vLMHash.size() == 0){
		printf("Error 2");
		return 0;
	}

	// hs
	CHashSet hs;
	if (fCrackerType)
	{
		int i;
		for (i = 0; i < vHash.size(); i++)
			hs.AddHash(vHash[i]);
	}
	else
	{
		int i;
		for (i = 0; i < vLMHash.size(); i++)
		{
			hs.AddHash(vLMHash[i].substr(0, 16));
			hs.AddHash(vLMHash[i].substr(16, 16));
		}
	}

	// Run
	/// printf("Running CE\n");
	CCrackEngine ce;
	ce.Run(vPathName, hs);

	// Statistics
	printf("statistics\n");
	printf("-------------------------------------------------------\n");
	printf("plaintext found:          %d of %d (%.2f%%)\n", hs.GetStatHashFound(),
															hs.GetStatHashTotal(),
															100.0f * hs.GetStatHashFound() / hs.GetStatHashTotal());
	printf("total disk access time:   %.2f s\n", ce.GetStatTotalDiskAccessTime());
	printf("total cryptanalysis time: %.2f s\n", ce.GetStatTotalCryptanalysisTime());
	printf("total chain walk step:    %d\n",     ce.GetStatTotalChainWalkStep());
	printf("total false alarm:        %d\n",     ce.GetStatTotalFalseAlarm());
	printf("total chain walk step due to false alarm: %d\n", ce.GetStatTotalChainWalkStepDueToFalseAlarm());
	printf("\n");

	// Result
	printf("result\n");
	printf("-------------------------------------------------------\n");
	if (fCrackerType)
	{
		int i;
		for (i = 0; i < vHash.size(); i++)
		{
			string sPlain, sBinary;
			if (!hs.GetPlain(vHash[i], sPlain, sBinary))
			{
				sPlain  = "<notfound>";
				sBinary = "<notfound>";
			}

			printf("%s  %s  hex:%s\n", vHash[i].c_str(), sPlain.c_str(), sBinary.c_str());
		}
	}
	else
	{
		int i;
		for (i = 0; i < vLMHash.size(); i++)
		{
			string sPlain1, sBinary1;
			bool fPart1Found = hs.GetPlain(vLMHash[i].substr(0, 16), sPlain1, sBinary1);
			if (!fPart1Found)
			{
				sPlain1  = "<notfound>";
				sBinary1 = "<notfound>";
			}

			string sPlain2, sBinary2;
			bool fPart2Found = hs.GetPlain(vLMHash[i].substr(16, 16), sPlain2, sBinary2);
			if (!fPart2Found)
			{
				sPlain2  = "<notfound>";
				sBinary2 = "<notfound>";
			}

			string sPlain = sPlain1 + sPlain2;
			string sBinary = sBinary1 + sBinary2;

			// Correct case
			if (fPart1Found && fPart2Found)
			{
				unsigned char NTLMHash[16];
				int nHashLen;
				ParseHash(vNTLMHash[i], NTLMHash, nHashLen);
				if (nHashLen != 16)
					printf("debug: nHashLen mismatch\n");
				string sNTLMPassword;
				if (LMPasswordCorrectCase(sPlain, NTLMHash, sNTLMPassword))
				{
					sPlain = sNTLMPassword;
					sBinary = HexToStr((const unsigned char*)sNTLMPassword.c_str(), sNTLMPassword.size());
				}
				else
					printf("case correction for password %s fail!\n", sPlain.c_str());
			}

			// Display
			printf("%-14s  %s  hex:%s\n", vUserName[i].c_str(),
					sPlain.c_str(),
					sBinary.c_str());
		}
	}

	/// printf("Exit");
	return 0;
}
