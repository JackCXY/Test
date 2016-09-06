#include "stdafx.h"
#include "invertQuery.h"

void invertQuery::columnAdd(FILE *&fr, map<int, int> gv, vector<int> &out, __int64 &io, int &matrixtotal)
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
		miter = columnIndex.find(idx);
		if (miter == columnIndex.end())
			continue;
		__int64 s = miter->second.row;
		int clen = miter->second.value;
		_fseeki64(fr, s * sizeof(twoTuple), SEEK_SET);

		matrixtotal += clen;
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
void  invertQuery::columnAdd(vector<string> in, vector<int> &cacheWrite, __int64 &io)
{
	
	cacheWrite.resize(total, 0);
	int *cacheRead = new int[INITLEN];
	for (int k = 0; k < total; k++)
		cacheWrite[k] = 0;
	vector<FILE *> vf(in.size(), NULL);
	int len;
	int blockTotal = total % INITLEN == 0 ? total / INITLEN : total / INITLEN + 1;

	for (int i = 0; i < in.size(); i++)
	{
		vf[i] = fopen(in[i].c_str(), "rb");
	}
	for (int i = 0; i < blockTotal; i++)
	{
		for (int j = 0; j < in.size(); j++)
		{
			len = fread(cacheRead, sizeof(int), INITLEN, vf[j]); io++; 
			for (int k = 0; k < len; k++)
			{
				cacheWrite[k + i * INITLEN] += cacheRead[k]; //here or what ? 
			}
		}
	}
	for (int i = 0; i < vf.size(); i++) if (vf[i]) fclose(vf[i]);
	if (cacheRead) delete[]cacheRead;
	
}

void invertQuery::candidate_branch(int v, vector<int> &out, vector<int> &result, __int64 &io, int threold)
{
	assert(out.size() == total);
	for (int i = 0; i < out.size(); i++) //ID->was line
	{
		int dis = max(v, common::readIntData(fv, i, mv, io)) - 2 * threold;
		if (out[i] >= dis)
			result.push_back(i);
	}
}

void invertQuery::candidate_label(int v, int e, vector<int> &out, vector<int> &result, __int64 &io, int threold)
{
	assert(out.size() == total);
	for (int i = 0; i < out.size(); i++)
	{
		int dis = max(v, common::readIntData(fv, i, mv, io)) + max(e, common::readIntData(fe, i, me, io)) - threold;
		if (out[i] >= dis)
			result.push_back(i);
	}
}

void invertQuery :: queryGraph(graph g, __int64 &io, int d, vector<int> &result)
{
	vector<int> br;
	vector<int> lr;
	vector<int> out;
	map<int, int> mb = g.weightBranchVector(eh);
	map<int, int> ml = g.weightLabelVector(eh);
	map<int, twoTuple> columnIndex;

	int matrixtotal = 0;

	columnAdd(fb, mb, out, io, matrixtotal);
	candidate_branch(g.v, out, br, io, d);
	
	columnAdd(fl, ml, out, io, matrixtotal);
	candidate_label(g.v, g.e, out, lr, io, d);
	
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

