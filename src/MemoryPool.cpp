/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#include "MemoryPool.h"
#include "Public.h"

CMemoryPool::CMemoryPool()
{
	m_pMem = NULL;
	m_nMemSize = 0;

	unsigned int nAvailPhys = GetAvailPhysMemorySize();
	if (nAvailPhys < 16 * 1024 * 1024)
		m_nMemMax = nAvailPhys / 2;					// Leave some memory for CChainWalkSet
	else
		m_nMemMax = nAvailPhys - 8 * 1024 * 1024;	// Leave some memory for CChainWalkSet
}

CMemoryPool::~CMemoryPool()
{
	if (m_pMem != NULL)
	{
		delete m_pMem;
		m_pMem = NULL;
		m_nMemSize = 0;
	}
}

unsigned char* CMemoryPool::Allocate(unsigned int nFileLen, unsigned int& nAllocatedSize)
{
	if (nFileLen <= m_nMemSize)
	{
		nAllocatedSize = nFileLen;
		return m_pMem;
	}

	unsigned int nTargetSize;
	if (nFileLen < m_nMemMax)
		nTargetSize = nFileLen;
	else
		nTargetSize = m_nMemMax;

	// Free existing memory
	if (m_pMem != NULL)
	{
		delete m_pMem;
		m_pMem = NULL;
		m_nMemSize = 0;
	}

	// Allocate new memory
	//printf("allocating %u bytes memory\n", nTargetSize);
	m_pMem = new unsigned char[nTargetSize];
	if (m_pMem != NULL)
	{
		m_nMemSize = nTargetSize;
		nAllocatedSize = nTargetSize;
		return m_pMem;
	}
	else
	{
		nAllocatedSize = 0;
		return NULL;
	}
}
