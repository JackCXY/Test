#include "twoLoseTree.h"

void twoLoseTree::externSort(const char *row, const char *out, __int64 &io)
{
	struct _stati64 info;
	_stati64(row, &info);
	totalsize = info.st_size / sizeof(twoTuple);
	if (fr) fclose(fr);
	fr = fopen(row, "rb+"); assert(fr);
	int level = 0;
	string path = externSort(io, level);
	if (fr) fclose(fr); fr = NULL;
	if (path == "")
	{
		this->copyData(row, out);
	}
	else
		this->copyData(path.c_str(), out);
	assert(level >= 0);
	for (int i = 0; i < level; i++)
	{
		string temp = "./sort/" + inTostring(i) + ".idx";
		std::remove(temp.c_str());
	}
}