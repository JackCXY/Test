#ifndef _INVERT_BUILD_H
#define _INVERT_BUILD_H

//construct the invert index
#include "stdafx.h"
#include "loseTree.h"
#include "twoTuple.h"

class invertBuild
{
public:
	
	string prefix;
	FILE *fb;
	FILE *fl;

	map<int, twoTuple> columnIndex;
	int total;

public:
	invertBuild()
	{
		fb = fl = NULL;
	}
	invertBuild(string prefix, int num)
	{
	
		this->prefix = prefix;
#ifdef WIN32
		if (_access((prefix + "/brow").c_str(), 0) == -1 || _access((prefix + "/lrow").c_str(), 0) == -1)
		{
			cout << "please build the row major matrix first " << endl;
			exit(0);
		}
		if (_access((prefix + "/invert").c_str(), 0) == -1) // need to build 
		{ 
			if (_mkdir((prefix + "/invert").c_str()) == -1)
				cout << "mkdir dir error" << endl;
		}
#else
		if (access((prefix + "/brow").c_str(), F_OK | R_OK) == -1 || access((prefix + "/lrow").c_str(), F_OK | R_OK) == -1)
		{
			cout << "please build the row major matrix first " << endl;
			exit(0);
		}
		if (access((prefix + "/invert").c_str(), F_OK | R_OK) == -1) // need to build 
		{
			if (mkdir((prefix + "/invert").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
				cout << "mkdir dir error" << endl;
		}
#endif
		fl = fopen((prefix + "/invert/label.ivt").c_str(), "wb+"); assert(fl);
		fb = fopen((prefix + "/invert/branch.ivt").c_str(), "wb+"); assert(fb);
		total = num;
	}
	~invertBuild()
	{
		this->writeInfo();
		if (fb) fclose(fb);
		if (fl) fclose(fl);
	}
	void writeInfo()
	{
		ofstream fout(prefix + "/invert/dfo");
		fout << columnIndex.size() << endl;
		for (map<int, twoTuple> ::iterator iter = columnIndex.begin(); iter != columnIndex.end(); ++iter)
		{
			fout << iter->first << " " << iter->second.row << " " << iter->second.value << endl;
		}
		fout.close();
	}

public:
	void externSortColumn(string row, string out)
	{
		__int64 io = 0;
		bool f = true;
		loseTree *lt = new loseTree();
		lt->copyData(row.c_str(), "./tmp.dat");
		lt->externSort("./tmp.dat", out.c_str(), f, io);
		std::remove("./tmp.dat");
		if (lt) delete lt;
		
 	}
	void buildInvert(string in, FILE *&fw)
	{
		FILE *fr = fopen(in.c_str(), "rb"); assert(fr);
		rewind(fw);

		matrixTuple *cache = new matrixTuple[TUPLELEN];
		twoTuple *cacheWrite = new twoTuple[TUPLELEN]; // 暂存由此产生的倒排索引数据

		int init_column = -1;
		int pos = 0, wpos = 0; //(1 << 31) * 12
		matrixTuple mt;

		while (!feof(fr))
		{
			int len = fread(cache, sizeof(matrixTuple), TUPLELEN, fr);
			for (int i = 0; i < len; i++)
			{
				mt = cache[i];
				cacheWrite[i].row = mt.row;
				cacheWrite[i].value = mt.value;
				if (mt.column > init_column)
				{
					int len = pos - wpos;
					if (len > 0)
					{
						columnIndex.insert(pair<int, twoTuple>(init_column, twoTuple(wpos, len)));
					}
					wpos = pos;
					init_column = mt.column;
				}
				pos++;
			}
			fwrite(cacheWrite, sizeof(twoTuple), len, fw);
		}
		columnIndex.insert(pair<int, twoTuple>(init_column, twoTuple(wpos, pos - wpos)));
		if (cache) delete[] cache;
		if (cacheWrite) delete[] cacheWrite;
		if (fr) fclose(fr);
	}
	void buildInvertIndex()
	{
		string column_Branch = prefix + "/invert/branch.clm";
		this->externSortColumn(prefix + "/brow", column_Branch);
		
		string column_Label = prefix + "/invert/label.clm"; 
		this->externSortColumn(prefix + "/lrow", column_Label);
		this->buildInvert(column_Branch, fb);
		this->buildInvert(column_Label, fl);
	}
};
#endif