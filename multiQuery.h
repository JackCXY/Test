#ifndef _MULTI_QUERY_H
#define _MULTI_QUERY_H
#include "singleQuery.h"

class multiQuery
{
private:

	int maxv;
	int maxe;
	int areaLength;
	string prefix;
	string hp;
	branchHash *bh;
	map<string, int> fp;

public:
	multiQuery(){}
	~multiQuery(){ bh = NULL; }
	multiQuery(string prefix, string hp)
	{
		this->prefix = prefix;
		this->hp = hp;
		bh = branchHash::getInstance(hp);
#ifdef WIN32
		if (_access((prefix + "/dfo").c_str(), 0) == -1)
		{
			cout << "please build index first" << endl;
			exit(0);
		}
#else
		if (access((prefix + "/dfo").c_str(), F_OK | R_OK) == -1)
		{
			cout << "please build index first" << endl;
			exit(0);
		}
#endif
		ifstream fin(prefix + "/dfo");
		fin >> maxv >> maxe;
		fin >> areaLength;
		int size, total;
		string str;
		fin >> size;
		for (int i = 0; i < size; i++)
		{
			fin >> str >> total;
			fp.insert(pair<string, int>(str, total));
		}
		fin.close();
	}

public:
	void unionArea(int v, int e, int &lx, int &rx, int &ly, int &ry, int threshold);
	void unionArea(int v, int e, int threshold, vector<string> &vs);

public:
	void queryGraph(graph g, __int64 &io, int threshold, vector<int> &vr);
	double indexSize()
	{
		double size = 0;
		for (map<string, int> ::iterator iter = fp.begin(); iter != fp.end(); ++iter)
		{
			string path = iter->first;
			size += common::indexSize(path);
		}
		return size; 
	}

};
#endif