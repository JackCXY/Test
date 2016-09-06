#ifndef _ROW_QUERY_H
#define _ROW_QUERY_H

#include "stdafx.h"
#include "matrixTuple.h"
#include "branchHash.h"
#include "graph.h"
#include "RTree.h"
//1.Q-gram store with row major lazzy mode

class rowQuery
{
private:
	branchHash *eh;
	FILE *fv;
	FILE *fe;
	FILE *fg;

	FILE *fb;
	FILE *fl;

	FILE *fbf;
	FILE *flf;

	int total;

	map<int, int*> mv;
	map<int, int*> me;

	map<int, matrixTuple*> mbf;
	map<int, matrixTuple*> mlf;

	map<int, int*> md;
	map<int, int*> ::iterator iter;
	map<int, matrixTuple*> ::iterator miter;

	RTree<int, int, 2, float> tree;

public:
	rowQuery(){}
	rowQuery(string prefix, string hp, int num)
	{
		total = num;
		eh = branchHash::getInstance(hp);

#ifdef WIN32
		if (_access((prefix + "/ROW").c_str(), 0) == -1)
		{
			cout << "the row major Matirx has not build done" << endl;
			exit(0);
		}
		if (_access((prefix + "/ROW/brow").c_str(), 0) == -1 || _access((prefix + "/ROW/lrow").c_str(), 0) == -1)
		{
			cout << "the row major Matirx has not build done" << endl;
			exit(0);
		}
#else
		if (access((prefix + "/ROW").c_str(), F_OK | R_OK) == -1 || access((prefix + "/ROW/brow").c_str(), F_OK | R_OK) == -1 || access((prefix + "/ROW/lrow").c_str(), F_OK | R_OK) == -1)
		{
			cout << "the row major Matirx has not build done" << endl;
			exit(0);
		}
#endif

		tree.Load((prefix + "/rtree.idx").c_str());
		fb = fopen((prefix + "/ROW/brow").c_str(), "rb+"); assert(fb);
		fl = fopen((prefix + "/ROW/lrow").c_str(), "rb+"); assert(fl);
		fv = fopen((prefix + "/vertex.idx").c_str(), "rb"); assert(fv);
		fe = fopen((prefix + "/edge.idx").c_str(), "rb"); assert(fe);
		fg = fopen((prefix + "/id.idx").c_str(), "rb"); assert(fg);

		fbf = fopen((prefix + "/ROW/bpage").c_str(), "rb"); assert(fbf);
		flf = fopen((prefix + "/ROW/lpage").c_str(), "rb"); assert(flf);

	}

	~rowQuery()
	{
		branchHash::instance = NULL;
		if (fv)fclose(fv);
		if (fe) fclose(fe);
		if (fb) fclose(fb);
		if (fl) fclose(fl);
		if (fg) fclose(fg);
		if (fbf) fclose(fbf);
		if (flf) fclose(flf);

		for (iter = mv.begin(); iter != mv.end(); ++iter){ if (iter->second) delete[] iter->second; }
		for (iter = me.begin(); iter != me.end(); ++iter){ if (iter->second) delete[] iter->second; }
		for (iter = md.begin(); iter != md.end(); ++iter){ if (iter->second) delete[] iter->second; }

		for (miter = mbf.begin(); miter != mbf.end(); ++miter) { if (miter->second) delete[] miter->second; }
		for (miter = mlf.begin(); miter != mlf.end(); ++miter) { if (miter->second) delete[] miter->second; }
		
	}

private:
	//out:->result
	int countingGram(map<int, int> &gv, matrixTuple *mt, int len)
	{
		int s = 0;
		map<int, int> ::iterator giter;
		for (int i = 0; i < len; i++)
		{
			giter = gv.find(mt[i].column);
			if (giter != gv.end())
				s += giter->second < mt[i].value ? giter->second : mt[i].value;
		}
		return s;
	}
	void countingGram(map<int, int> &gv, matrixTuple *cache, int graph_count, vector<int> &out)
	{
		map<int, int> ::iterator iter;
		matrixTuple *graphCache = new matrixTuple[TUPLELEN];

		int length = 0;
		int init_row = cache[0].row;

		for (int i = 0; i < graph_count; i++)
		{
			if (cache[i].row == init_row)
			{
				graphCache[length++] = cache[i];
			}
			else
			{
				out[init_row] = countingGram(gv, graphCache, length);
				init_row = cache[i].row;
				length = 0;
				graphCache[length++] = cache[i];
			}

		}
		if (length > 0) //the last cache need verify
		{
			out[init_row] = countingGram(gv, graphCache, length);
		}
		if (graphCache) delete[] graphCache;
	}
	void countingGram(map<int, int> &gv, matrixTuple *cache, int graph_count, map<int, int> &gb)
	{
		map<int, int> ::iterator iter;
		matrixTuple *graphCache = new matrixTuple[TUPLELEN];

		int length = 0;
		int init_row = cache[0].row;

		for (int i = 0; i < graph_count; i++)
		{
			if (cache[i].row == init_row)
			{
				graphCache[length++] = cache[i];
			}
			else
			{
				if ((iter = gb.find(init_row)) != gb.end())
				{
					int value = countingGram(gv, graphCache, length);
					iter->second = value;
				}
				init_row = cache[i].row;
				length = 0;
				graphCache[length++] = cache[i];
			}

		}
		if (length > 0) //the last cache need verify
		{
			if ((iter = gb.find(init_row)) != gb.end())
			{
				int value = countingGram(gv, graphCache, length);
				iter->second = value;
			}
		}
		if (graphCache) delete[] graphCache;
	}

public:

	void  row_add(FILE *&fr, FILE *&ff, map<int, matrixTuple*> &mf, vector<int> &vr, map<int, int> &gv, map<int, int> &gb, __int64 &io);
	void row_add(FILE *&fr, FILE *&ff, map<int, matrixTuple*> &mf, map<int, int> &gv, vector<int> &out, __int64 &io, int &count);
	void candidate_branch(int v, vector<int> &out, vector<int> &result, __int64 &io, int d);
	void candidate_branch(int v, map<int, int > &gb, vector<int> &result, __int64 &io, int d);
	void candidate_label(int v, int e, vector<int> &out, vector<int> &result, __int64 &io, int d);
	void candidate_label(int v, int e, map<int, int > &gb, vector<int> &result, __int64 &io, int d);

public:
	void queryGraphRtree(graph g, __int64 &io, vector<int> &result, int d);
	void queryGraph(graph g, __int64 &io, vector<int> &result, int d);

public:
	void queryGraphLB(graph g, __int64 &io, vector<int> &result, int d)
	{
		vector<int> br;
		vector<int> lr;
		map<int, int> gb;
		vector<int> out(total, 0);

		map<int, int> mb = g.weightBranchVector(eh);
		map<int, int> ml = g.weightLabelVector(eh);

		int matrixTupleTotal = 0;
		row_add(fl, flf, mlf, ml, out, io, matrixTupleTotal);
		candidate_label(g.v, g.e, out, lr, io, d);
		
		gb.clear();

		row_add(fb, fbf, mbf, lr, mb, gb, io);
		candidate_branch(g.v, gb, br, io, d);
		sort(br.begin(), br.end());
		for (int i = 0; i < br.size(); i++)
		{
			int graphID = common::readIntData(fg, br[i], md, io);
			result.push_back(graphID);
		}
	}
};
#endif