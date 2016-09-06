#include "stdafx.h"
#include "columnSpmv.h"

columnSpmv::columnSpmv(const char *idx, const char *vertex, const char *edge, const char *brow, const char *lrow, int num, int d)
{
	eh = emHash::getInstance(idx);

	fv = fopen(vertex, "rb"); assert(fv);
	fe = fopen(edge, "rb"); assert(fe);
	fb = fopen(brow, "rb"); assert(fb);
	fl = fopen(lrow, "rb"); assert(fl);

	total = num;
	threold = d;
}


void columnSpmv::build_column_index(FILE *&fr, map<int, twoTuple> &columnIndex)
{
	rewind(fr);
	columnIndex.clear();

	matrixTuple *cache = new matrixTuple[TUPLELEN];
	int init_column = -1;
	int pos = 0, wpos = 0;
	matrixTuple mt;

	while (!feof(fr))
	{
		int len = fread(cache, sizeof(matrixTuple), TUPLELEN, fr);
		for (int i = 0; i < len; i++)
		{
			mt = cache[i];
			if (mt.column > init_column)
			{
				int len = pos - wpos;
				if (len > 0)
				{
					columnIndex.insert(pair<int, twoTuple>(init_column, twoTuple(wpos, len)));
				}
				wpos = pos;
				init_column = mt.column;
			}
			pos++;
		}
	}
	columnIndex.insert(pair<int, twoTuple>(init_column, twoTuple(wpos, pos - wpos)));
	if (cache) delete[] cache;
}

void columnSpmv::column_spmv(FILE *&fr, map<int, twoTuple> &cindex, map<int, int> gv, vector<int> &out, __int64 &io, int &matrixtotal)
{
	__int64 rio = 0, wio = 0;
	map<int, int>::iterator iter;
	map<int, twoTuple >::iterator miter;

	string temp = "./data/column_temp.dat";
	FILE *fw = fopen(temp.c_str(), "wb"); assert(fw);

	twoTuple mt;
	int n = 0, len, temp_count = 0;

	matrixTuple *cache = new matrixTuple[TUPLELEN];
	twoTuple *cacheWrite = new twoTuple[TWOLEN];

	for (iter = gv.begin(); iter != gv.end(); iter++)
	{
		int count = 0;
		int idx = iter->first;
		int value = iter->second;
		miter = cindex.find(idx);
		if (miter == cindex.end())
			continue;
		int s = miter->second.row;
		int clen = miter->second.value;
		_fseeki64(fr, s * sizeof(matrixTuple), SEEK_SET);

		matrixtotal += clen;

		while (count < clen)
		{
			int READLEN = (clen - count) < TUPLELEN ? (clen - count) : TUPLELEN;
			len = fread(cache, sizeof(matrixTuple), READLEN, fr); rio++;
			assert(len == READLEN);

			for (int j = 0; j < len; j++)
			{
				if (temp_count == TWOLEN)
				{
					fwrite(cacheWrite, sizeof(twoTuple), TWOLEN, fw); io++; wio++;
					temp_count = 0;
				}
				mt.row = cache[j].row;
				mt.value = cache[j].value < value ? cache[j].value : value;
				cacheWrite[temp_count++] = mt;
			}
			count += len;
			n = n + len;
		}
	}

	if (temp_count > 0)
	{
		fwrite(cacheWrite, sizeof(twoTuple), temp_count, fw); io++; wio++;
	}
	cout << "scan io" << rio + wio << endl;
	fclose(fw);
	if (cache) delete[] cache;
	if (cacheWrite) delete[] cacheWrite;
	part_sum(temp.c_str(), out, io);
	std::remove(temp.c_str());
}

