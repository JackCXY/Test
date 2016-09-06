#ifndef _SINGLE_QUERY_H
#define _SINGLE_QUERY_H

#include "stdafx.h"
#include "twoTuple.h"
#include "matrixTuple.h"
#include "branchHash.h"
#include "graph.h"
#include "loseTree.h"
#include "common.h"

class singleQuery
{
	//the area information
private:

	string prefix;
	
	FILE *fv;
	FILE *fe;
	FILE *fg;
	
	FILE *bd;
	FILE *ld;
	FILE *bs;
	FILE *ls;

	FILE *bp;
	FILE *lp;

	int total;
	branchHash *eh;

	map<int, int *> mv; //vertex inforeamtion
	map<int, int *> me; //edge information
	map<int, int *> mg; //gid information

	map<int, matrixTuple *> mb; //bpage information
	map<int, matrixTuple *> ml; // lpage information

	map<int, twoTuple> scl;
	map<int, twoTuple> scb;

public:
	singleQuery(string p, string hp,  int total)
	{
		this->prefix = p;
		this->total = total;
#ifdef WIN32
		if (_access((prefix + "/sparseDense/").c_str(), 0) == -1 || _access((prefix + "/sparseDense/bd").c_str(), 0) == -1)
		{
			cout << "build the matrix first " << endl;
			exit(0);
		}
#else
		if (access((prefix + "/sparseDense/").c_str(), F_OK | R_OK) == -1 || access((prefix + "/sparseDense/bd").c_str(), F_OK | R_OK) == -1)
		{
			cout << "build the matrix first " << endl;
			exit(0);
		}
#endif
		eh = branchHash::getInstance(hp);
		fv = fopen((prefix + "/vertex.idx").c_str(), "rb"); assert(fv);
		fe = fopen((prefix + "/edge.idx").c_str(), "rb"); assert(fe);
		fg = fopen((prefix + "/id.idx").c_str(), "rb"); assert(fg);

		bd = fopen((prefix + "/sparseDense/bd").c_str(), "rb+"); assert(bd);
		ld = fopen((prefix + "/sparseDense/ld").c_str(), "rb+"); assert(ld);
		bs = fopen((prefix + "/sparseDense/bs").c_str(), "rb+"); assert(bs);
		ls = fopen((prefix + "/sparseDense/ls").c_str(), "rb+");  assert(ls);
	
		bp = fopen((prefix + "/sparseDense/bpage").c_str(), "rb+"); assert(bp);
		lp = fopen((prefix + "/sparseDense/lpage").c_str(), "rb+"); assert(lp);

		ifstream fin(prefix + "/sparseDense/invertdfo");
		int tmp, column, begin, len;
		fin >> tmp; 
		for (int i = 0; i < tmp; i++)
		{
			fin >> column >> begin >> len;
			scl.insert(pair<int, twoTuple>(column, twoTuple(begin, len)));
		}
		fin >> tmp;
		for (int i = 0; i < tmp; i++)
		{
			fin >> column >> begin >> len;
			scb.insert(pair<int, twoTuple>(column, twoTuple(begin, len)));
		}
		fin.close();
	}

	~singleQuery()
	{
		eh = NULL;
		if (fv) fclose(fv);
		if (fe) fclose(fe);
		if (fg) fclose(fg);

		if (bd) fclose(bd);
		if (ld) fclose(ld);
		if (bs) fclose(bs);
		if (ls) fclose(ls);

		if (bp) fclose(bp);
		if (lp) fclose(lp);

		map<int, int*> ::iterator iter;
		for (iter = mv.begin(); iter != mv.end(); ++iter){ if (iter->second) delete[] iter->second; }
		for (iter = me.begin(); iter != me.end(); ++iter){ if (iter->second) delete[] iter->second; }
		for (iter = mg.begin(); iter != mg.end(); ++iter){ if (iter->second) delete[] iter->second; }
		
		map<int, matrixTuple *> ::iterator miter;
		for (miter = mb.begin(); miter != mb.end(); ++miter){ if (miter->second) delete[] miter->second; }
		for (miter = ml.begin(); miter != ml.end(); ++miter){ if (miter->second) delete[] miter->second; }
	
		map<int, twoTuple>().swap(scl);
		map<int, twoTuple>().swap(scb);
	}
public:

	void columnDenseAdd(FILE *&fr, map<int, int> &gv, map<int, twoTuple> &mcl,  vector<int> &out, __int64 &io);
	void columnDenFilter(int v, map<int, int> &gv, map<int, twoTuple> &mcl, vector<int> &out, __int64 &io, int threold);
	void columnDenFilter(int v, int e, map<int, int> &gv, map<int, twoTuple> &mcl, vector<int> &out, __int64 &io, int threold);

public:
	int countingGram(map<int, int> &gv, matrixTuple *mt, int len);
	void countingGram(map<int, int> &gv, matrixTuple *cache, int graph_count, map<int, int> &gb);
	void qGramFilter(int v, int e, map<int, int> &gb, vector<int> &result, __int64 &io, int d);
	void qGramFilter(int v, map<int, int> &gb, vector<int> &result, __int64 &io, int d);
	void  row_add(FILE *&fr, FILE *&ff, map<int, matrixTuple*> &mf, vector<int> &vr, map<int, int> &gv, map<int, int> &gb, __int64 &io);
	void  row_add(FILE *&fr, FILE *&ff, map<int, matrixTuple*> &mf, vector<int> &out, vector<int> &vr, map<int, int> &gv, map<int, int> &gb, __int64 &io);
public:
	void queryGraph(graph g, __int64 &io, int d, vector<int> &vr);

};

#endif