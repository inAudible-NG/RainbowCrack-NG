/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#ifndef _HASHSET_H
#define _HASHSET_H

#include "Public.h"

class CHashSet
{
public:
	CHashSet();
	virtual ~CHashSet();

private:
	vector<string> m_vHash;
	vector<bool>   m_vFound;
	vector<string> m_vPlain;
	vector<string> m_vBinary;

public:
	void AddHash(string sHash);		// lowercase, len % 2 == 0, MIN_HASH_LEN * 2 <= len <= MAX_HASH_LEN * 2
	bool AnyhashLeft();
	bool AnyHashLeftWithLen(int nLen);
	void GetLeftHashWithLen(vector<string>& vHash, int nLen);

	void SetPlain(string sHash, string sPlain, string sBinary);
	bool GetPlain(string sHash, string& sPlain, string& sBinary);

	int GetStatHashFound();
	int GetStatHashTotal();
};

#endif