void columnSpmv::part_sum(const char *in, vector<int> &out, __int64 &io)
{
	FILE *fr = fopen(in, "rb"); assert(fr);
	__int64 fileSize = FileSize(in, sizeof(twoTuple));

	int group = fileSize % total == 0 ? fileSize / total : fileSize / total + 1;
	int count = 0, len, pos = 0;
	int plength = total % TWOLEN == 0 ? total / TWOLEN : total / TWOLEN + 1;

	__int64 sio = 0;
	twoTuple *cache = new twoTuple[TWOLEN];
	string prefix = "./data/temp_column/";
#ifdef WIN32
	if (_access(prefix.c_str(), 0) == -1)
	{
		if (_mkdir(prefix.c_str()) == -1)
		{
			cout << "mkdir the sort folder error" << endl;
		}
	}
#else
	if (access("./sort/", R_OK | W_OK) == -1) // in the linux
	{
		if (mkdir("./sort/", 0777) == -1)
			cout << "mkdir sort folder error" << endl;
	}
#endif
	vector<string> path;
	for (int i = 0; i < group; i++)
	{
		string intmp = prefix + inToString(i);
		string tempout = prefix + inToString(i) + ".out";
		FILE *fw = fopen(intmp.c_str(), "wb+"); assert(fw);
		path.push_back(tempout);
		_fseeki64(fr, i * total * sizeof(twoTuple), SEEK_SET);

		for (int j = 0; j < plength; j++)
		{
			int READLEN = (total - j * TWOLEN) < TWOLEN ? (total - j * TWOLEN) : TWOLEN;
			len = fread(cache, sizeof(twoTuple), READLEN, fr); io++;
			fwrite(cache, sizeof(twoTuple), len, fw);
			if (feof(fr)) break;
		}
		if (fw) fclose(fw);
		sio += row_add(intmp.c_str(), tempout.c_str(), io);
		std::remove(intmp.c_str());
	}
	cout << "sort io:" << sio << endl;
	sio = row_add(path, out, io);  //merge step
	cout << "merge io:" << sio << endl;
	fclose(fr);
	if (cache) delete[] cache;
	for (int i = 0; i < path.size(); i++)
		std::remove(path[i].c_str());
}
__int64 columnSpmv::row_add(const char *in, const char *result, __int64 &io)
{
	__int64 bio = io;
	string temp = "./data/row_extern_temp.dat";
	twoLoseTree *lt = new twoLoseTree();
	lt->externSort(in, temp.c_str(), io);
	if (lt) delete lt;

	FILE *fr = fopen(temp.c_str(), "rb"); assert(fr);
	FILE *fw = fopen(result, "wb"); assert(fw);

	twoTuple *cache = new twoTuple[TWOLEN];
	twoTuple mt;

	int *cacheWrite = new int[INITLEN];
	int len, init_row = 0, i, j;
	int count = 0, zero = 0, sum = 0;

	while (!feof(fr))
	{
		len = fread(cache, sizeof(twoTuple), TWOLEN, fr); io++;
		for (i = 0; i < len; i++)
		{
			mt = cache[i];
			if (init_row == mt.row)
			{
				sum += mt.value;
			}
			else
			{
				if (count == INITLEN)
				{
					fwrite(cacheWrite, sizeof(int), INITLEN, fw);
					io++;
					count = 0;
				}
				cacheWrite[count++] = sum;
				for (j = init_row + 1; j < mt.row; j++)
				{
					if (count == INITLEN)
					{
						fwrite(cacheWrite, sizeof(int), INITLEN, fw);
						io++;
						count = 0;
					}
					cacheWrite[count++] = zero;
				}
				init_row = mt.row;
				sum = mt.value;
			}
		}
	}
	if (count == INITLEN)
	{
		fwrite(cacheWrite, sizeof(int), INITLEN, fw);
		io++;
		count = 0;
	}
	cacheWrite[count++] = sum;
	for (j = init_row + 1; j < total; j++)
	{
		if (count == INITLEN)
		{
			fwrite(cacheWrite, sizeof(int), INITLEN, fw);
			io++;
			count = 0;
		}
		cacheWrite[count++] = zero;
	}
	fwrite(cacheWrite, sizeof(int), count, fw); io++;
	__int64 eio = io;
	fclose(fw);
	fclose(fr);
	if (cacheWrite) delete[] cacheWrite;
	if (cache) delete[] cache;
	return eio - bio;
}

__int64 columnSpmv::row_add(vector<string> in, vector<int> &cacheWrite, __int64 &io)
{
	__int64 mio = 0;
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
			len = fread(cacheRead, sizeof(int), INITLEN, vf[j]); io++; mio++;

			for (int k = 0; k < len; k++)
			{
				cacheWrite[k + i * INITLEN] += cacheRead[k]; //here or what ? 
			}
		}
		mio++;
	}
	for (int i = 0; i < vf.size(); i++) if (vf[i]) fclose(vf[i]);
	if (cacheRead) delete[]cacheRead;
	return mio;
}

void columnSpmv::candidate_branch(int v, vector<int> &out, vector<int> &result)
{
	assert(out.size() == total);
	for (int i = 0; i < out.size(); i++) //ID->was line
	{
		int dis = max(v, readIntData(fv, i, mv)) - 2 * threold;
		if (out[i] >= dis)
			result.push_back(i);
	}
}

void columnSpmv::candidate_label(int v, int e, vector<int> &out, vector<int> &result)
{
	assert(out.size() == total);
	for (int i = 0; i < out.size(); i++)
	{
		int dis = max(v, readIntData(fv, i, mv)) + max(e, readIntData(fe, i, me)) - threold;
		if (out[i] >= dis)
			result.push_back(i);
	}
}

void columnSpmv::queryGraph(graph g, __int64 &io, vector<int> &result)
{
	vector<int> br;
	vector<int> lr;
	vector<int> out;
	map<int, int> mb = weightBranchVector(g);
	map<int, int> ml = weightLabelVector(g);
	map<int, twoTuple> columnIndex;

	//cout << "step1"<< endl;	
	build_column_index(fb, columnIndex);
	//cout << "build over" << endl;
	int matrixtotal = 0;
	column_spmv(fb, columnIndex, mb, out, io, matrixtotal);
	candidate_branch(g.v, out, br);

	//cout << "step2"<< endl;	
	build_column_index(fl, columnIndex);
	column_spmv(fl, columnIndex, ml, out, io, matrixtotal);
	candidate_label(g.v, g.e, out, lr);
	sort(br.begin(), br.end());
	sort(lr.begin(), lr.end());
	//cout << "step3"<< endl;	
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
	cout << matrixtotal << endl;
}
