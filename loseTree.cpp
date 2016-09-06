#include "stdafx.h"
#include "loseTree.h"
//在原始的文件基础上进行排序<紧凑储存>

loseTree::loseTree(const char *file, __int64 n, bool f)
{
	fr = fopen(file, "rb+"); assert(fr);
	this->totalsize = n;
	this->K = MELN;
	this->f = f;
}

void loseTree::adjustTree(matrixTuple *data, int *loseData, int i, int k){

	int t = (i + k) / 2;
	int temp;
	if (f)
	{
		while (t > 0)
		{
			if (cmpColumn(data[loseData[t]], data[i]))
			{
				temp = i;
				i = loseData[t];
				loseData[t] = temp;
			}
			t = t / 2;
		}
	}
	else
	{
		while (t > 0)
		{
			if (cmpRow(data[loseData[t]], data[i]))
			{
				temp = i;
				i = loseData[t];
				loseData[t] = temp;
			}
			t = t / 2;
		}
	}
	loseData[0] = i;
}

void loseTree::mergeRun(__int64 begin, __int64 end, __int64 runs, FILE *&fw, __int64 f, __int64 &io)
{
	__int64 total = 0;
	int k = (end - begin < K) ? end - begin : K;
	__int64 *tlen = new __int64[k];
	__int64 *len = new __int64[k];
	int *loseData = new int[k];
	__int64 count = 0;
	int t;
	int cache = 0, num = 0;

	matrixTuple *data = new matrixTuple[k + 1];
	data[k] = MINTUPLE;
	mergeData *md = new mergeData[k];
	matrixTuple *cacheTuple = new matrixTuple[TUPLELEN];
	vector<__int64> offset(k, 0);

	for (int i = 0; i < k; i++)
	{
		loseData[i] = k;
		tlen[i] = 0;
		if (i == k - 1 && end == f)
		{
			len[i] = totalsize - (end - 1) * runs;
		}
		else
		{
			len[i] = runs;
		}
		offset[i] = (begin + i) * runs;
		total += len[i];
		assert(total <= totalsize);
	}

	for (int i = 0; i < k; i++)
		md[i].mData = new matrixTuple[TUPLELEN];

	for (int i = 0; i < k; i++)
	{
		fseeko(fr, offset[i] * sizeof(matrixTuple), SEEK_SET);
		md[i].len = fread(md[i].mData, sizeof(matrixTuple), TUPLELEN, fr);
		io++;
		md[i].pos = 0;
	}

	for (int i = 0; i < k; i++)
	{
		data[i] = md[i].mData[0];
		md[i].pos++;
	}
	for (int i = k - 1; i >= 0; i--)
		adjustTree(data, loseData, i, k);

	while (count < total){
		t = loseData[0];
		cacheTuple[cache++] = data[t];
		count++; tlen[t]++;
		if (cache == TUPLELEN)
		{
			fseeko(fw, (offset[0] + num * TUPLELEN) * sizeof(matrixTuple), SEEK_SET);
			fwrite(cacheTuple, sizeof(matrixTuple), TUPLELEN, fw);
			io++;
			cache = 0;
			num++;
		}
		if (tlen[t] == len[t]) data[t] = MAXTUPLE;
		else
		{
			if (md[t].len == md[t].pos)
			{
				fseeko(fr, (offset[t] + tlen[t]) * sizeof(matrixTuple), SEEK_SET);
				md[t].len = fread(md[t].mData, sizeof(matrixTuple), TUPLELEN, fr);
				io++;
				md[t].pos = 0;
			}
			data[t] = md[t].mData[md[t].pos];
			md[t].pos++;
		}
		adjustTree(data, loseData, t, k);

	}
	if (cache > 0)
	{
		fwrite(cacheTuple, sizeof(matrixTuple), cache, fw);
		io++;
	}

	if (len) delete[] len; if (tlen) delete[] tlen;
	if (cacheTuple) delete[] cacheTuple;
	if (data) delete[] data;
	if (loseData) delete[] loseData;
	for (int i = 0; i < k; i++) if (md[i].mData) delete[] md[i].mData;
	if (md) delete[] md;

}

void loseTree::kMerge(int level, __int64 runs, __int64 &io)
{
	string path = "./sort/" + inTostring(level) + ".idx";
	FILE *fw = fopen(path.c_str(), "wb"); assert(fw);

	__int64 begin = 0;
	__int64 end = (totalsize % runs == 0) ? totalsize / runs : totalsize / runs + 1;

	while (begin < end)
	{
		__int64 e = (begin + K < end) ? begin + K : end;
		mergeRun(begin, e, runs, fw, end, io);
		begin += K;
	}
	if (fw) fclose(fw);
	if (fr) fclose(fr);
	fr = fopen(path.c_str(), "rb+");

}
string loseTree::externSort(__int64 &io, int &level)
{
	matrixTuple *mt = new matrixTuple[TUPLELEN];
	__int64 i = 0;
	while (i < totalsize)
	{
		fseeko(fr, i * sizeof(matrixTuple), SEEK_SET);
		int len = fread(mt, sizeof(matrixTuple), TUPLELEN, fr);
		io++;
		if (f) sort(mt, mt + len, cmpColumn);
		else  sort(mt, mt + len, cmpRow);
		fseeko(fr, i * sizeof(matrixTuple), SEEK_SET);
		fwrite(mt, sizeof(matrixTuple), len, fr);
		io++;
		i += len;
	}
	__int64 runs = TUPLELEN;
	while (runs < totalsize)
	{
		this->kMerge(level, runs, io);
		level++;
		runs = K * runs;
	}
	if (level >= 1)
	{
		string path = "./sort/" + inTostring(level - 1) + ".idx";
		level--;
		for (int i = 0; i < level; i++)
		{
			string tempPath = "./sort/" + inTostring(i) + ".idx";
			std::remove(tempPath.c_str());
		}
		return path;
	}
	else
		return "";
}
