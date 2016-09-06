#ifndef _COLUMNSPMV_H
#define _COLUMNSPMV_H

#include "stdafx.h"
#include "matrixTuple.h"
#include "twoTuple.h"
#include "emHash.h"
#include "graph.h"
#include "twoLoseTree.h"

class columnSpmv
{
private:

	emHash *eh;
	FILE *fv;
	FILE *fb;
	FILE *fl;
	FILE *fe;
	map<int, int*> mv;
	map<int, int*> me;
	int total;
	int threold;

public:
	columnSpmv()
	{
		eh = emHash::getInstance("./emHash/eh.idx");
		fv = fopen("./single/vertex", "rb"); assert(fv);
		fe = fopen("./single/edge", "rb"); assert(fe);
		fb = fopen("./single/branch.column", "rb"); assert(fb);
		fl = fopen("./single/label.column", "rb"); assert(fl);
	}
	columnSpmv(string prefix, int num, int d)
	{

		eh = emHash::getInstance("./emHash/eh.idx");
		fv = fopen((prefix + "/vertex").c_str(), "rb"); assert(fv);
		fe = fopen((prefix + "/edge").c_str(), "rb"); assert(fe);
		fb = fopen((prefix + "/branch.column").c_str(), "rb"); assert(fb);
		fl = fopen((prefix + "/label.column").c_str(), "rb"); assert(fl);

		total = num;
		threold = d;
	}
	columnSpmv(const char *idx, const char *vertex, const char *edge, const char *brow, const char *lrow, int num, int d);
	void build_column_index(FILE *&fr, map<int, twoTuple> &columnIndex);
	void column_spmv(FILE *&fr, map<int, twoTuple> &columnIndex, map<int, int> gv, vector<int> &out, __int64 &io, int &matrixtotal); //return scan IO
	void part_sum(const char *in, vector<int> &out, __int64 &io); //extern sort IO
	__int64 row_add(const char *in, const char *out, __int64 &io);  //write into file
	__int64 row_add(vector<string> in, vector<int> &out, __int64 &io); //merge IO 

	void candidate_branch(int v, vector<int> &out, vector<int> &result);
	void candidate_label(int v, int e, vector<int> &out, vector<int> &result);
	void queryGraph(graph g, __int64 &io, vector<int> &result);

	~columnSpmv()
	{
		if (eh) delete eh;
		if (fv)fclose(fv);
		if (fe) fclose(fe);
		if (fb) fclose(fb);
		if (fl) fclose(fl);
		map<int, int*> ::iterator iter;
		iter = mv.begin();
		if (iter != mv.end() && iter->second) delete[] iter->second;
		iter = me.begin();
		if (iter != me.end() && iter->second) delete[] iter->second;
	}
	int  readIntData(FILE *&fr, int pos, map<int, int *> &mt)
	{
		int offset = pos / INITLEN;
		map<int, int *> ::iterator iter;
		iter = mt.find(offset);
		if (iter == mt.end())
		{
			iter = mt.begin();
			if (iter != mt.end() && iter->second) delete[] iter->second;
			{
				map<int, int*> tmp;
				tmp.swap(mt);
			}
			_fseeki64(fr, offset * BLOCKSIZE, SEEK_SET);
			int *cache = new int[INITLEN];
			fread(cache, sizeof(int), INITLEN, fr);
			mt.insert(pair<int, int* >(offset, cache));
			return cache[pos % INITLEN];
		}
		else
			return iter->second[pos % INITLEN];
	}
	map<int, int> weightBranchVector(graph g)
	{
		map<string, int> wbv = g.branchGramSet();
		map<int, int> qv = eh->queryVector(wbv);
		return qv;
	}
	map<int, int> weightLabelVector(graph g)
	{
		map<string, int> vl = g.vertexLabel();
		map<string, int> el = g.edgeLabel();
		map<int, int> vlw = eh->queryVector(vl);
		map<int, int> elw = eh->queryVector(el);

		map<int, int>::iterator iter;
		map<int, int> vew;
		for (iter = vlw.begin(); iter != vlw.end(); ++iter)
			vew.insert(pair<int, int>(iter->first, iter->second));
		for (iter = elw.begin(); iter != elw.end(); ++iter)
			vew.insert(pair<int, int>(iter->first, iter->second));
		return vew;
	}
	__int64 FileSize(const char *in, int size)
	{
		struct _stati64 info;
		_stati64(in, &info);
		__int64 totalsize = info.st_size / size;
		return totalsize;
	}
	string inToString(int n)
	{
		stringstream ss;
		ss << n;
		return ss.str();
	}

};

#endif