#include "stdafx.h"
#include "singleIndex.h"

/*@para:
1. fr: the matrix as column major layout
2. mr: recode the column informatiom:(begin, length, flag)
*/
void singleIndex::build_column_index(FILE *&fr, map<int, columnIndex> &mr, double f, __int64 &io)
{
	matrixTuple *cache = new matrixTuple[TUPLELEN];
	map<int, columnIndex> ::iterator iter;
	int init_column = -1;
	__int64 pos = 0, wpos = 0;
	set<int> sparse_set;
	vector<twoTuple> vve;

	while (!feof(fr))
	{
		int len = fread(cache, sizeof(matrixTuple), TUPLELEN, fr); io++;
		for (int i = 0; i < len; i++)
		{
			matrixTuple mt = cache[i];
			if (mt.column > init_column)
			{
				int len = pos - wpos;
				if (len > 0)
				{
					mr.insert(pair<int, columnIndex>(init_column, columnIndex(wpos, len)));
				}
				wpos = pos;
				init_column = mt.column;
			}
			pos++;
		}
	}

	mr.insert(pair<int, columnIndex>(init_column, columnIndex(wpos, pos - wpos)));
	for (iter = mr.begin(); iter != mr.end(); ++iter)
	{
		vve.push_back(twoTuple(iter->second.length, iter->first));
	}
	sort(vve.begin(), vve.end());
	for (int i = 0; i < vve.size() / 2; i++)
	{
		twoTuple vt = vve[i];
		vve[i] = vve[vve.size() - 1 - i];
		vve[vve.size() - 1 - i] = vt;
	}
	int max = (int)(factor * vve.size()) + 1;
	for (int i = 0; i < max; i++)
		sparse_set.insert(vve[i].value); //the dense part 
	for (iter = mr.begin(); iter != mr.end(); ++iter)
	{
		if (sparse_set.find(iter->first) != sparse_set.end())
			iter->second.f = true;  //dense part
		else
			iter->second.f = false;  //sparse part:  using prefix filtering (invert Index)
	}
	if (cache) delete[] cache;
}
//dense：行优先存储
//sparse：列优先存储
/*@para:
1. fc: the matrix data file
2. row and sparse: the tmp file
3. mr: recode the column informatiom:(begin, length, flag)
*/
void singleIndex::buildSparseDense(FILE *&fc, string dense, string sparse, map<int, columnIndex> &mr, __int64 &io)
{
	map <int, columnIndex > ::iterator iter;
	matrixTuple *cache = new matrixTuple[TUPLELEN];

	string tempDense = "./temp_dense";
	FILE *fw = fopen(tempDense.c_str(), "wb+"); assert(fw);
	FILE *fm = fopen(sparse.c_str(), "wb+"); assert(fm);

	for (iter = mr.begin(); iter != mr.end(); ++iter)
	{
		bool f = iter->second.f;
		if (!f) // sparse area 
		{
			__int64 bp = iter->second.begin;
			_fseeki64(fc, bp * sizeof(matrixTuple), SEEK_SET);
			int length = iter->second.length;
			int count = 0;

			while (count < length)
			{
				int READLEN = (length - count) < TUPLELEN ? (length - count) : TUPLELEN;
				int len = fread(cache, sizeof(matrixTuple), READLEN, fc); io++;
				fwrite(cache, sizeof(matrixTuple), len, fm); io++;
				count += len;
			}

		}
		else //dense part 
		{
			__int64 bp = iter->second.begin;
			_fseeki64(fc, bp * sizeof(matrixTuple), SEEK_SET);
			int length = iter->second.length;
			int count = 0;

			while (count < length)
			{
				int READLEN = (length - count) < TUPLELEN ? (length - count) : TUPLELEN;
				int len = fread(cache, sizeof(matrixTuple), READLEN, fc); io++;
				fwrite(cache, sizeof(matrixTuple), len, fw); io++;
				count += len; 
			}
		}//else
	}//for
	if (fw) fclose(fw);
	if (fm) fclose(fm);
	if (cache) delete[] cache;

	loseTree *lt = new loseTree();
	lt->externSort(tempDense.c_str(), dense.c_str(), false, io); //dense: row Major
	if (lt) delete lt;
	std::remove(tempDense.c_str());

}
void singleIndex::buildSingleMatrix(__int64& io)
{
	map<int, columnIndex> mb;
	map<int, columnIndex> ml;
	build_column_index(fl, ml, factor, io);
	build_column_index(fb, mb, factor , io);

	string dense = "./tempDense";
	string sparse = "./tempSparse";

	buildSparseDense(fb, dense, sparse, mb, io);
	buildInvertColumn(sparse, bs, scb);
	buildRowIndex(bd, bp, dense.c_str(), io);
	
	buildSparseDense(fl, dense, sparse, ml, io);
	buildInvertColumn(sparse, ls, scl);
	buildRowIndex(ld, lp, dense.c_str(), io);
	
	std::remove(dense.c_str());
	std::remove(sparse.c_str());
}
/*@para:
1. fc: store the matrixTuple data
2. fm: store the page offset information
3. row: the tmp file
*/
void singleIndex::buildRowIndex(FILE *&fc, FILE *&fm, const char *row, __int64& io)
{

	FILE *fr = fopen(row, "rb"); assert(fr);
	const int MAXLABEL = 10000;
	int zero = 0;
	matrixTuple *cache = new matrixTuple[MAXLABEL];
	matrixTuple *cachePage = new matrixTuple[TUPLELEN];
	matrixTuple *cacheRead = new matrixTuple[TUPLELEN];
	twoTuple *writeData = new twoTuple[TWOLEN];

	matrixTuple blockZero(-1, -1, -1);
	matrixTuple mt;

	int block = 0;
	int init_row = 1;
	int blockLeft = TWOLEN;
	bool flag = true;
	int graphLen = 0;
	int pageLen = 0;
	int dataLen = 0;

	while (!feof(fr))
	{
		int len = fread(cacheRead, sizeof(matrixTuple), TUPLELEN, fr);io++;
		for (int i = 0; i < len; i++)
		{
			if (flag)
			{
				init_row = cacheRead[i].row;
				for (int j = 0; j < init_row; j++)
				{
					if (pageLen == TUPLELEN)
					{
						fwrite(cachePage, sizeof(matrixTuple), TUPLELEN, fm);
						_fseeki64(fm, LEFTBLOCK, SEEK_CUR);
						pageLen = 0;
					}
					cachePage[pageLen++] = blockZero; 
				}
				flag = false;
			}
			if (init_row == cacheRead[i].row)
			{
				cache[graphLen++] = cacheRead[i]; 
			}
			else
			{
				if (graphLen > blockLeft)
				{
					for (int j = 2 * blockLeft; j >= 0; j--) //fill the file 
						fwrite(&zero, sizeof(int), 1, fc);
					block++;
					blockLeft = TWOLEN;
					_fseeki64(fc, block * BLOCKSIZE, SEEK_SET);
				}
				for (int k = 0; k < graphLen; k++)
				{
					writeData[k].row = cache[k].column;
					writeData[k].value = cache[k].value;
				}
				fwrite(writeData, sizeof(twoTuple), graphLen, fc); 
				mt.row = block; mt.column = TWOLEN - blockLeft; mt.value = graphLen;

				if (pageLen == TUPLELEN)
				{
					fwrite(cachePage, sizeof(matrixTuple), TUPLELEN, fm);
					_fseeki64(fm, LEFTBLOCK, SEEK_CUR);
					pageLen = 0;
				}
				cachePage[pageLen++] = mt;

				for (int j = init_row + 1; j < cacheRead[i].row; j++)
				{
					if (pageLen == TUPLELEN)
					{
						fwrite(cachePage, sizeof(matrixTuple), TUPLELEN, fm);
						_fseeki64(fm, LEFTBLOCK, SEEK_CUR);
						pageLen = 0;
}
					cachePage[pageLen++] = blockZero;
					}

				blockLeft -= graphLen;
				graphLen = 0;
				init_row = cacheRead[i].row;
				cache[graphLen++] = cacheRead[i];
				}
			}
		}

	if (graphLen > 0)
	{
		if (graphLen > blockLeft)
		{
			for (int j = 2 * blockLeft; j >= 0; j--)
				fwrite(&zero, sizeof(int), 1, fc);
			block++;
			blockLeft = TUPLELEN;
			_fseeki64(fc, block * BLOCKSIZE, SEEK_SET);
		}
		for (int k = 0; k < graphLen; k++)
		{
			writeData[k].row = cache[k].column;
			writeData[k].value = cache[k].value;
		}
		fwrite(writeData, sizeof(twoTuple), graphLen, fc);
		mt.row = block; mt.column = TWOLEN - blockLeft; mt.value = graphLen;

		if (pageLen == TUPLELEN)
		{
			fwrite(cachePage, sizeof(matrixTuple), TUPLELEN, fm);
			_fseeki64(fm, LEFTBLOCK, SEEK_CUR);
			pageLen = 0;
		}
		cachePage[pageLen++] = mt;

		for (int j = init_row + 1; j < total; j++)
		{
			if (pageLen == TUPLELEN)
			{
				fwrite(cachePage, sizeof(matrixTuple), TUPLELEN, fm);
				_fseeki64(fm, LEFTBLOCK, SEEK_CUR);
				pageLen = 0;
			}
			cachePage[pageLen++] = blockZero;
		}
	}
	if (pageLen > 0) fwrite(cachePage, sizeof(matrixTuple), pageLen, fm);

	if (cacheRead) delete[] cacheRead;
	if (cache) delete[] cache;
	if (cachePage) delete[] cachePage;
	if (writeData) delete[] writeData;
	if (fr) fclose(fr);
}
/*@para:
1.clm: the column need to build invertIndex
2.fw: the invert index file
*/
void singleIndex::buildInvertColumn(string in, FILE *&fw, map<int, twoTuple> &scf)
{
	FILE *fr = fopen(in.c_str(), "rb"); assert(fr);
	rewind(fw);

	matrixTuple *cache = new matrixTuple[TUPLELEN];
	twoTuple *cacheWrite = new twoTuple[TUPLELEN]; // 暂存由此产生的倒排索引数据

	int init_column = -1;
	int pos = 0, wpos = 0; //(1 << 31) * 12
	matrixTuple mt;

	while (!feof(fr))
	{
		int len = fread(cache, sizeof(matrixTuple), TUPLELEN, fr);
		for (int i = 0; i < len; i++)
		{
			mt = cache[i];
			cacheWrite[i].row = mt.row;
			cacheWrite[i].value = mt.value;
			if (mt.column > init_column)
			{
				int len = pos - wpos;
				if (len > 0)
				{
					scf.insert(pair<int, twoTuple>(init_column, twoTuple(wpos, len)));
				}
				wpos = pos;
				init_column = mt.column;
			}
			pos++;
		}
		fwrite(cacheWrite, sizeof(twoTuple), len, fw);
	}
	scf.insert(pair<int, twoTuple>(init_column, twoTuple(wpos, pos - wpos)));
	if (cache) delete[] cache;
	if (cacheWrite) delete[] cacheWrite;
	if (fr) fclose(fr);
}