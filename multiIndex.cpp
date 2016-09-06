#include "stdafx.h"
#include "multiIndex.h"

void multiIndex::staticGraphDataBase(const char *input, int total)
{
	this->total = total;

	FILE *fr = fopen(input, "r"); assert(fr);
	int gid, v, e;
	int vl;
	int f, t, l;
	map<int, int> vc;
	map<int, int> ec;
	map<int, int>::iterator iter;
	int  max;
	int count = 0;

	while (!feof(fr))
	{
		fscanf(fr, "%d\n", &gid);
		fscanf(fr, "%d %d\n", &v, &e);
		iter = vc.find(v);
		if (iter == vc.end())
		{
			vc.insert(pair<int, int>(v, 1));
		}
		else
			iter->second++;
		iter = ec.find(v);

		if (iter == ec.end()) { ec.insert(pair<int, int>(e, 1)); }
		else iter->second++;
		for (int i = 0; i < v; i++)
		{
			fscanf(fr, "%d\n", &vl);
		}
		for (int i = 0; i < e; i++)
		{
			fscanf(fr, "%d %d %d\n", &f, &t, &l);
		}
		count++;
		if (count % 500000 == 0)
			cout << count << endl;
		if (count >= total)
			break;
	}
	if (fr) fclose(fr);
	max = 0;
	for (iter = vc.begin(); iter != vc.end(); ++iter)
	{
		if (max < iter->second)
		{
			max = iter->second;
			maxv = iter->first;
		}
	}
	max = 0;
	for (iter = ec.begin(); iter != ec.end(); ++iter)
	{
		if (max < iter->second)
		{
			max = iter->second;
			maxe = iter->first;
		}
	}
}
void multiIndex::splitGraphDataBase(const char *input, map<string, int> &fp, int total)
{
	FILE *fr = fopen(input, "r"); assert(fr);
	int gid, v, e;
	int f, t, l;
	int vl;
	int count = 0;

	map<string, FILE *> mf; //@may occur the FILE number excessed a process can handle:
	map<string, FILE *> ::iterator iter;
	map<string, int>::iterator areaIter;

	string areaFile;

	FILE *fw = NULL;
	while (!feof(fr))
	{
		fscanf(fr, "%d\n", &gid);
		fscanf(fr, "%d %d\n", &v, &e);
		vector<int> vc(v, 0);
		vector<int> from;
		vector<int> to;
		vector<int> label;

		for (int i = 0; i < v; i++)
		{
			fscanf(fr, "%d\n", &vl);
			vc[i] = vl;
		}
		for (int i = 0; i < e; i++)
		{
			fscanf(fr, "%d %d %d\n", &f, &t, &l);
			from.push_back(f);
			to.push_back(t);
			label.push_back(l);

		}
		string garea = enchoding(prefix, v, e);
		iter = mf.find(garea);
		
		if (iter == mf.end())
		{
			areaFile = garea + "/input.dat";
			fw = fopen(areaFile.c_str(), "w"); assert(fw);
			mf.insert(pair<string, FILE *>(garea, fw)); //copy structure
			fp.insert(pair<string, int>(garea, 1));
		}
		else
		{
			fw = iter->second;
			areaIter = fp.find(garea);
			assert(areaIter != fp.end());
			areaIter->second++; 
		}
		fprintf(fw, "%d\n", gid);
		fprintf(fw, "%d %d\n", v, e);
		for (int i = 0; i < v; i++)
		{
			fprintf(fw, "%d\n", vc[i]);
		}
		for (int i = 0; i < e; i++)
		{
			fprintf(fw, "%d %d %d\n", from[i], to[i], label[i]);
		}

		count++;
		if (count >= total)
			break;
	}
	if (fr) fclose(fr);

	for (iter = mf.begin(); iter != mf.end(); ++iter)
	{
		if (iter->second)
		{
			fflush(iter->second);
			fclose(iter->second);
			iter->second = NULL;
		}
	}
}
double multiIndex::build_index(const char *input, int total, __int64 &io)
{
	map<string, int> ::iterator iter;
	staticGraphDataBase(input, total);
	splitGraphDataBase(input, this->fp, total);
	double indexSize = 0.0;
	cout << "split data over " << endl;

	for (iter = fp.begin(); iter != fp.end(); ++iter)
	{
		singleIndex *dl = new singleIndex(iter->first + "/input.dat", iter->first, this->hp, iter->second, factor);
		dl->buildSingleMatrix(io);
		indexSize += dl->indexSize();
		std::remove((iter->first + "/input.dat").c_str());
		if (dl) delete dl;
	}

	cout << "success " << endl;
	return indexSize;
}
