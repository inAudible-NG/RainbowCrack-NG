/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 * Copyright 2011 Logan Watt <logan.watt@gmail.com>
 * Copyright 2011 Karl Fox <karl@lithik.com>
 *
 * This file is part of freerainbowtables.
 *
 * freerainbowtables is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * freerainbowtables is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with freerainbowtables.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Public.h"

#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#ifdef _WIN32
	#include <windows.h>
#endif

#if defined(_WIN32) && !defined(__GNUC__)
	#include <windows.h>
	#include <time.h>
	#include <io.h>

	#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
		#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
	#else
		#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
	#endif

	struct timezone
	{
		int tz_minuteswest; /* minutes W of Greenwich */
		int tz_dsttime;     /* type of dst correction */
	};

	int gettimeofday(struct timeval *tv, struct timezone *tz)
	{
		// Define a structure to receive the current Windows filetime
		FILETIME ft;

		// Initialize the present time to 0 and the timezone to UTC
		unsigned __int64 tmpres = 0;
		static int tzflag = 0;

		if (NULL != tv)
		{
			GetSystemTimeAsFileTime(&ft);

			// The GetSystemTimeAsFileTime returns the number of 100 nanosecond
			// intervals since Jan 1, 1601 in a structure. Copy the high bits to
			// the 64 bit tmpres, shift it left by 32 then or in the low 32 bits.
			tmpres |= ft.dwHighDateTime;
			tmpres <<= 32;
			tmpres |= ft.dwLowDateTime;

			// Convert to microseconds by dividing by 10
			tmpres /= 10;

			// The Unix epoch starts on Jan 1 1970.  Need to subtract the difference
			// in seconds from Jan 1 1601.
			tmpres -= DELTA_EPOCH_IN_MICROSECS;

			// Finally change microseconds to seconds and place in the seconds value.
			// The modulus picks up the microseconds.
			tv->tv_sec = (long)(tmpres / 1000000UL);
			tv->tv_usec = (long)(tmpres % 1000000UL);
		}

		if (NULL != tz)
		{
			if (!tzflag)
			{
				_tzset();
				tzflag++;
			}

			// Adjust for the timezone west of Greenwich
			tz->tz_minuteswest = _timezone / 60;
			tz->tz_dsttime = _daylight;
		}

		return 0;
	}

#elif defined(__APPLE__) || \
	((defined(__unix__) || defined(unix)) && !defined(USG))

	#include <sys/param.h>

	#if defined(BSD)
		#include <sys/sysctl.h>
	#elif defined(__linux__) || defined(__sun__)
		#include <sys/sysinfo.h>
	#else
		#error Unsupported Operating System
	#endif
#endif

//////////////////////////////////////////////////////////////////////

unsigned int GetFileLen(FILE* file)
{
	unsigned int pos = ftell(file);
	fseek(file, 0, SEEK_END);
	unsigned int len = ftell(file);
	fseek(file, pos, SEEK_SET);

	return len;
}

string TrimString(string s)
{
	while (s.size() > 0)
	{
		if (s[0] == ' ' || s[0] == '\t')
			s = s.substr(1);
		else
			break;
	}

	while (s.size() > 0)
	{
		if (s[s.size() - 1] == ' ' || s[s.size() - 1] == '\t')
			s = s.substr(0, s.size() - 1);
		else
			break;
	}

	return s;
}

bool ReadLinesFromFile(string sPathName, vector<string>& vLine)
{
	vLine.clear();

	FILE* file = fopen(sPathName.c_str(), "rb");
	if (file != NULL)
	{
		unsigned int len = GetFileLen(file);
		char* data = new char[len + 1];
		fread(data, 1, len, file);
		data[len] = '\0';
		string content = data;
		content += "\n";
		delete data;

		int i;
		for (i = 0; i < content.size(); i++)
		{
			if (content[i] == '\r')
				content[i] = '\n';
		}

		int n;
		while ((n = content.find("\n", 0)) != -1)
		{
			string line = content.substr(0, n);
			line = TrimString(line);
			if (line != "")
				vLine.push_back(line);
			content = content.substr(n + 1);
		}

		fclose(file);
	}
	else
		return false;

	return true;
}

