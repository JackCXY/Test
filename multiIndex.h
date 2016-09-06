#ifndef _MULTI_INDEX_H
#define _MULTI_INDEX_H
#include "singleIndex.h"

class multiIndex
{
private:
	int areaLength;
	int maxv;
	int maxe;
	int total;
	double factor;
	string prefix;
	string hp;
	branchHash *bh;
	map<string, int> fp;

public:
	multiIndex(){ areaLength = 4; prefix = "./area"; factor = 0.06;  bh = NULL; }
	~multiIndex()
	{
		this->writeInfo();
		bh->sequenceHash();
		bh = NULL;
	}
	multiIndex(string p, string hp, int l,  double f = 0.9)
	{
		this->prefix = p;
		this->areaLength = l;
		this->factor = f;
		this->hp = hp;

		bh = branchHash::getInstance(hp);
	}
public:
	void position(graph &g, int &x, int &y)
	{
		common::position(g.v, g.e, maxv, maxe, areaLength, x, y);
	}
	string enchoding(string prefix, int x, int y)
	{
		int ax, ay;
		common::position(x, y, maxv, maxe, areaLength, ax, ay);
		string path = common::inToString(prefix, ax, ay);

#ifdef WIN32
		if (_access(path.c_str(), 0) == -1)
		{
			if (_mkdir(path.c_str()) == -1)
			{
				cout << "mkdir error in path:" <<path << endl;
			}
		}
#else 
		if(access(path.c_str(), F_OK | R_OK) == -1)
		{
			if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
			{
				cout << "mkdir error in" << path << endl;
			}
		}
#endif
		return path;
	}
	void writeInfo()
	{
		ofstream fout((prefix + "/dfo"));
		fout << maxv << " " << maxe << endl;
		fout << areaLength << endl;
		fout << fp.size()<< endl;
		for (map<string, int>::iterator iter = fp.begin(); iter != fp.end(); ++iter)
			fout << iter->first << " " <<iter->second << endl;
		fout.close();
	}
public:
	void staticGraphDataBase(const char *input, int total);
	void splitGraphDataBase(const char *input, map<string, int> &fp, int total);

public:
	double build_index(const char *input, int total, __int64 &io); 
};
#endif