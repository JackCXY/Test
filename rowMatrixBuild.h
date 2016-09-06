#ifndef  _ROW_MATRIX_BUILD_H
#define _ROW_MATRIX_BUILD_H
#include "stdafx.h"
#include "matrixTuple.h"
#include "branchHash.h"
//using lazzy methods to build the matrix 

class rowMatrixBuild
{
private:
	
	FILE *fb; 
	FILE *fl;
	FILE *fbs;
	FILE *fls;
	
	FILE *fbf; 
	FILE *flf; 
	
	string prefix;
	int total;

public:
	rowMatrixBuild(){ fb = fbs = fl = fls = NULL;  }
	rowMatrixBuild(string db, string p, string hp, int t)
	{
		total = t;
		prefix = p;
#ifdef WIN32 
		if (_access((prefix + "/brow").c_str(), 0) == -1 || _access((prefix + "/lrow").c_str(), 0) == -1)
		{
			cout << "build the row matrix now" << endl;
			rowMatrix *rm = new rowMatrix(prefix, hp);
			rm->buildRowMatrix(db.c_str(), total);
			if (rm) delete rm;
		}
		if (_access((prefix + "/ROW").c_str(), 0) == -1)
		{
			if (_mkdir((prefix + "/ROW").c_str()) == -1)
			{
				cout << "mkdir error" << endl;
				exit(0);
			}
		}
#else // in the linux environment
		if (access((prefix + "/brow").c_str(), F_OK | R_OK) == -1 || access((prefix + "/lrow").c_str(), F_OK | R_OK) == -1)
		{
			cout << "build the row matrix now" << endl;
			rowMatrix *rm = new rowMatrix(prefix, hp);
			rm->buildRowMatrix(db.c_str(), total);
			if (rm) delete rm;
		}	
		if (access((prefix + "/ROW").c_str(), F_OK | R_OK) == -1)
		{
			if (mkdir((prefix + "/ROW").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
			{
				cout << "mkdir error" << endl;
				exit(0);
			}
		}
#endif
		fb = fopen((prefix + "/brow").c_str(), "rb+"); assert(fb);
		fl = fopen((prefix + "/lrow").c_str(), "rb+"); assert(fl);
		fbs = fopen((prefix + "/ROW/brow").c_str(), "wb+"); assert(fbs);
		fls = fopen((prefix + "/ROW/lrow").c_str(), "wb+"); assert(fls);
		fbf = fopen((prefix + "/ROW/bpage").c_str(), "wb+"); assert(fbf);
		flf = fopen((prefix + "/ROW/lpage").c_str(), "wb+"); assert(flf);
	}
	~rowMatrixBuild()
	{
		if(fbf) fclose(fbf);
		if(flf) fclose(flf);
		if (fb) fclose(fb);
		if (fl) fclose(fl);
		if (fbs) fclose(fbs);
		if (fls) fclose(fls);
	}

public:
	//using the lazzy methods
	//ensure that: every graph in a single disk and give the efficiency test
	void buildRowMajor(FILE *&fr, FILE *&fc, FILE *&fm, __int64 &io)
	{
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
		if(pageLen > 0) fwrite(cachePage, sizeof(matrixTuple), pageLen, fm);

		if (cacheRead) delete[] cacheRead;
		if (cache) delete[] cache;
		if (cachePage) delete[] cachePage;
		if (writeData) delete[] writeData;
		
	}
	void buildRowMajorMatrix()
	{
		__int64  io = 0;
		buildRowMajor(fb, fbs, fbf, io);
		buildRowMajor(fl, fls, flf, io);
		fflush(fbs);
		fflush(fls);
	}
};
#endif