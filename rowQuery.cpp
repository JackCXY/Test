#include "stdafx.h"
#include "rowQuery.h"

void  rowQuery::row_add(FILE *&fr, FILE *&ff, map<int, matrixTuple*> &mf, vector<int> &vr, map<int, int> &gv, map<int, int> &gb, __int64 &io)
{
	twoTuple *cache = new twoTuple[TWOLEN]; //read data and analysize the data
	matrixTuple *graphData = new matrixTuple[TWOLEN];
	matrixTuple mt;

	map<int, vector<matrixTuple> > pageSet;
	map<int, vector<matrixTuple> > ::iterator psiter;

	for (int i = 0; i < vr.size(); i++)
	{
		matrixTuple pageOff = common::readTupleData(ff, vr[i], mf, io);
		gb.insert(pair<int, int>(vr[i], 0));
		if (pageOff.row != -1)
		{
			psiter = pageSet.find(pageOff.row);
			if (psiter == pageSet.end())
			{
				vector<matrixTuple> a; a.push_back(matrixTuple(vr[i], pageOff.column, pageOff.value));
				pageSet.insert(pair<int, vector<matrixTuple> >(pageOff.row, a));
			}
			else
				psiter->second.push_back(matrixTuple(vr[i], pageOff.column, pageOff.value));
		}
	}

	for (psiter = pageSet.begin(); psiter != pageSet.end(); psiter++)
	{
		__int64 pageOff = psiter->first;
		_fseeki64(fr, pageOff * BLOCKSIZE, SEEK_SET); //seek the offset?:
		int len = fread(cache, sizeof(twoTuple), TWOLEN, fr); io++;  //optimized
		vector<matrixTuple>  tmp = psiter->second;
		int graphLen = 0;

		for (int j = 0; j < tmp.size(); j++)
		{
			for (int l = 0; l < tmp[j].value; l++)
			{
				mt.row = tmp[j].row;
				mt.column = cache[tmp[j].column + l].row;
				mt.value = cache[tmp[j].column + l].value;
				graphData[graphLen++] = mt;
			}
		}
		countingGram(gv, graphData, graphLen, gb);
	}

	if (cache) delete[] cache;
	if (graphData) delete[] graphData;
}
void rowQuery::row_add(FILE *&fr, FILE *&off, map<int, matrixTuple*> &mf, map<int, int> &gv, vector<int> &out, __int64 &io, int &matrixTupleTotal)
{
	rewind(fr);
	map<int, int> ::iterator liter;
	twoTuple *cache = new twoTuple[TWOLEN]; //read data and analysize the data
	matrixTuple *graphData = new matrixTuple[TWOLEN];
	matrixTuple mt;
	map<int, vector<matrixTuple> > pageSet;
	map<int, vector<matrixTuple> > ::iterator psiter;

	for (int i = 0; i < total; i++)
	{
		if (out[i] != -1)
		{
			matrixTuple pageOff = common::readTupleData(off, i, mf, io);
			if (pageOff.row != -1)
			{
				psiter = pageSet.find(pageOff.row);
				if (psiter == pageSet.end())
				{
					vector<matrixTuple> a; a.push_back(matrixTuple(i, pageOff.column, pageOff.value));
					pageSet.insert(pair<int, vector<matrixTuple> >(pageOff.row, a));
				}
				else
					psiter->second.push_back(matrixTuple(i, pageOff.column, pageOff.value)); //can write into File, then read it
			}
		}
	}
	//the computing process
	for (psiter = pageSet.begin(); psiter != pageSet.end(); psiter++) //pageSet must be in Mmemory
	{
		__int64 pageOff = psiter->first;
		_fseeki64(fr, pageOff * BLOCKSIZE, SEEK_SET); //seek the offset?:
		int len = fread(cache, sizeof(twoTuple), TWOLEN, fr); io++;  //optimized
		vector<matrixTuple>  tmp = psiter->second; //can optimized it:   
		int graphLen = 0;
		
		for (int j = 0; j < tmp.size(); j++)
		{
			for (int l = 0; l < tmp[j].value; l++)
			{
				mt.row = tmp[j].row;
				mt.column = cache[tmp[j].column + l].row;
				mt.value = cache[tmp[j].column + l].value;
				graphData[graphLen++] = mt;
			}
		}
		countingGram(gv, graphData, graphLen, out);
	}
	if (cache) delete[] cache;
	if (graphData) delete[] graphData;
}

