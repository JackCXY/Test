#include "stdafx.h"
#include "emHash.h"

emHash::emHash(const char *file)
{
#ifdef WIN32
	if (_access(file, 0) == -1) //如果不存在，就创建
#else
	if (access(file, R_OK | W_OK) == -1)
#endif
	{
		INDEX = 0;
		fh = fopen(file, "wb+"); assert(fh);
		int d = -1;
		for (int i = 0; i < hashLen; i++)
		{
			fwrite(SENTRY.c_str(), sizeof(char), LEN, fh);
			fwrite(&d, sizeof(int), 1, fh);
		}
	}
	else
	{
		fh = fopen(file, "rb+"); assert(fh);
	}
}
int emHash::BKDRHASH(const char *str){

	unsigned int seed = 131;
	unsigned int hash = 0;
	while (*str){
		hash = hash * seed + (*str++);
	}
	hash = (hash & 0x7FFFFFFF);
	return hash % hashLen;

}

int emHash::find(string str, __int64 &pos, string &s){

	int v = this->BKDRHASH(str.c_str());
	pos = v * (LEN + sizeof(int));
	int groupID;
	s = this->strRead(pos, groupID);
	string temp = s;
	while (true){
		if (s == str)
			return groupID;
		if (s == SENTRY)
			return -1;//find the insertion place
		this->strCollapse(temp, v);
		v = this->BKDRHASH(temp.c_str());
		pos = v * (LEN + sizeof(int));
		s = this->strRead(pos, groupID);
	}
	return groupID;
}

void emHash::insert(string str)
{
	__int64 pos; int groupID;
	string s;
	groupID = this->find(str, pos, s);
	if (s == SENTRY)
	{
		this->strWrite(pos, str, INDEX++);
		return;
	}
	else if (s == str)
	{
		return;
	}
	else
	{
		cout << "error in insert string" << endl;
		return;
	}
}

map<int, int> emHash::queryVector(map<string, int> &fs)
{
	map<string, int>::iterator iter;
	map<int, int> qv;
	__int64 p; string s;

	for (iter = fs.begin(); iter != fs.end(); iter++)
	{
		string str = iter->first;
		int v = iter->second;
		this->insert(str);
		int groupID = this->find(str, p, s);
		if (s != str || groupID == -1)
		{
			cout << "not occuring in hash:" << s << " " << groupID << endl;
			continue;
		}
		qv.insert(pair<int, int>(groupID, v));
	}
	return qv;
}
