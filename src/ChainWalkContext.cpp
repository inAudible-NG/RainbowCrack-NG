/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include "ChainWalkContext.h"

#include <cstring>
#include <ctype.h>
#include <openssl/rand.h>
#ifdef _WIN32
	#pragma comment(lib, "libcrypto.lib")
#endif

//////////////////////////////////////////////////////////////////////

string CChainWalkContext::m_sHashRoutineName;
HASHROUTINE CChainWalkContext::m_pHashRoutine;
int CChainWalkContext::m_nHashLen;

unsigned char CChainWalkContext::m_PlainCharset[256];
int CChainWalkContext::m_nPlainCharsetLen;
int CChainWalkContext::m_nPlainLenMin;
int CChainWalkContext::m_nPlainLenMax;
string CChainWalkContext::m_sPlainCharsetName;
string CChainWalkContext::m_sPlainCharsetContent;
uint64 CChainWalkContext::m_nPlainSpaceUpToX[MAX_PLAIN_LEN + 1];
uint64 CChainWalkContext::m_nPlainSpaceTotal;

int CChainWalkContext::m_nRainbowTableIndex;
uint64 CChainWalkContext::m_nReduceOffset;

//////////////////////////////////////////////////////////////////////

CChainWalkContext::CChainWalkContext()
{
}

CChainWalkContext::~CChainWalkContext()
{
}

bool CChainWalkContext::LoadCharset(string sName)
{
	if (sName == "byte")
	{
		int i;
		for (i = 0x00; i <= 0xff; i++)
			m_PlainCharset[i] = i;
		m_nPlainCharsetLen = 256;
		m_sPlainCharsetName = sName;
		m_sPlainCharsetContent = "0x00, 0x01, ... 0xff";
		return true;
	}

	vector<string> vLine;
	if (ReadLinesFromFile("charset.txt", vLine))
	{
		int i;
		for (i = 0; i < vLine.size(); i++)
		{
			// Filter comment
			if (vLine[i][0] == '#')
				continue;

			vector<string> vPart;
			if (SeperateString(vLine[i], "=", vPart))
			{
				// sCharsetName
				string sCharsetName = TrimString(vPart[0]);
				if (sCharsetName == "")
					continue;

				// sCharsetName charset check
				bool fCharsetNameCheckPass = true;
				int j;
				for (j = 0; j < sCharsetName.size(); j++)
				{
					if (   !isalpha(sCharsetName[j])
						&& !isdigit(sCharsetName[j])
						&& (sCharsetName[j] != '-'))
					{
						fCharsetNameCheckPass = false;
						break;
					}
				}
				if (!fCharsetNameCheckPass)
				{
					printf("invalid charset name %s in charset configuration file\n", sCharsetName.c_str());
					continue;
				}

				// sCharsetContent
				string sCharsetContent = TrimString(vPart[1]);
				if (sCharsetContent == "" || sCharsetContent == "[]")
					continue;
				if (sCharsetContent[0] != '[' || sCharsetContent[sCharsetContent.size() - 1] != ']')
				{
					printf("invalid charset content %s in charset configuration file\n", sCharsetContent.c_str());
					continue;
				}
				sCharsetContent = sCharsetContent.substr(1, sCharsetContent.size() - 2);
				if (sCharsetContent.size() > 256)
				{
					printf("charset content %s too long\n", sCharsetContent.c_str());
					continue;
				}

				//printf("%s = [%s]\n", sCharsetName.c_str(), sCharsetContent.c_str());

				// Is it the wanted charset?
				if (sCharsetName == sName)
				{
					m_nPlainCharsetLen = sCharsetContent.size();
					memcpy(m_PlainCharset, sCharsetContent.c_str(), m_nPlainCharsetLen);
					m_sPlainCharsetName = sCharsetName;
					m_sPlainCharsetContent = sCharsetContent;
					return true;
				}
			}
		}
		printf("charset %s not found in charset.txt\n", sName.c_str());
	}
	else
		printf("can't open charset configuration file\n");

	return false;
}

