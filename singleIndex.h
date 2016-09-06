#pragma once
#ifndef _SINGLE_INDEX_H
#define _SINGLE_INDEX_H

#include "stdafx.h"
#include "matrixTuple.h"
#include "branchHash.h"
#include "graph.h"
#include "loseTree.h"
#include "common.h"
#include "twoTuple.h"
#include "rowMatrix.h"

//single build
//1. the sparse part with InvertIndex
//2. the dense part with Lazzy mode

class singleIndex
{
	struct columnIndex
	{
		__int64 begin;
		int length;
		bool f;
		columnIndex(){ begin = length = 0; f = false; }
		columnIndex(__int64 b, int l)
		{
			begin = b;
			length = l;
			f = false;
		}
	};

public:

	FILE *fb;
	FILE *fl;

	FILE *bd; //branch
	FILE *ld; //label

	FILE *bs;
	FILE *ls;

	FILE *lp; // label page
	FILE *bp; //branch page

	ofstream fout;
	branchHash *eh;

	double factor;
	string prefix; // the singleIndex area prefix information
	int total;

	map<int, twoTuple> scl;
	map<int, twoTuple> scb;

public:
	singleIndex()
	{
		fb = fl = bd = ld = bs = ls = bp = lp = NULL;
		eh = NULL;
		total = 0;
		factor = 0.0;
	}
	~singleIndex()
	{
		this->writeInfo();
		fout.close();

		if (fb) fclose(fb);
		if (fl) fclose(fl);

		if (bd) fclose(bd);
		if (ld) fclose(ld);
		if (bs) fclose(bs);
		if (ls) fclose(ls);

		if (bp) fclose(bp);
		if (lp) fclose(lp);
	}
	
	singleIndex(string input, string p, string hp, int total, double f)
	{
		this->prefix = p;
		eh = branchHash::getInstance(hp);

#ifdef WIN32 
		//need to construct the column major Matrix
		if (_access((prefix + "/branch.clm").c_str(), 0) == -1 || _access((prefix + "/label.clm").c_str(), 0) == -1)
		{
			//cout << "begin build the clm matrix first" << endl;
			rowMatrix *rm = new rowMatrix(prefix, hp);
			rm->buildRowMatrix(input.c_str(), total);
			if (rm) delete rm;
			//then: extern sort the row Matrix
			__int64 io = 0; 
			loseTree *lt = new loseTree();
			//copy data then sort it 
			string tmp = "./temp_sort";
			lt->copyData((prefix + "/brow").c_str(), tmp.c_str());
			lt->externSort(tmp.c_str(), (prefix + "/branch.clm").c_str(), true, io);
			lt->copyData((prefix + "/lrow").c_str(), tmp.c_str());
			lt->externSort(tmp.c_str(), (prefix + "/label.clm").c_str(), true, io);
			if (lt) delete lt;
			std::remove(tmp.c_str());
		}
		if (_access((prefix + "/sparseDense").c_str(), 0) == -1)
		{
			if (_mkdir((prefix + "/sparseDense").c_str()) == -1)
			{
				cout << prefix << endl;
				cout << "mkdir error in path" << endl;
			}
		}
#else
		if (access((prefix + "/branch.clm").c_str(), F_OK | R_OK) == -1 || _access((prefix + "/label.clm").c_str(), F_OK | R_OK) == -1)
		{
			//cout << "begin build the clm matrix first" << endl;
			rowMatrix *rm = new rowMatrix(prefix, hp);
			rm->buildRowMatrix(input.c_str(), total);
			if (rm) delete rm;
			//then: extern sort the row Matrix
			__int64 io = 0; 
			loseTree *lt = new loseTree();
			//copy data then sort it 
			string tmp = "./temp_sort";
			lt->copyData((prefix + "/brow").c_str(), tmp.c_str());
			lt->externSort(tmp.c_str(), (prefix + "/branch.clm").c_str(), true, io);
			lt->copyData((prefix + "/lrow").c_str(), tmp.c_str());
			lt->externSort(tmp.c_str(), (prefix + "/label.clm").c_str(), true, io);
			if (lt) delete lt;
			std::remove(tmp.c_str());
		}
		if (access((prefix + "/sparseDense").c_str(), F_OK) == -1)
		{
			if (mkdir((prefix + "/sparseDense").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) == -1)
			{
				cout << prefix << endl;
				cout << "mkdir error in path" << endl;
			}
		}
#endif
		string branch_column = prefix + "/branch.clm";
		string label_column = prefix + "/label.clm";
	
		string branch_dense = prefix + "/sparseDense/bd";
		string label_dense = prefix + "/sparseDense/ld";
		string branch_sparse = prefix + "/sparseDense/bs";
		string label_sparse = prefix + "/sparseDense/ls";

		string fcp = prefix + "/sparseDense/cp";  //construct B+ tree
		string sp = prefix + "/sparseDense/sp"; // according to map<int, columnIndex> msb and msl
		string bpage = prefix + "/sparseDense/bpage";
		string lpage = prefix + "/sparseDense/lpage";

	
		fb = fopen(branch_column.c_str(), "rb+"); assert(fb);
		fl = fopen(label_column.c_str(), "rb+"); assert(fl);

		bd = fopen(branch_dense.c_str(), "wb+"); assert(bd);
		ld = fopen(label_dense.c_str(), "wb+"); assert(ld);
		bs = fopen(branch_sparse.c_str(), "wb+"); assert(bs);
		ls = fopen(label_sparse.c_str(), "wb+"); assert(ls);

		bp = fopen(bpage.c_str(), "wb+"); assert(bp);
		lp = fopen(lpage.c_str(), "wb+"); assert(lp);
		
		fout.open(prefix + "/sparseDense/invertdfo", ios::out);
		factor = f; //ensure that: factor > 0.0
		this->total = total;
	}
	void writeInfo()
	{
		fout << scl.size() << endl;
		for (map<int, twoTuple> ::iterator iter = scl.begin(); iter != scl.end(); ++iter)
		{
			fout << iter->first << " " << iter->second.row << " " << iter->second.value << endl;
		}
		fout << scb.size() << endl;
		for (map<int, twoTuple> ::iterator iter = scb.begin(); iter != scb.end(); ++iter)
		{
			fout << iter->first << " " << iter->second.row << " " << iter->second.value << endl;
		}
	}
public:
	
	void build_column_index(FILE *&fr, map<int, columnIndex> &mr, double f, __int64 &io);
	void buildRowIndex(FILE *&fc, FILE *&fm, const char *row, __int64& io);
	void buildSparseDense(FILE *&fc, string row, string sparse, map<int, columnIndex> &mr, __int64 &io);
	void buildSingleMatrix(__int64& io);
	void buildInvertColumn(string in, FILE *&fw, map<int, twoTuple> &scf);

public:
	//only statics the matrix of Dense and sparse part
	double indexSize()
	{
		__int64 totalSize = 0;
		totalSize += common::FileSize((prefix + "/sparseDense/bd").c_str());
		totalSize += common::FileSize((prefix + "/sparseDense/ld").c_str());
		totalSize += common::FileSize((prefix + "/sparseDense/bs").c_str());
		totalSize += common::FileSize((prefix + "/sparseDense/ls").c_str());
		double size = totalSize * 1.0 / (1024 * 1024.0);
		return size;
	}
};
#endif