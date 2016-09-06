#ifndef _TWOTUPLE_H
#define _TWOTUPLE_H

class twoTuple
{
public:
	int row;
	int value;
	twoTuple(){}
	twoTuple(int r, int v) :row(r), value(v){}
	bool operator<(const twoTuple &f) const  //according to the keywords: row
	{
		if (row < f.row) return true;
		else if (row >  f.row) return false;
		else
		{
			if (value < f.value) return true;
			else
				return false;
		}
	}
	twoTuple& operator=(twoTuple p)
	{
		if (this != &p)
		{
			this->row = p.row;
			this->value = p.value;
		}
		return *this;
	}
	bool operator==(const twoTuple &t)
	{
		return row == t.row && value == t.value;
	}
};
#endif