//////////////////////////////////////////////////////////////////////

bool CChainWalkContext::SetHashRoutine(string sHashRoutineName)
{
	CHashRoutine hr;
	hr.GetHashRoutine(sHashRoutineName, m_pHashRoutine, m_nHashLen);
	if (m_pHashRoutine != NULL)
	{
		m_sHashRoutineName = sHashRoutineName;
		return true;
	}
	else
		return false;
}

bool CChainWalkContext::SetPlainCharset(string sCharsetName, int nPlainLenMin, int nPlainLenMax)
{
	// m_PlainCharset, m_nPlainCharsetLen, m_sPlainCharsetName, m_sPlainCharsetContent
	if (!LoadCharset(sCharsetName))
		return false;

	// m_nPlainLenMin, m_nPlainLenMax
	if (nPlainLenMin < 1 || nPlainLenMax > MAX_PLAIN_LEN || nPlainLenMin > nPlainLenMax)
	{
		printf("invalid plaintext length range: %d - %d\n", nPlainLenMin, nPlainLenMax);
		return false;
	}
	m_nPlainLenMin = nPlainLenMin;
	m_nPlainLenMax = nPlainLenMax;

	// m_nPlainSpaceUpToX
	m_nPlainSpaceUpToX[0] = 0;
	uint64 nTemp = 1;
	int i;
	for (i = 1; i <= m_nPlainLenMax; i++)
	{
		nTemp *= m_nPlainCharsetLen;
		if (i < m_nPlainLenMin)
			m_nPlainSpaceUpToX[i] = 0;
		else
			m_nPlainSpaceUpToX[i] = m_nPlainSpaceUpToX[i - 1] + nTemp;
	}

	// m_nPlainSpaceTotal
	m_nPlainSpaceTotal = m_nPlainSpaceUpToX[m_nPlainLenMax];

	return true;
}

bool CChainWalkContext::SetRainbowTableIndex(int nRainbowTableIndex)
{
	if (nRainbowTableIndex < 0)
		return false;
	m_nRainbowTableIndex = nRainbowTableIndex;
	m_nReduceOffset = 65536 * nRainbowTableIndex;

	return true;
}

bool CChainWalkContext::SetupWithPathName(string sPathName, int& nRainbowChainLen, int& nRainbowChainCount)
{
	// something like lm_alpha#1-7_0_100x16_test.rt

#ifdef _WIN32
	int nIndex = sPathName.find_last_of('\\');
#else
	int nIndex = sPathName.find_last_of('/');
#endif
	if (nIndex != -1)
		sPathName = sPathName.substr(nIndex + 1);

	if (sPathName.size() < 3)
	{
		printf("%s is not a rainbow table\n", sPathName.c_str());
		return false;
	}
	if (sPathName.substr(sPathName.size() - 3) != ".rt")
	{
		printf("%s is not a rainbow table\n", sPathName.c_str());
		return false;
	}

	// Parse
	vector<string> vPart;
	if (!SeperateString(sPathName, "___x_", vPart))
	{
		printf("filename %s not identified\n", sPathName.c_str());
		return false;
	}

	string sHashRoutineName   = vPart[0];
	int nRainbowTableIndex    = atoi(vPart[2].c_str());

	nRainbowChainLen          = atoi(vPart[3].c_str());
	nRainbowChainCount        = atoi(vPart[4].c_str());

	// Parse charset definition
	string sCharsetDefinition = vPart[1];
	string sCharsetName;
	int nPlainLenMin, nPlainLenMax;
	if (sCharsetDefinition.find('#') == -1)		// For backward compatibility, "#1-7" is implied
	{
		sCharsetName = sCharsetDefinition;
		nPlainLenMin = 1;
		nPlainLenMax = 7;
	}
	else
	{
		vector<string> vCharsetDefinitionPart;
		if (!SeperateString(sCharsetDefinition, "#-", vCharsetDefinitionPart))
		{
			printf("filename %s not identified\n", sPathName.c_str());
			return false;
		}
		else
		{
			sCharsetName = vCharsetDefinitionPart[0];
			nPlainLenMin = atoi(vCharsetDefinitionPart[1].c_str());
			nPlainLenMax = atoi(vCharsetDefinitionPart[2].c_str());
		}
	}

	// Setup
	if (!SetHashRoutine(sHashRoutineName))
	{
		printf("hash routine %s not supported\n", sHashRoutineName.c_str());
		return false;
	}
	if (!SetPlainCharset(sCharsetName, nPlainLenMin, nPlainLenMax))
		return false;
	if (!SetRainbowTableIndex(nRainbowTableIndex))
	{
		printf("invalid rainbow table index %d\n", nRainbowTableIndex);
		return false;
	}

	return true;
}

