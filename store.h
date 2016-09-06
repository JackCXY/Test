#ifndef _STORE_H
#define _STORE_H
#include "stdafx.h"
#include "common.h"
#include "graph.h"

class store
{
private:
	vector<int> offsetBlob; // the offset of sizeof(int)
	int BS;

public:
	store(){ BS = 128; }
	store(string offsetIndex)
	{
		BS = 128;
		FILE *fr = fopen(offsetIndex.c_str(), "rb+"); assert(fr);
		int offset;
		while (!feof(fr))
		{
			fread(&offset, sizeof(int), 1, fr);
			offsetBlob.push_back(offset);
		}
		if (fr) fclose(fr);
	}
	//storing the graph into Blob format
	//delete the error graph from the database 
	//
	void storeGraphBlob(const char *graphFile, const char *graphBlob, const char *offsetIndex, int n)
	{
		FILE *fr = fopen(graphFile, "r"); assert(fr);
		FILE *fm = fopen(graphBlob, "wb"); assert(fm);
		FILE *fi = fopen(offsetIndex, "wb"); assert(fi);

		int gid, v, e;
		int f, t, l;
		int total = 0;
		int pageIdx = 0;
		int vd;
		int mem = 0;

		while (!feof(fr))
		{
			fscanf(fr, "%d\n", &gid);
			fscanf(fr, "%d %d\n", &v, &e);
			fwrite(&pageIdx, sizeof(int), 1, fi);
			_int64 offset = pageIdx;
			//u64 offset = pageIdx;
			offset *= BS;
			_fseeki64(fm, offset, SEEK_SET);
			fwrite(&gid, sizeof(int), 1, fm);  mem += sizeof(int);
			fwrite(&v, sizeof(int), 1, fm);  mem += sizeof(int);
			fwrite(&e, sizeof(int), 1, fm);  mem += sizeof(int);

			for (int i = 0; i < v; i++)
			{
				fscanf(fr, "%d\n", &vd);
				fwrite(&vd, sizeof(int), 1, fm);
				mem += sizeof(int);
			}
			for (int i = 0; i < e; i++)
			{
				fscanf(fr, "%d %d %d\n", &f, &t, &l);
				fwrite(&f, sizeof(int), 1, fm); mem += sizeof(int);
				fwrite(&t, sizeof(int), 1, fm); mem += sizeof(int);
				fwrite(&l, sizeof(int), 1, fm); mem += sizeof(int);
			}
			pageIdx += mem % BS == 0 ? mem / BS : mem / BS + 1;
			mem = 0;
			total++;
			if (total >= n)
				break;
		}
		fclose(fm);
		fclose(fr);
		fclose(fi);
	}
	//the offsetVector has init when we are ready to read the graph according to standard format
	void extractGraphBlob(const char *in, const char *out, vector<int> &vr)
	{
		FILE *fr = fopen(in, "rb"); assert(fr);
		ofstream fout(out, ios::out);
		int gid, v, e, f, t, l, graphId, vs, offset;
		for (int i = 0; i < vr.size(); i++)
		{
			graphId = vr[i];
			offset = offsetBlob[graphId];
			__int64 pos = offset; pos *= BS;
			_fseeki64(fr, pos, SEEK_SET);
			fread(&gid, sizeof(int), 1, fr);

			if (gid != graphId)
			{
				cout << gid << " " << graphId << "error" << endl;
				break;
			}
			fread(&v, sizeof(int), 1, fr);
			fread(&e, sizeof(int), 1, fr);
			fout << graphId << endl;
			fout << v << " " << e << endl;
			for (int j = 0; j < v; j++)
			{
				fread(&vs, sizeof(int), 1, fr);
				fout << vs << endl;
			}
			for (int j = 0; j < e; j++)
			{
				fread(&f, sizeof(int), 1, fr);
				fread(&t, sizeof(int), 1, fr);
				fread(&l, sizeof(int), 1, fr);
				fout << f << " " << t << " " << l << endl;
			}
		}
		fclose(fr);
		fout.close();
	}
	// convert the graph into path-gram format
	void convertPG(string in, string out, int total)
	{
		ifstream fin(in);
		ofstream fout(out);
		int count = 0;
		int gid, v, e;
		int vd, f, t, l;
		while (fin >> gid)
		{
			fin >> v >> e;
			fout << "t " << gid << " " << v << endl;
			for (int j = 0; j < v; j++)
			{
				fin >> vd;
				fout << "v " << j << " " << vd << endl;
			}
			for (int j = 0; j < e; j++)
			{
				fin >> f >> t >> l;
				fout << "e " << f << " " << t << " " << l << endl;
			}
			count++;
			if (count >= total)
				break;
		}
		fout.close();
		fin.close();
	}
	void convertTKDE(string in, string out, int total)
	{
		ifstream fin(in);
		ofstream fout(out);
		int count = 0;
		int gid, v, e;
		int vd, f, t, l;
		while (fin >> gid)
		{
			fin >> v >> e;
			fout << "t " << gid << " " << v << " " << e << endl;
			for (int j = 0; j < v; j++)
			{
				fin >> vd;
				fout << j << " " << vd << endl;
			}
			for (int j = 0; j < e; j++)
			{
				fin >> f >> t >> l;
				fout << f << " " << t << " " << l << endl;
			}
			count++;
			if (count >= total)
				break;
		}
		fout.close();
		fin.close();
	}
	void convertTKDEQuery(string in, string folder, int total) //
	{
		ifstream fin(in);
		int count = 0;
		int gid, v, e;
		int vd, f, t, l;
		while (fin >> gid)
		{
			string out = folder + common::inToString(count);
			ofstream fout(out);
			fin >> v >> e;
			fout << "t " << gid << " " << v << " " << e << endl;
			for (int j = 0; j < v; j++)
			{
				fin >> vd;
				fout << j << " " << vd << endl;
			}
			for (int j = 0; j < e; j++)
			{
				fin >> f >> t >> l;
				fout << f << " " << t << " " << l << endl;
			}
			count++;
			fout.close();
			if (count >= total)
				break;
		}
		fin.close();
	}

};
#endif