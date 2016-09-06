#pragma once
#ifndef _BRANCHHASH_H
#define _BRANCHHASH_H

#include "stdafx.h"
#include "twoTuple.h"
class branchHash
{
	//store the degree information:
private:
	string hashTable[10001]; //hashTable[hash(str)] = str
	int offsetID[10001]; // offsetID[hash(str)] = ID(str)
	int hashIndex;
	string SENTRY;
	string prefix;

public:
	static branchHash * instance;
	static branchHash * getInstance(string p);

private:
	branchHash(string prefix)
	{
		this->prefix = prefix;
		hashIndex = 0;
		SENTRY.resize(16, '$');
		for (int i = 0; i < 10001; i++)
		{
			hashTable[i] = SENTRY;
			offsetID[i] = -1;
		}
#ifdef WIN32 
	if (_access((prefix + "/eh.idx").c_str(), 0) != -1)
		this->readHashTable((prefix + "/eh.idx").c_str());
#else // need to rewrite in the linux 
	if(access((prefix + "/eh.idx").c_str(), F_OK | R_OK) != -1))
		this->readHashTable((prefix + "/eh.idx").c_str());
#endif

	}
public:
	branchHash(){}
	~branchHash(){ instance = NULL; this->sequenceHash(); }
	map<int, int> queryVector(map<string, int> &fs);
	map<int, int> queryVector(map<string, twoTuple> &fs)
	{
		map<string, twoTuple>::iterator iter;
		map<int, int> qv;
		int offset;
		for (iter = fs.begin(); iter != fs.end(); iter++)
		{
			string str = iter->first;
			int v = iter->second.row;
			insert(str, offset);
			int branchID = getBranchID(offset);
			qv.insert(pair<int, int>(branchID, v));
		}
		return qv;
	}

	void sequenceHash()
	{
		string hashFile = prefix + "/eh.idx";
		ofstream fo(hashFile);
		fo << 10001 << endl;
		for (int i = 0; i < 10001; i++)
			fo << hashTable[i] << endl;
		for (int i = 0; i < 10001; i++)
			fo << offsetID[i] << endl;
		fo.close();
	}

	void readHashTable(string hashFile)
	{
		ifstream fin(hashFile);
		int size;
		int idx, freq;
		string str;

		fin >> size;
		for (int i = 0; i < size; i++)
		{
			fin >> str;
			hashTable[i] = str;
		}
		for (int i = 0; i < 10001; i++)
		{
			fin >> idx;
			offsetID[i] = idx;
		}
		fin.close();
	}
	//test for the HashTable Size 

private:

	int DJBHASH(const char *str)
	{
		unsigned long hash = 5381;
		int c;
		while (c = *str++)
			hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		return hash % 10001;
	}
	void strCollapse(string &str, int value)
	{
		stringstream ss;
		ss << value;
		str += ss.str();
	}

	int getBranchID(const int &offset)
	{
		assert(offset < 10001);
		int iter = offsetID[offset];
		if (iter != -1)
			return iter;
		else
		{
			cout << "the mapping relationship error" << endl;
			return -1;
		}
	}

	int  find(string str, int &pos, int &hashValue)
	{
		hashValue = DJBHASH(str.c_str());
		string s = hashTable[hashValue];
		string temp = s;
		while (true)
		{
			if (s == str)
				return hashValue;
			if (s == SENTRY)
				return -1;//find the insertion place
			strCollapse(temp, hashValue);
			hashValue = DJBHASH(temp.c_str());
			s = hashTable[hashValue];
		}
		return -1;
	}
	void insert(const string &str, int &offset);
};

#endif