string CChainWalkContext::GetHashRoutineName()
{
	return m_sHashRoutineName;
}

int CChainWalkContext::GetHashLen()
{
	return m_nHashLen;
}

string CChainWalkContext::GetPlainCharsetName()
{
	return m_sPlainCharsetName;
}

string CChainWalkContext::GetPlainCharsetContent()
{
	return m_sPlainCharsetContent;
}

int CChainWalkContext::GetPlainLenMin()
{
	return m_nPlainLenMin;
}

int CChainWalkContext::GetPlainLenMax()
{
	return m_nPlainLenMax;
}

uint64 CChainWalkContext::GetPlainSpaceTotal()
{
	return m_nPlainSpaceTotal;
}

int CChainWalkContext::GetRainbowTableIndex()
{
	return m_nRainbowTableIndex;
}

void CChainWalkContext::Dump()
{
	printf("hash routine: %s\n", m_sHashRoutineName.c_str());
	printf("hash length: %d\n", m_nHashLen);

	printf("plain charset: ");
	int i;
	for (i = 0; i < m_nPlainCharsetLen; i++)
	{
		if (isprint(m_PlainCharset[i]))
			printf("%c", m_PlainCharset[i]);
		else
			printf("?");
	}
	printf("\n");

	printf("plain charset in hex: ");
	for (i = 0; i < m_nPlainCharsetLen; i++)
		printf("%02x ", m_PlainCharset[i]);
	printf("\n");

	printf("plain length range: %d - %d\n", m_nPlainLenMin, m_nPlainLenMax);
	printf("plain charset name: %s\n", m_sPlainCharsetName.c_str());
	//printf("plain charset content: %s\n", m_sPlainCharsetContent.c_str());
	//for (i = 0; i <= m_nPlainLenMax; i++)
	//	printf("plain space up to %d: %s\n", i, uint64tostr(m_nPlainSpaceUpToX[i]).c_str());
	printf("plain space total: %s\n", uint64tostr(m_nPlainSpaceTotal).c_str());

	printf("rainbow table index: %d\n", m_nRainbowTableIndex);
	printf("reduce offset: %s\n", uint64tostr(m_nReduceOffset).c_str());
	printf("\n");
}

void CChainWalkContext::GenerateRandomIndex()
{
	RAND_bytes((unsigned char*)&m_nIndex, 8);
	m_nIndex = m_nIndex % m_nPlainSpaceTotal;
}

void CChainWalkContext::SetIndex(uint64 nIndex)
{
	m_nIndex = nIndex;
}

void CChainWalkContext::SetHash(unsigned char* pHash)
{
	memcpy(m_Hash, pHash, m_nHashLen);
}