bool SeperateString(string s, string sSeperator, vector<string>& vPart)
{
	vPart.clear();

	int i;
	for (i = 0; i < sSeperator.size(); i++)
	{
		int n = s.find(sSeperator[i]);
		if (n != -1)
		{
			vPart.push_back(s.substr(0, n));
			s = s.substr(n + 1);
		}
		else
			return false;
	}
	vPart.push_back(s);

	return true;
}

string uint64tostr(uint64 n)
{
	char str[32];

#ifdef _WIN32
	sprintf(str, "%I64u", n);
#else
	sprintf(str, "%lu", n);
#endif

	return str;
}

string uint64tohexstr(uint64 n)
{
	char str[32];

#ifdef _WIN32
	sprintf(str, "%016I64x", n);
#else
	sprintf(str, "%016lx", n);
#endif

	return str;
}

string HexToStr(const unsigned char* pData, int nLen)
{
	string sRet;
	int i;
	for (i = 0; i < nLen; i++)
	{
		char szByte[3];
		sprintf(szByte, "%02x", pData[i]);
		sRet += szByte;
	}

	return sRet;
}

unsigned long GetAvailPhysMemorySize()
{
#ifdef _WIN32
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);
	return ms.dwAvailPhys;
#elif defined(BSD)
	int mib[2] = { CTL_HW, HW_PHYSMEM };
	uint64_t physMem;
	//XXX warning size_t isn't portable
	size_t len;
	len = sizeof(physMem);
	sysctl(mib, 2, &physMem, &len, NULL, 0);
	return physMem;
#elif defined(__linux__)
	FILE *procfd = NULL;

	if ( (procfd = fopen("/proc/meminfo", "r")) != NULL )
	{
		char result[256]={0};
		char *tmp = NULL;
		unsigned int cachedram = 0, freeram = 0, bufferram = 0;
		uint64_t tempram = 0;

		while( fgets(result,sizeof(char)*256,procfd) != NULL )
		{
			tmp = strtok(result, " ");
			if( (strncmp(tmp,"MemFree:" , 8)) == 0 )
			{
				tmp = strtok(NULL, " ");
				freeram = atoi(tmp);
			}
			else if( (strncmp(tmp, "Buffers:", 8)) == 0 )
			{
				tmp = strtok(NULL, " ");
				bufferram = atoi(tmp);
			}
			else if( (strncmp(tmp, "Cached:", 7)) == 0 )
			{
				tmp = strtok(NULL, " ");
				cachedram = atoi(tmp);
				// in 2.6 and 3.0 kernels the order is maintained and this is the
				// last value to read.  Break and don't read more lines
				break;
			}
		}

		fclose(procfd);

		tempram = (uint64_t)(freeram + bufferram + cachedram) * 1024;

		if ( sizeof(long) == 4 )
		{
			// ensure that we don't return more than 2^31-1 on 32-bit platforms
			if ( tempram > 0x7FFFFFFFLLU )
				return (unsigned long) 0x7FFFFFFFLLU;
			else
				return (unsigned long) tempram;
		}

		return tempram;
	}

	struct sysinfo info;
	sysinfo(&info);
	return ( info.freeram + info.bufferram ) * (unsigned long) info.mem_unit;
#elif defined(__sun__)
	return ((unsigned long)sysconf(_SC_AVPHYS_PAGES) * (unsigned long)sysconf(_SC_PAGESIZE));
#else
	return 0;
	#error Unsupported Operating System
#endif
}

void ParseHash(string sHash, unsigned char* pHash, int& nHashLen)
{
	int i;
	for (i = 0; i < sHash.size() / 2; i++)
	{
		string sSub = sHash.substr(i * 2, 2);
		int nValue;
		sscanf(sSub.c_str(), "%02x", &nValue);
		pHash[i] = (unsigned char)nValue;
	}

	nHashLen = sHash.size() / 2;
}

void Logo()
{
	printf("RainbowCrack 1.2 - Making a Faster Cryptanalytic Time-Memory Trade-Off\n");
	printf("by Zhu Shuanglei <shuanglei@hotmail.com>\n");
	printf("http://www.antsight.com/zsl/rainbowcrack/\n\n");
}
