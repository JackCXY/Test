#include "stdafx.h"
#include "multiQuery.h"

void multiQuery::unionArea(int v, int e, int &lx, int &rx, int &ly, int &ry, int threshold)
{
	int lv = v - threshold;
	int rv = v + threshold;
	int le = e - threshold;
	int re = e + threshold;
	common::position(lv, le, maxv, maxe, areaLength, lx, ly);
	common::position(rv, re, maxv, maxe,areaLength, rx, ry);
}
void multiQuery::unionArea(int v, int e, int threshold, vector<string> &vr)
{
	int lx, rx, ly, ry;
	unionArea(v, e, lx, rx, ly, ry, threshold);

	for (int i = lx; i <= rx; i++)
	{
		for (int j = ly; j <= ry; j++)
		{
			string path = common::inToString(prefix, i, j);
			vr.push_back(path);
		}
	}
}

void multiQuery::queryGraph(graph g, __int64 &io, int threshold, vector<int> &vr)
{
	vr.clear();
	vector<string> queryArea;
	unionArea(g.v, g.e, threshold, queryArea);

	map<string, int> ::iterator iter;
	for (int i = 0; i < queryArea.size(); i++)
	{
		iter = fp.find(queryArea[i]);
		if (iter != fp.end())
		{
#ifdef WIN32
			assert(_access(queryArea[i].c_str(), 0) != -1);
#else
			assert(access(queryArea[i].c_str(), F_OK | R_OK) != -1);
#endif
			singleQuery *sg = new singleQuery(queryArea[i], hp, iter->second);
			sg->queryGraph(g, io, threshold, vr);
			if (sg) delete sg;
		}
	}
}
