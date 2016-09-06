#pragma once
#ifndef _EMHASH_H
#define _EMHASH_H
#include "stdafx.h"

const int LEN = 16;
const std::string SENTRY(LEN, '$');

class emHash
{
public:
	emHash(const char *file);
	int   find(string str, __int64 &pos, string &s);
	void insert(string str);
	map<int, int> queryVector(map<string, int> &fs);
	~emHash(){ if (fh) fclose(fh); }

private:
	int BKDRHASH(const char *str);
	inline void strCollapse(string &str, int value)
	{
		stringstream ss;
		ss << value;
		str += ss.str();
	}
	inline int strPos(string str)
	{
		if (str[0] == '$') return 0;
		int i, j = 0;
		for (i = 0; i < str.size(); i++)
		{
			if (str[i] == '$')
			{
				j = i;
				break;
			}
		}
		if (j == 0) return 0;
		return j;
	}
	inline string strRead(__int64 pos, int &groupID)
	{

		fseeko(fh, pos, SEEK_SET);
		string str(LEN, '$');
		int l = fread(&str[0], sizeof(char), LEN, fh);
		l = fread(&groupID, sizeof(int), 1, fh);
		int j = strPos(str);
		if (j == 0) return SENTRY;
		return str.substr(0, j);

	}
	inline void strWrite(__int64 pos, string str, int v)
	{
		fseeko(fh, pos, SEEK_SET);
		str.resize(LEN, '$');
		int l = fwrite(str.c_str(), sizeof(char), LEN, fh);
		l = fwrite(&v, sizeof(int), 1, fh);
	}
	FILE *fh;
	int INDEX;
};

#endif
