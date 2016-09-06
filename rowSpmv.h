#ifndef _ROWSPMV_H
#define _ROWSPMV_H

#include "stdafx.h"
#include "matrixTuple.h"
#include "emHash.h"
#include "graph.h"

class rowSpmv
{
private:
	emHash *eh;
	FILE *fv;
	FILE *fb;
	FILE *fl;
	FILE *fe;
	FILE *fg;
	map<int, int*> mv;
	map<int, int*> me;
	map<int, int*> md;
	int total;
	int threold;

public:
	rowSpmv()
	{
	}
	rowSpmv(string prefix, int num, int d)
	{
		eh = new emHash((prefix + "/eh.idx").c_str());
		fv = fopen((prefix + "/vertex.idx").c_str(), "rb"); assert(fv);
		fe = fopen((prefix + "/edge.idx").c_str(), "rb"); assert(fe);
		fb = fopen((prefix + "/brow").c_str(), "rb"); assert(fb);
		fl = fopen((prefix + "/lrow").c_str(), "rb"); assert(fl);
		fg = fopen((prefix + "/id.idx").c_str(), "rb"); assert(fg);
		
		total = num;
		threold = d;
	}
	
	~rowSpmv(){

		if (eh) delete eh;

		if (fv)fclose(fv);
		if (fe) fclose(fe);
		if (fb) fclose(fb);
		if (fl) fclose(fl);
		if (fg) fclose(fg);

		map<int, int*> ::iterator iter;
		iter = mv.begin();
		if (iter != mv.end() && iter->second) delete[] iter->second;
		iter = me.begin();
		if (iter != me.end() && iter->second) delete[] iter->second;
		iter = md.begin();
		if (iter != md.end() && iter->second) delete[] iter->second;
	}

public:
	void row_add(FILE *&fr, map<int, int> &gv, vector<int> &out, __int64 &io, int &count);
	void candidate_branch(int v, vector<int> &out, vector<int> &result, __int64 &io);
	void candidate_label(int v, int e, vector<int> &out, vector<int> &result, __int64 &io);
	void queryGraph(graph g, __int64 &io, vector<int> &result);

};
#endif