void rowQuery::candidate_branch(int v, map<int, int > &gb, vector<int> &result, __int64 &io, int d)
{
	map<int, int>::iterator iter;
	for (iter = gb.begin(); iter != gb.end(); ++iter)
	{
		int i = iter->first;
		int dis = max(v, common::readIntData(fv, i, mv, io)) - 2 * d;
		if (iter->second >= dis)
			result.push_back(i);
	}
}
void rowQuery::candidate_branch(int v, vector<int> &out, vector<int> &result, __int64 &io, int d)
{
	assert(out.size() == total);
	for (int i = 0; i < out.size(); i++)
	{
		int dis = max(v, common::readIntData(fv, i, mv, io)) - 2 * d;
		if (out[i] != -1 && out[i] >= dis)
			result.push_back(i);
	}
}
void rowQuery::candidate_label(int v, int e, map<int, int > &gb, vector<int> &result, __int64 &io, int d)
{
	map<int, int>::iterator iter;
	for (iter = gb.begin(); iter != gb.end(); ++iter)
	{
		int i = iter->first;
		int dis = max(v, common::readIntData(fv, i, mv, io)) + max(e, common::readIntData(fe, i, me, io)) - d;
		if (iter->second >= dis)
			result.push_back(i);
	}
}
void rowQuery::candidate_label(int v, int e, vector<int> &out, vector<int> &result, __int64 &io, int d)
{
	assert(out.size() == total);
	for (int i = 0; i < out.size(); i++)
	{
		int dis = max(v, common::readIntData(fv, i, mv, io)) + max(e, common::readIntData(fe, i, me, io)) - d;
		if (out[i] != -1 && out[i] >= dis)
			result.push_back(i);
	}
	
}
void rowQuery::queryGraphRtree(graph g, __int64 &io, vector<int> &result, int d)
{
	vector<int> out;
	vector<int> vr;
	out.resize(total, -1);
	Rect rt(g.v - d, g.e - d, g.v + d, g.e + d);
	tree.Search(rt.min, rt.max, vr);
	sort(vr.begin(), vr.end());

	vector<int> br;
	vector<int> lr;
	map<int, int> gb;

	map<int, int> mb = g.weightBranchVector(eh);
	map<int, int> ml = g.weightLabelVector(eh);

	int matrixTupleTotal = 0;
	row_add(fl, flf, mlf, vr, ml, gb, io);
	candidate_label(g.v, g.e, gb, lr, io, d);
	
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

void rowQuery::queryGraph(graph g, __int64 &io, vector<int> &result, int d)
{
	vector<int> br;
	vector<int> lr;
	vector<int> out(total, 0);
	map<int, int> mb = g.weightBranchVector(eh);
	map<int, int> ml = g.weightLabelVector(eh);

	int matrixTupleTotal = 0;
	row_add(fl, flf, mlf, ml, out, io, matrixTupleTotal);
	candidate_label(g.v, g.e, out, lr, io, d);
	out.clear(); out.resize(total, 0);
	row_add(fb, fbf, mbf, mb, out, io, matrixTupleTotal);
	candidate_branch(g.v, out, br, io, d);


	sort(br.begin(), br.end());
	sort(lr.begin(), lr.end());
	int i = 0, j = 0;
	while (i < br.size() && j < lr.size())
	{
		if (br[i] < lr[j])
			i++;
		else if (br[i] > lr[j])
			j++;
		else
		{
			int graphID = common::readIntData(fg, br[i], md, io);
			result.push_back(graphID);
			i++;
			j++;
		}
	}
}