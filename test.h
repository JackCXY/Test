#ifndef _TEST_H
#define _TEST_H

#include "rowMatrix.h"
#include "rowMatrixBuild.h"
#include "rowMatrixRtree.h"
#include "rowQuery.h"
#include "invertBuild.h"
#include "invertQuery.h"
#include "singleIndex.h"
#include "singleQuery.h"
#include "multiIndex.h"
#include "multiQuery.h"
#include "RTree.h"

class test
{
public:
	void testBuildRowMatrix(string db, string p, string hp, int total)
	{
		rowMatrix *rm = new rowMatrix(p, hp);
		rm->buildRowMatrix(db.c_str(), total);
		if (rm) delete rm;
	}
	void testBuildRowRTreeMatrix(string db, string p, string hp, int total)
	{
		rowMatrixRtree *rm = new rowMatrixRtree(p, hp);
		rm->buildRowMatrix(db.c_str(), total);
		if (rm) delete rm;
	}
	void testBuildRowMajorMatirx(string db, string prefix, string hp, int t)
	{
		rowMatrixBuild *rm = new rowMatrixBuild(db, prefix, hp, t);
		rm->buildRowMajorMatrix();
		if (rm) delete rm;
	}
	void testBuildInverIndex(string prefix, int t)
	{
		invertBuild *ib = new invertBuild(prefix, t);
		ib->buildInvertIndex(); 
		if (ib) delete ib;
	}
	void testRowQuery(string query, string prefix, string hp, int total)
	{
		vector<graph> q = graph::readGraphMemory(query.c_str(), 50); 
		rowQuery *sv = new rowQuery(prefix, hp, 42687);
		for (int d = 1; d <= 5; d++)
		{
			__int64 io = 0;
			vector<int> result;
			int sum = 0;
			for (int i = 0; i < q.size(); i++)
			{
				sv->queryGraphRtree(q[i], io, result, d);
				sum += result.size();
				result.clear();
			}
			cout << "average candidate and io:" << 1.0 * sum / q.size() << " " << io * 1.0 / q.size() << endl;
		}
		if (sv) delete sv;
	}

	void testBuildSparseDense(string db, string prefix, string hp, int total, double f)
	{
		__int64 io = 0;
		singleIndex *si = new singleIndex(db, prefix, hp, total, f);
		si->buildSingleMatrix(io);
		si->eh->sequenceHash();
		if (si) delete si;
		cout << "done" << endl;
	}

	void testSparseDenseQuery(string query, string prefix, string hp, int total)
	{
		vector<graph> vg = graph::readGraphMemory(query.c_str(), 50);
		singleQuery *sv = new singleQuery(prefix, hp, total);
		for (int d = 1; d <= 5; d++)
		{
			__int64 io = 0;
			int sum = 0;
			for (int i = 0; i < vg.size(); i++)
			{
				vector<int> result;
				sv->queryGraph(vg[i], io, d, result);
				sum += result.size();
			}
			cout << "average:" << sum * 1.0 / vg.size() << ":" << 1.0 * io / vg.size() << endl;
		}
		if (sv) delete sv;
	}
	void testQueryInvertIndex(string query, string prefix, string hp, int total)
	{
		vector<graph> vg = graph::readGraphMemory(query.c_str(), 50);
		invertQuery *iq = new invertQuery(prefix, hp, total);

		for (int d = 1; d <= 5; d++)
		{
			__int64 io = 0;
			int sum = 0;
			for (int i = 0; i < vg.size(); i++)
			{
				vector<int> result;
				iq->queryGraph(vg[i], io, d, result);
				sum += result.size();
				
			}
			cout << "average:" << sum * 1.0 / vg.size() << ":" << 1.0 * io / vg.size() << endl;
		}
		if (iq) delete iq;
	}
	void testBuildMultiIndex(string db, string prefix, string hp, int total, int l, double f = 0.06)
	{
		__int64 io = 0; 
		vector<string> vs;
		multiIndex *mi = new multiIndex(prefix, hp, l, f);
		double sz = mi->build_index(db.c_str(), total, io);
		if (mi) delete mi;
	}

	void testMultiQuery(string query, string prefix, string hp)
	{
		vector<graph> vg = graph::readGraphMemory(query.c_str(), 50);
		multiQuery *mq = new multiQuery(prefix, hp);
		cout << mq->indexSize() << endl;
		for (int d = 1; d <= 5; d++)
		{
			__int64 io = 0;
			int sum = 0;
			for (int i = 0; i < vg.size(); i++)
			{
				vector<int> result;
				mq->queryGraph(vg[i], io, d, result);
				sum += result.size();

			}
			cout << "average:" << sum * 1.0 / vg.size() << ":" << 1.0 * io / vg.size() << endl;
		}
		if (mq) delete mq;
	}
};
#endif
