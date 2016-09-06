#ifndef _INVERT_QUERY_H
#define _INVERT_QUERY_H
#include "stdafx.h"
#include "twoTuple.h"
#include "graph.h"

class invertQuery
{
private:
	branchHash *eh;
	FILE *fv;
	FILE *fe;
	FILE *fg;

	FILE *fb; 
	FILE *fl;
	
	map<int, twoTuple> columnIndex;
	map<int, int*> mv;
	map<int, int*> me;
	map<int, int*> md;

	int total;

public:
	invertQuery(){ eh = NULL; fb = fl = fe = fv = fg = NULL; }
	invertQuery(string prefix, string hp,  int num)
	{
		total = num;
		eh = branchHash::getInstance(hp);
#ifdef WIN32
		if (_access((prefix + "/invert").c_str(), 0) == -1)
		{
			cout << "the invert index has not build done" << endl;
			exit(0);
		}
		if (_access((prefix + "/invert/label.ivt").c_str(), 0) == -1 || _access((prefix + "/invert/branch.ivt").c_str(), 0) == -1)
		{
			cout << "the invert Matirx has not build done" << endl;
			exit(0);
		}
#else
		if (access((prefix + "/invert").c_str(), F_OK | R_OK) == -1 || access((prefix + "/invert/label.ivt").c_str(), F_OK | R_OK) == -1 || access((prefix + "/invert/branch.ivt").c_str(), F_OK | R_OK) == -1)
		{
			cout << "the invert index has not build done" << endl;
			exit(0);
		}
#endif
		fl = fopen((prefix + "/invert/label.ivt").c_str(), "rb+"); assert(fl);
		fb = fopen((prefix + "/invert/branch.ivt").c_str(), "rb+"); assert(fb);
		
		fv = fopen((prefix + "/vertex.idx").c_str(), "rb"); assert(fv);
		fe = fopen((prefix + "/edge.idx").c_str(), "rb"); assert(fe);
		fg = fopen((prefix + "/id.idx").c_str(), "rb"); assert(fg);

		ifstream fin(prefix + "/invert/dfo");
		int tmp, column, begin, len;
		fin >> tmp;
		for (int i = 0; i < tmp; i++)
		{
			fin >> column >> begin >> len;
			columnIndex.insert(pair<int, twoTuple>(column, twoTuple(begin, len)));
		}
		fin.close();
	}
	~invertQuery()
	{
		if (eh) delete eh;
		if (fv)fclose(fv);
		if (fe) fclose(fe);
		if (fb) fclose(fb);
		if (fl) fclose(fl);
		if (fg) fclose(fg);

		map<int, int*> ::iterator iter;
		for (iter = mv.begin(); iter != mv.end(); ++iter){ if (iter->second) delete[] iter->second; }
		for (iter = me.begin(); iter != me.end(); ++iter){ if (iter->second) delete[] iter->second; }
		for (iter = md.begin(); iter != md.end(); ++iter){ if (iter->second) delete[] iter->second; }
		
	}

public:
	void columnAdd(FILE *&fr, map<int, int> gv, vector<int> &out, __int64 &io, int &matrixtotal);
	void columnAdd(vector<string> in, vector<int> &cacheWrite, __int64 &io);
	void candidate_branch(int v, vector<int> &out, vector<int> &result, __int64 &io, int threold);
	void candidate_label(int v, int e, vector<int> &out, vector<int> &result, __int64 &io, int threold);
	void queryGraph(graph g, __int64 &io, int d, vector<int> &result);
};
#endif