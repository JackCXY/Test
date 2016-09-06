#pragma(4)
#pragma once
#include <stdio.h>
#include <cstdlib>
#include <assert.h>
#include <string.h>
#include <cmath>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <stack>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <set>
#include <ctime>
#include <queue>
#include <functional>
#include <algorithm>
#include <random>
#include <list>
#include <sys/stat.h>
#include <stdint.h>
#include <direct.h>
#ifdef WIN32
#include <io.h>
#include <time.h>
#define  fseeko _fseeki64
#define stat64 _stati64
#else
#include <sys/io.h>
#include <sys/time.h>
#define _stati64 stat64
#define _fseeki64 fseeko
#define _FILE_OFFSET_BITS 64   
typedef long long __int64;
#endif
typedef unsigned int uint;

struct Rect
{
	Rect()  {}

	Rect(int a_minX, int a_minY, int a_maxX, int a_maxY)
	{
		min[0] = a_minX;
		min[1] = a_minY;

		max[0] = a_maxX;
		max[1] = a_maxY;
	}
	int min[2];
	int max[2];
};
const int hashLen = (1 << 16) - 1;
const uint BLOCKSIZE = 4096;

const int MELN = 1024 * 16;
const uint TUPLELEN = BLOCKSIZE / (sizeof(int) * 3);
const int  INITLEN = BLOCKSIZE / sizeof(int);
const int  TWOLEN = BLOCKSIZE / (sizeof(int) * 2);
const int LEFTBLOCK = BLOCKSIZE - TUPLELEN * 3 * sizeof(int);
const std::string  AIDS = "d:\\data\\database\\AIDS";
const std::string AIDS_Query = "d:\\data\\query\\AIDS";
const std::string AIDS_100 = "d:\\data\\query\\AIDS_100";
const std::string Query_400 = "d:\\data\\query\\m1200";
const std::string Pub_400 = "d:\\data\\database\\m400";
const std::string Pub_2400 = "d:\\data\\query\\m1200";
const std::string Query_4 = "d:\\data\\query\\m4";
const std::string AIDS_temp = "d:\\data\\query\\temp";
const std::string AIDS_random = "d:\\data\\query\\random";
using namespace std;
