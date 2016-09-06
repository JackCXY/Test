#ifndef _COMMON_H
#define _COMMON_H
#include "stdafx.h"
#include "matrixTuple.h"

class common
{
public:
	static string inToString(string prefix, int v, int e)
	{
		stringstream ss;
		ss << v << e;
		return prefix + "/" + ss.str();
	}
	static string inToString(int n)
	{
		stringstream ss;
		ss << n;
		return ss.str();
	}

	static int unionStringSet(map<string, int> &s1, map<string, int> &s2)
	{
		map<string, int> ::iterator iter, it;
		int sum = 0;
		for (iter = s1.begin(); iter != s1.end(); ++iter)
		{
			it = s2.find(iter->first);
			if (it != s2.end())
				sum += min(iter->second, it->second);
		}
		return sum;
	}

	//unionSet<>(Set A * B) for vertex and edge label
	static int unionSet(map<int, int> &lv1, map<int, int> &lv2)
	{
		map<int, int>::iterator iter, it;
		int sum = 0;
		for (iter = lv1.begin(); iter != lv1.end(); ++iter)
		{
			it = lv2.find(iter->first);
			if (it != lv2.end())
				sum += iter->second < it->second ? iter->second : it->second;
		}
		return sum;
	}
	
	static int starEditDistance(vector<int> &s1, vector<int> &s2)
	{
		int sameLabel = 0;
		int ed = 0;
		int len1 = s1.size(), len2 = s2.size();
		if (len1 == 0) return len2;
		if (len2 == 0) return len1;

		if (s1[len1 - 1] != s2[len2 - 1])
			ed += 1;
		int i = len1 - 2, j = len2 - 2;
		while (i >= 0 && j >= 0)
		{
			if (s1[i] == s2[j])
			{
				sameLabel++;
				i--;
				j--;
			}
			else if (s1[i] < s2[j])
				j--;
			else
				i--;
		}
		ed += max(len1 - 1, len2 - 1) - sameLabel;
		ed += s1.size() < s2.size() ? s2.size() - s1.size() : s1.size() - s2.size();
		return ed;
	}
	static vector<vector<double >> simMatrix(vector<vector<int> > &s1, vector<vector<int >> &s2)
	{
		int r = s1.size();
		int c = s2.size();
		int max = r < c ? c : r;
		vector<double> vd(max, 0.0);
		vector<vector<double > > sim(max, vd);
		//two different init method 
		bool f = (r == max);
		if (f) // s1.size() > s2.size();
		{
			for (int i = 0; i < r; i++)
			{
				int j = 0;
				for (; j < c; j++)
					sim[i][j] = starEditDistance(s1[i], s2[j]);
				for (; j < r; j++)
					sim[i][j] = s1[i].size();
			}
		}
		else
		{
			int i = 0;
			for (; i < r; i++)
				for (int j = 0; j < c; j++)
					sim[i][j] = starEditDistance(s1[i], s2[j]);
			for (; i < c; i++)
				for (int j = 0; j < c; j++)
					sim[i][j] = s2[j].size();
		}
		return sim;
	}
	
	static int unionElem(vector<int> &v1, vector<int> &v2)
	{
		int i = 0, j = 0;
		int total = 0;
		while (i < v1.size() && j < v2.size())
		{
			if (v1[i] == v2[j])
			{
				total++;
				i++;
				j++;
			}
			else if (v1[i] < v2[j]) i++;
			else
				j++;

		}
		return total;
	}
	//path: ²»´ø/
	static void getJustCurrentFile(string path, vector<string>& files)
	{
		long   hFile = 0;
		struct _finddata_t fileinfo;
		string p;
		if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
		{
			do
			{
				if (fileinfo.name == string(".") || fileinfo.name == string("..")) continue;
				if ((fileinfo.attrib &  _A_SUBDIR))
				{
					files.push_back(path +"/"+ fileinfo.name);
				}
				else
				{

				}
			} while (_findnext(hFile, &fileinfo) == 0);
			_findclose(hFile);
		}
	}
	static matrixTuple  readTupleData(FILE *&fr, int pos, map<int, matrixTuple *> &mt, __int64 &io)
	{
		int offset = pos / TUPLELEN;
		map<int, matrixTuple *> ::iterator iter;
		iter = mt.find(offset);

		if (iter == mt.end())
		{
			_fseeki64(fr, offset * BLOCKSIZE, SEEK_SET);
			matrixTuple  *cache = new matrixTuple[TUPLELEN];
			fread(cache, sizeof(matrixTuple), TUPLELEN, fr); io++;
			mt.insert(pair<int, matrixTuple * >(offset, cache));
			return cache[pos %  TUPLELEN];
		}
		else
			return iter->second[pos %  TUPLELEN];
	}
	static int  readIntData(FILE *&fr, int pos, map<int, int *> &mt, __int64 &io)
	{
		int offset = pos / INITLEN;
		map<int, int *> ::iterator iter;
		iter = mt.find(offset);
		if (iter == mt.end())
		{
			_fseeki64(fr, offset * BLOCKSIZE, SEEK_SET);
			int *cache = new int[INITLEN];
			fread(cache, sizeof(int), INITLEN, fr); io++;
			mt.insert(pair<int, int* >(offset, cache));
			return cache[pos % INITLEN];
		}
		else
			return iter->second[pos % INITLEN];
	}
	static __int64 FileSize(const char *in)
	{
#ifdef WIN32
		if (_access(in, 0) == -1){ cout << "the file:" << in << " is not existing" << endl; exit(0); }
#else
		if(access(in, F_OK | R_OK) == -1){ cout << "the file:" << in << " is not existing" << endl; exit(0); }
#endif
		struct _stati64 info;
		_stati64(in, &info);
		return info.st_size; // unit in Bytes
	}
	static __int64 FileSize(const char *in, int size)
	{
		struct _stati64 info;
		_stati64(in, &info);
		__int64 totalsize = info.st_size / size;
		return totalsize;
	}

	 //uing for the QSearch : ensure  that: L >= 2
	static void position(int tv, int te, int maxv, int maxe, int areaLength, int &x, int &y)
	{
		int pointx = (maxv - areaLength / 2);
		int pointy = (maxe - areaLength / 2);
		if (tv >= pointx)
		{
			int disx = tv - pointx;
			x = disx / areaLength;
		}
		if (tv < pointx)
		{
			int disx = pointx - tv;
			x = -(disx / areaLength) - 1;
		}
		if (te >= pointy)
		{
			int disy = te - pointy;
			y = disy / areaLength;
		}
		if (te < pointy)
		{
			int disy = pointy - te;
			y = -(disy / areaLength) - 1;
		}
	}
	static double indexSize(string prefix)
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