void CChainWalkContext::IndexToPlain()
{
	int i;
	for (i = m_nPlainLenMax - 1; i >= m_nPlainLenMin - 1; i--)
	{
		if (m_nIndex >= m_nPlainSpaceUpToX[i])
		{
			m_nPlainLen = i + 1;
			break;
		}
	}

	uint64 nIndexOfX = m_nIndex - m_nPlainSpaceUpToX[m_nPlainLen - 1];

#if !defined(_M_X64) && !defined(_M_IX86) && !defined(__i386__) && !defined(__x86_64__)
	// Slow version
	for (i = m_nPlainLen - 1; i >= 0; i--)
	{
		m_Plain[i] = m_PlainCharset[nIndexOfX % m_nPlainCharsetLen];
		nIndexOfX /= m_nPlainCharsetLen;
	}
#elif defined(_M_X64) || defined(_M_IX86) || defined(__i386__) || defined(__x86_64__)
	// Fast version
	for (i = m_nPlainLen - 1; i >= 0; i--)
	{
#ifdef _WIN32
		if (nIndexOfX < 0x100000000I64)
			break;
#else
		if (nIndexOfX < 0x100000000llu)
			break;
#endif

		m_Plain[i] = m_PlainCharset[nIndexOfX % m_nPlainCharsetLen];
		nIndexOfX /= m_nPlainCharsetLen;
	}
	unsigned int nIndexOfX32 = (unsigned int)nIndexOfX;
	for (; i >= 0; i--)
	{
		//m_Plain[i] = m_PlainCharset[nIndexOfX32 % m_nPlainCharsetLen];
		//nIndexOfX32 /= m_nPlainCharsetLen;

		unsigned int nPlainCharsetLen = m_nPlainCharsetLen;
		unsigned int nTemp;
#ifdef _WIN32
	// Заменяем ассемблер на стандартный C++
	nTemp = nIndexOfX32 % nPlainCharsetLen;
	nIndexOfX32 = nIndexOfX32 / nPlainCharsetLen;
#else
	// Оригинальный GCC inline asm
	__asm__ __volatile__ (
		"mov %2, %%eax;"
		"xor %%edx, %%edx;"
		"divl %3;"
		"mov %%eax, %0;"
		"mov %%edx, %1;"
		: "=m"(nIndexOfX32), "=m"(nTemp)
		: "m"(nIndexOfX32), "m"(nPlainCharsetLen)
		: "%eax", "%edx"
	);
#endif
		m_Plain[i] = m_PlainCharset[nTemp];
	}
#endif
}

void CChainWalkContext::PlainToHash()
{
	m_pHashRoutine(m_Plain, m_nPlainLen, m_Hash);
}

void CChainWalkContext::HashToIndex(int nPos)
{
	m_nIndex = (*(uint64*)m_Hash + m_nReduceOffset + nPos) % m_nPlainSpaceTotal;
}

uint64 CChainWalkContext::GetIndex()
{
	return m_nIndex;
}

string CChainWalkContext::GetPlain()
{
	string sRet;
	int i;
	for (i = 0; i < m_nPlainLen; i++)
	{
		char c = m_Plain[i];
		if (c >= 32 && c <= 126)
			sRet += c;
		else
			sRet += '?';
	}

	return sRet;
}

string CChainWalkContext::GetBinary()
{
	return HexToStr(m_Plain, m_nPlainLen);
}

string CChainWalkContext::GetPlainBinary()
{
	string sRet;
	sRet += GetPlain();
	int i;
	for (i = 0; i < m_nPlainLenMax - m_nPlainLen; i++)
		sRet += ' ';

	sRet += "|";

	sRet += GetBinary();
	for (i = 0; i < m_nPlainLenMax - m_nPlainLen; i++)
		sRet += "  ";

	return sRet;
}

string CChainWalkContext::GetHash()
{
	return HexToStr(m_Hash, m_nHashLen);
}

bool CChainWalkContext::CheckHash(unsigned char* pHash)
{
	if (memcmp(m_Hash, pHash, m_nHashLen) == 0)
		return true;

	return false;
}
