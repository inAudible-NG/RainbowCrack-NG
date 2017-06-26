/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#ifndef _MEMORYPOOL_H
#define _MEMORYPOOL_H

class CMemoryPool
{
public:
	CMemoryPool();
	virtual ~CMemoryPool();

private:
	unsigned char* m_pMem;
	unsigned int m_nMemSize;

	unsigned int m_nMemMax;

public:
	unsigned char* Allocate(unsigned int nFileLen, unsigned int& nAllocatedSize);
};

#endif
