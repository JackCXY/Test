#ifndef _TWOLOSETREE_H
#define _TWOLOSETREE_H
#include "stdafx.h"
#include "twoTuple.h"

#define MINTWOTUPLE  twoTuple(-1, 0)
#define MAXTWOTUPLE  twoTuple((1<<31) - 1, (1<<31) - 1)

struct twoMergeData
{
	twoTuple *mData;
	int len;
	int pos;
	twoMergeData(){ len = pos = 0; mData = NULL; }
};

class twoLoseTree{

private:
	int K;
	__int64 totalsize;
	FILE *fr;

public:
	inline string inTostring(int n){
		stringstream ss;
		ss << n;
		return ss.str();
	}
	twoLoseTree(){ K = MELN; fr = NULL; }
	twoLoseTree(const char *file, __int64 n)
	{
		fr = fopen(file, "rb+"); assert(fr);
		this->totalsize = n;
		this->K = MELN;
	}
	~twoLoseTree(){ if (fr) fclose(fr); }

	void adjustTree(twoTuple *data, int *loseData, int i, int k)
	{
		int t = (i + k) / 2;
		int temp;
		while (t > 0)
		{
			if (data[loseData[t]] <  data[i])
			{
				temp = i;
				i = loseData[t];
				loseData[t] = temp;
			}
			t = t / 2;
		}
		loseData[0] = i;
	}
	void mergeRun(__int64 begin, __int64 end, __int64 runs, FILE *&fw, __int64 f, __int64 &io)
	{
		__int64 total = 0;
		int k = (end - begin < K) ? end - begin : K;
		__int64 *tlen = new __int64[k];
		__int64 *len = new __int64[k];
		int *loseData = new int[k];
		__int64 count = 0;
		int t;
		int cache = 0, num = 0;

		twoTuple *data = new twoTuple[k + 1];
		data[k] = MINTWOTUPLE;
		twoMergeData *md = new twoMergeData[k];
		twoTuple *cacheTuple = new twoTuple[TWOLEN];
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
			md[i].mData = new twoTuple[TWOLEN];

		for (int i = 0; i < k; i++)
		{
			_fseeki64(fr, offset[i] * sizeof(twoTuple), SEEK_SET);
			md[i].len = fread(md[i].mData, sizeof(twoTuple), TWOLEN, fr);
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
			if (cache == TWOLEN)
			{
				_fseeki64(fw, (offset[0] + num * TWOLEN) * sizeof(twoTuple), SEEK_SET);
				fwrite(cacheTuple, sizeof(twoTuple), TWOLEN, fw);
				cache = 0;
				num++;
			}
			if (tlen[t] == len[t]) data[t] = MAXTWOTUPLE;
			else
			{
				if (md[t].len == md[t].pos)
				{
					_fseeki64(fr, (offset[t] + tlen[t]) * sizeof(twoTuple), SEEK_SET);
					md[t].len = fread(md[t].mData, sizeof(twoTuple), TWOLEN, fr);
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
			fwrite(cacheTuple, sizeof(twoTuple), cache, fw);
			io++;
		}

		if (len) delete[] len; if (tlen) delete[] tlen;
		if (cacheTuple) delete[] cacheTuple;
		if (data) delete[] data;
		if (loseData) delete[] loseData;
		for (int i = 0; i < k; i++) if (md[i].mData) delete[] md[i].mData;
		if (md) delete[] md;

	}

	void kMerge(int level, __int64 runs, __int64 &io)
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
	void copyData(const char *in, const char *out)
	{
		FILE *fin = fopen(in, "rb");
		FILE *fout = fopen(out, "wb");
		twoTuple *cache = new twoTuple[TWOLEN];
		int len;
		while (!feof(fin))
		{
			if (len = fread(cache, sizeof(twoTuple), TWOLEN, fin))
				fwrite(cache, sizeof(twoTuple), len, fout);
		}
		if (fin) fclose(fin);
		if (fout) fclose(fout);
		if (cache) delete[] cache;
	}
	void externSort(const char *row, const char *out, __int64 &io);
	string externSort(__int64 &io, int &level)
	{
		twoTuple *mt = new twoTuple[TWOLEN];
		__int64 i = 0;

		while (i < totalsize)
		{
			_fseeki64(fr, i * sizeof(twoTuple), SEEK_SET);
			int len = fread(mt, sizeof(twoTuple), TWOLEN, fr);
			sort(mt, mt + len);
			_fseeki64(fr, i * sizeof(twoTuple), SEEK_SET);
			fwrite(mt, sizeof(twoTuple), len, fr);
			io++;
			i += len;
		}

		level = 0; __int64 runs = TWOLEN;
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
	void test()
	{

		FILE *fw = fopen("tmp.dat", "wb"); assert(fw);
		int i = 10000000;
		for (int j = 0; j < i; j++)
		{
			int r = (rand() * 100) % 40041;
			int value = j;
			twoTuple tt(r, value);
			fwrite(&tt, sizeof(twoTuple), 1, fw);
		}
		fclose(fw);
		ofstream fout("row_tmp.dat");
		twoLoseTree *tlt = new twoLoseTree();
		__int64 io = 0;
		tlt->externSort("tmp.dat", "out.dat", io);
		cout << io << endl;
		FILE *fr = fopen("out.dat", "rb");
		twoTuple tt;
		if (fread(&tt, sizeof(twoTuple), 1, fr))
			while (!feof(fr))
			{
				twoTuple temp;
				if (fread(&temp, sizeof(twoTuple), 1, fr))
				{
					fout << tt.row << " " << tt.value << endl;
					if (temp < tt)
					{
						cout << "error" << endl;
					}
					tt = temp;
				}

			}
		fclose(fr);
		fout.close();

	}

};
#endif