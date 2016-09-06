#pragma once
#ifndef _LOSETREE_H
#define _LOSETREE_H

#include "stdafx.h"
#include "matrixTuple.h"

#define MINTUPLE  matrixTuple(-1, -1, 0)
#define MAXTUPLE  matrixTuple((1 << 31) - 1, (1 << 31) - 1, (1 << 31) - 1) 

extern bool cmpRow(const matrixTuple &f, const matrixTuple &s);
extern bool cmpColumn(const matrixTuple &f, const matrixTuple &s);

struct mergeData
{
	matrixTuple *mData;
	int len;
	int pos;
	mergeData(){ len = pos = 0; mData = NULL; }
};
class loseTree
{
private:
	int K; //the merge length 
	__int64 totalsize; // the file length
	bool f; // rowMajor or columnMajor
	FILE *fr; //the file need to operation

public:
	inline string inTostring(int n)
	{
		stringstream ss;
		ss << n;
		return ss.str();
	}
	loseTree(){ K = MELN; fr = NULL; f = true; }
	loseTree(const char *file, __int64 n, bool f = true);
	~loseTree(){ if (fr) fclose(fr); }
	void adjustTree(matrixTuple *data, int *loseData, int i, int k);
	void mergeRun(__int64 begin, __int64 end, __int64 runs, FILE *&fw, __int64 f, __int64 &io);
	void kMerge(int level, __int64 runs, __int64 &io);
	void copyData(FILE *&fin, const char *out)
	{
		fflush(fin);
		rewind(fin);
		FILE *fout = fopen(out, "wb");
		matrixTuple *cache = new matrixTuple[TUPLELEN];
		int len;
		while (!feof(fin))
		{
			if (len = fread(cache, sizeof(matrixTuple), TUPLELEN, fin))
				fwrite(cache, sizeof(matrixTuple), len, fout);
		}
		if (fout) fclose(fout);
	}
	void copyData(const char *in, const char *out)
	{
		FILE *fin = fopen(in, "rb");
		FILE *fout = fopen(out, "wb");
		matrixTuple *cache = new matrixTuple[TUPLELEN];
		int len;
		while (!feof(fin))
		{
			if (len = fread(cache, sizeof(matrixTuple), TUPLELEN, fin))
				fwrite(cache, sizeof(matrixTuple), len, fout);
		}
		if (fin) fclose(fin);
		if (fclose) fclose(fout);
		if (cache) delete[] cache;
	}
	//false: row Major
	void externSort(const char *row, const char *out, bool f, __int64 &io)
	{
#ifdef WIN32
		if (_access("./sort/", 0) == -1)
		{
			if (_mkdir("./sort/") == -1)
			{
				cout << "mkdir the sort folder error" << endl;
			}
		}
#else
	if (access("./sort/", R_OK | W_OK) == -1) // in the linux
	{
		if (mkdir("./sort/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH == -1)
			cout << "mkdir sort folder error" << endl;
	}
#endif
		
		struct _stati64 info;
		_stati64(row, &info);
		totalsize = info.st_size / sizeof(matrixTuple);
		if (fr) fclose(fr);

		fr = fopen(row, "rb+"); assert(fr);
		this->f = f;
		int level = 0;
		string path = externSort(io, level);
		if (fr) fclose(fr); fr = NULL;

		if (path == "")
			this->copyData(row, out);
		else
			this->copyData(path.c_str(), out);
		assert(level >= 0);

		for (int i = 0; i < level; i++)
		{
			string temp = "./sort/" + inTostring(i) + ".idx";
			std::remove(temp.c_str());
		}

	}
	string externSort(__int64 &io, int &level);
	string externSort();
	void test();
};
#endif
