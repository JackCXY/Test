#include "stdafx.h"
#include "singleQuery.h"

/*@para:
1. fr: the invertIndex data file
2. gv: the twoTuple of graph
3. out: the dense part result
*/
void singleQuery::columnDenseAdd(FILE *&fr, map<int, int> &gv, map<int, twoTuple> &mcl, vector<int> &out, __int64 &io)
{
	out.clear();
	out.resize(total, 0);
	rewind(fr);

	map<int, int>::iterator iter;
	map<int, twoTuple >::iterator miter;
	twoTuple mt;
	int  len = 0;
	twoTuple *cache = new twoTuple[TWOLEN];

	for (iter = gv.begin(); iter != gv.end(); iter++)
	{
		int count = 0;
		int idx = iter->first;
		int value = iter->second;
		miter = mcl.find(idx);
		if (miter == mcl.end())
			continue;
		__int64 s = miter->second.row;
		int clen = miter->second.value;
		_fseeki64(fr, s * sizeof(twoTuple), SEEK_SET);

		while (count < clen)
		{
			int READLEN = (clen - count) < TWOLEN ? (clen - count) : TWOLEN;
			len = fread(cache, sizeof(twoTuple), READLEN, fr); io++;
			for (int j = 0; j < len; j++)
			{
				mt.row = cache[j].row;
				mt.value = cache[j].value < value ? cache[j].value : value;
				out[mt.row] += mt.value;
			}
			count += len;
		}
	}
	if (cache) delete[] cache;
}

/*@para:
1. v and e: the vertex and edge
2. gv:
3. out: the part sum
4. result: store the graph pass the part filter
*/
void singleQuery::columnDenFilter(int v, map<int, int> &gv, map<int, twoTuple> &mcl, vector<int> &out, __int64 &io, int threshold)
{
	map<int, twoTuple> ::iterator siter;
	int ys = 0;

	for (map<int, int>::iterator iter = gv.begin(); iter != gv.end(); iter++)
	{
		siter = mcl.find(iter->first); 
		if (siter == mcl.end()) //******the sparse part
			ys += iter->second;
	}
	for (int i = 0; i < total; i++)
	{
		int y = max(v, common::readIntData(fv, i, mv, io)) - 2 * threshold - ys;
		if (out[i] < y)
			out[i] = -1;
	}
}

void singleQuery::columnDenFilter(int v, int e, map<int, int> &gv, map<int, twoTuple> &mcl, vector<int> &out, __int64 &io, int threshold)
{
	map<int, twoTuple> ::iterator siter;
	int ys = 0;

	for (map<int, int>::iterator iter = gv.begin(); iter != gv.end(); iter++)
	{
		siter = mcl.find(iter->first);
		if (siter == mcl.end()) //******the sparese part
			ys += iter->second;
	}
	for (int i = 0; i < total; i++)
	{
		int y = max(v, common::readIntData(fv, i, mv, io)) + max(e, common::readIntData(fe, i, me, io)) - threshold - ys;
		if (out[i] < y)
			out[i] = -1;
	}
}

int singleQuery::countingGram(map<int, int> &gv, matrixTuple *mt, int len)
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
/*@para:
			1. gb: store <gid, common-gram>
*/
void singleQuery::countingGram(map<int, int> &gv, matrixTuple *cache, int graph_count, map<int, int> &gb)
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
				iter->second += value;
			}
			init_row = cache[i].row;
			length = 0;
			graphCache[length++] = cache[i];
		}

	}
	//*bug*: is here 
	if (length > 0) //the last cache need verify
	{
		if ((iter = gb.find(init_row)) != gb.end())
		{
			int value = countingGram(gv, graphCache, length);
			iter->second += value;
		}
	}
	if (graphCache) delete[] graphCache;
}

