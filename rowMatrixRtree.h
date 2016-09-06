#ifndef _ROW_MATRIX_RTREE_H
#define _ROW_MATRIX_RTREE_H
#include "stdafx.h"
#include "branchHash.h"
#include "graph.h"
#include "matrixTuple.h"
#include "RTree.h"

class rowMatrixRtree
{
public:
	branchHash *bh;
	RTree<int, int, 2, float> tree;

private:
	vector<int> id;//<gline, graphID>
	vector<int> vertex;
	vector<int> edge;
	string prefix;

	FILE *fl;
	FILE *fb;

public:
	rowMatrixRtree(string prefix, string hp)
	{
		this->prefix = prefix;
		bh = branchHash::getInstance(hp);

		fb = fopen((prefix + "/brow").c_str(), "wb+"); assert(fb);
		fl = fopen((prefix + "/lrow").c_str(), "wb+"); assert(fl);
	}
	rowMatrixRtree()
	{
		
		bh = NULL;
		fl = fb = NULL;
	}
	~rowMatrixRtree()
	{
		bh->sequenceHash();
		this->writeInfoRowMatrix();
		if (fl) fclose(fl);
		if (fb) fclose(fb);
		tree.Save((prefix + "/rtree.idx").c_str());
	}

private:
	void insert_Tuple(FILE *&fw, map<int, int>& qv, matrixTuple *cache, int &pos, int line, __int64 &io)
	{
		map<int, int> ::iterator iter;
		matrixTuple mt;
		for (iter = qv.begin(); iter != qv.end(); iter++)
		{
			mt.row = line; mt.column = iter->first; mt.value = iter->second;
			if (pos == TUPLELEN)
			{
				fwrite(cache, sizeof(matrixTuple), TUPLELEN, fw); io++; 
				pos = 0;
			}
			cache[pos++] = mt;
		}
	}

	void insert_Graph_branch(graph g, matrixTuple *mt, int &pos, int line, __int64 &io)
	{
		map<int, int> qv = g.weightBranchVector(bh);
		insert_Tuple(fb, qv, mt, pos, line, io);
	}
	void insert_Graph_label(graph g, matrixTuple *mt, int &pos, int line, __int64 &io)
	{
		map<int, int> qv = g.weightLabelVector(bh);
		insert_Tuple(fl, qv, mt, pos, line, io);
	}
	void insert_graph(graph g, matrixTuple *cacheBranch, int &branchPos, matrixTuple *cacheLabel, int &labelPos, int &line, __int64 &io)
	{
		vertex.push_back(g.v);
		edge.push_back(g.e);
		id.push_back(g.graph_id);

		insert_Graph_branch(g, cacheBranch, branchPos, line, io);
		insert_Graph_label(g, cacheLabel, labelPos, line, io);

		line++;
	}
	void writeInfoRowMatrix()
	{
		FILE *fv = fopen((prefix + "/vertex.idx").c_str(), "wb+"); assert(fv);
		for (int i = 0; i < vertex.size(); i++)
		{
			int r = vertex[i];
			fwrite(&r, sizeof(int), 1, fv);
		}
		fclose(fv);
		FILE *fe = fopen((prefix + "/edge.idx").c_str(), "wb+"); assert(fe);
		for (int i = 0; i < edge.size(); i++)
		{
			int r = edge[i];
			fwrite(&r, sizeof(int), 1, fe);
		}
		fclose(fe);
		FILE *fg = fopen((prefix + "/id.idx").c_str(), "wb+"); assert(fg);
		for (int i = 0; i < id.size(); i++)
		{
			int r = id[i];
			fwrite(&r, sizeof(int), 1, fg);
		}
		fclose(fg);
	}
public:
	void buildRowMatrix(const char *db, int total)
	{
		FILE *fr = fopen(db, "r"); assert(fr);
		int branchPos = 0, line = 0;
		int labelPos = 0;
		__int64 io = 0;

		int gid, v, e;
		int f, t, l;
		int count = 0;

		matrixTuple *cacheBranch = new matrixTuple[TUPLELEN];
		matrixTuple *cacheLabel = new matrixTuple[TUPLELEN];

		while (!feof(fr))
		{
			fscanf(fr, "%d\n", &gid);
			fscanf(fr, "%d %d\n", &v, &e);
			int ch;
			vector<int> vc(v, 0);
			for (int i = 0; i < v; i++)
			{
				fscanf(fr, "%d\n", &ch);
				vc[i] = ch;
			}
			vector<graph::edge> te;
			vector<vector<graph::edge > > ve(v, te);

			for (int i = 0; i < e; i++)
			{
				fscanf(fr, "%d %d %d\n", &f, &t, &l);
				graph::edge ef(t, l); graph::edge et(f, l);
				ve[f].push_back(ef);
				ve[t].push_back(et);
			}
			count++;
			graph g(gid, v, vc, ve, e);
			Rect rt(g.v, g.e, g.v, g.e);
			tree.Insert(rt.min, rt.max, line);
			insert_graph(g, cacheBranch, branchPos, cacheLabel, labelPos, line, io);
			if (count >= total)
				break;
		}
		if (fr) fclose(fr);
		if (branchPos > 0)
			fwrite(cacheBranch, sizeof(matrixTuple), branchPos, fb);
		if (labelPos > 0)
			fwrite(cacheLabel, sizeof(matrixTuple), labelPos, fl);
		if (cacheLabel) delete[] cacheLabel;
		if (cacheBranch) delete[] cacheBranch;
		fflush(fb);
		fflush(fl);
	}

};

#endif