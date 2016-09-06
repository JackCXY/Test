#include "rowSpmv.h"

void rowSpmv::row_add(FILE *&fr, map<int, int> &gv, vector<int> &out, __int64 &io, int &matrixTupleTotal)
{
	out.clear();
	out.resize(total, 0);
	rewind(fr);

	int count = 0, temp = 0, sum = 0, len;
	map<int, int>::iterator it;
	matrixTuple mt;
	matrixTuple *cache = new matrixTuple[TUPLELEN];
	int zero = 0;

	while (!feof(fr))
	{
		len = fread(cache, sizeof(matrixTuple), TUPLELEN, fr); io++;
		matrixTupleTotal += len;

		for (int i = 0; i < len; i++)
		{
			mt = cache[i];
			if (mt.row == temp)
			{
				it = gv.find(mt.column);
				if (it != gv.end())
					sum += mt.value < it->second ? mt.value : it->second;
			}
			else
			{
				out[count++] = sum;
				for (int i = temp + 1; i < mt.row; i++)
				{
					out[count++] = zero;
				}
				temp = mt.row;
				it = gv.find(mt.column);
				if (it != gv.end())
					sum = mt.value < it->second ? mt.value : it->second;
				else
					sum = 0;
			}
		}
	}
	out[count++] = sum;
	for (int i = temp + 1; i < total; i++)
	{
		out[count++] = zero;
	}
	if (cache) delete[] cache;
}

void rowSpmv::candidate_branch(int v, vector<int> &out, vector<int> &result, __int64 &io)
{
	assert(out.size() == total);
	for (int i = 0; i < out.size(); i++)
	{
		int dis = max(v, common::readIntData(fv, i, mv, io)) - 2 * threold;
		if (out[i] >= dis)
			result.push_back(i);
	}
}

void rowSpmv::candidate_label(int v, int e, vector<int> &out, vector<int> &result, __int64 &io)
{
	assert(out.size() == total);
	for (int i = 0; i < out.size(); i++)
	{
		int dis = max(v, common::readIntData(fv, i, mv, io)) + max(e, common::readIntData(fe, i, me, io)) - threold;
		if (out[i] >= dis)
			result.push_back(i);
	}
}

void rowSpmv::queryGraph(graph g, __int64 &io, vector<int> &result)
{
	vector<int> br;
	vector<int> lr;
	vector<int> out;
	map<int, int> mb = g.weightBranchVector(eh);
	map<int, int> ml = g.weightLabelVector(eh);

	int matrixTupleTotal = 0;
	row_add(fb, mb, out, io, matrixTupleTotal);
	//system("sync && echo 3 >> /proc/sys/vm/drop_caches");
	candidate_branch(g.v, out, br, io);
	row_add(fl, ml, out, io, matrixTupleTotal);
	//system("sync && echo 3 >> /proc/sys/vm/drop_caches");
	candidate_label(g.v, g.e, out, lr, io);
	//get the union set: br and lr
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
			result.push_back(br[i]);
			i++;
			j++;
		}
	}
	//cout << matrixTupleTotal << endl;

}