void singleQuery::row_add(FILE *&fr, FILE *&ff, map<int, matrixTuple*> &mf, vector<int> &out, vector<int> &vr, map<int, int> &gv, map<int, int> &gb, __int64 &io)
{

	gb.clear();

	twoTuple *cache = new twoTuple[TWOLEN]; //read data and analysize the data
	matrixTuple *graphData = new matrixTuple[TWOLEN];
	matrixTuple mt;

	map<int, vector<matrixTuple> > pageSet;
	map<int, vector<matrixTuple> > ::iterator psiter;
	sort(out.begin(), out.end());

	for (int j = 0; j < out.size(); j++)
	{
		int i = out[j]; 
		if (vr[i] != -1) //case 1: 在稠密区没被过滤掉
		{
			matrixTuple pageOff = common::readTupleData(ff, i, mf, io);
			gb.insert(pair<int, int>(i, vr[i]));
			if (pageOff.row != -1) //case 2: 在稀疏区存在三元组
			{
				psiter = pageSet.find(pageOff.row);
				if (psiter == pageSet.end())
				{
					vector<matrixTuple> a; a.push_back(matrixTuple(i, pageOff.column, pageOff.value));
					pageSet.insert(pair<int, vector<matrixTuple> >(pageOff.row, a));
				}
				else
					psiter->second.push_back(matrixTuple(i, pageOff.column, pageOff.value));
			}
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

void  singleQuery::row_add(FILE *&fr, FILE *&ff, map<int, matrixTuple*> &mf, vector<int> &vr, map<int, int> &gv, map<int, int> &gb, __int64 &io)
{
	gb.clear();
	assert(vr.size() == total);

	twoTuple *cache = new twoTuple[TWOLEN]; //read data and analysize the data
	matrixTuple *graphData = new matrixTuple[TWOLEN];
	matrixTuple mt;

	map<int, vector<matrixTuple> > pageSet;
	map<int, vector<matrixTuple> > ::iterator psiter;

	for (int i = 0; i < total; i++)
	{
		if (vr[i] != -1) //case 1: 在稠密区没被过滤掉
		{
			matrixTuple pageOff = common::readTupleData(ff, i, mf, io);
			gb.insert(pair<int, int>(i, vr[i]));

			if (pageOff.row != -1) //case 2: 在稀疏区存在三元组，
			{
				psiter = pageSet.find(pageOff.row);
				if (psiter == pageSet.end())
				{
					vector<matrixTuple> a; a.push_back(matrixTuple(i, pageOff.column, pageOff.value));
					pageSet.insert(pair<int, vector<matrixTuple> >(pageOff.row, a));
				}
				else
					psiter->second.push_back(matrixTuple(i, pageOff.column, pageOff.value));
			}
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
/*@para:
			1. gb: store the part sum <gid, partSum>
*/
void singleQuery::qGramFilter(int v, int e, map<int, int> &gb, vector<int> &result, __int64 &io, int d)
{
	result.clear();
	map<int, int>::iterator iter;
	for (iter = gb.begin(); iter != gb.end(); iter++)
	{
		int value = max(common::readIntData(fv, iter->first, mv, io), v) + max(e, common::readIntData(fe, iter->first, me, io)) - d;
		if (iter->second >= value)
			result.push_back(iter->first);
	}
}

void singleQuery::qGramFilter(int v, map<int, int> &gb, vector<int> &result, __int64 &io, int d)
{
	result.clear();
	map<int, int>::iterator iter;
	for (iter = gb.begin(); iter != gb.end(); iter++)
	{
		int value = max(common::readIntData(fv, iter->first, mv, io), v) - 2 * d;
		if (iter->second >= value)
			result.push_back(iter->first);
	}
}

//some problem
void singleQuery::queryGraph(graph g, __int64 &io, int d, vector<int> &vr)
{
	map<int, int> mqb = g.weightBranchVector(eh);
	map<int, int> mql = g.weightLabelVector(eh);

	vector<int> out;
	map<int, int> tmp;
	columnDenseAdd(ls, mql, scl, out, io); 
	columnDenFilter(g.v, g.e, mql, scl, out, io, d); 
	row_add(ld, lp, ml, out, mql, tmp, io);
	qGramFilter(g.v, g.e, tmp, out, io, d);
	
	vector<int> bout;
	columnDenseAdd(bs, mqb, scb, bout, io);
	columnDenFilter(g.v, mqb, scb, bout, io, d);
	row_add(bd, bp, mb, out,  bout, mqb, tmp, io);
	qGramFilter(g.v, tmp, out, io, d);
	{
		vector<int>().swap(bout);
		map<int, int>().swap(tmp);
	}
	sort(out.begin(), out.end());
	for (int i = 0; i < out.size(); i++)
	{
		int gid = common::readIntData(fg, out[i], mg, io);
		vr.push_back(gid);
	}

}