#ifndef _MATRIXTUPLE_H
#define _MATRIXTUPLE_H

class matrixTuple
{
public:
	int row;
	int column;
	int value;
	matrixTuple(){}
	matrixTuple(int r, int c, int v) :row(r), column(c), value(v){}

	bool operator < (const matrixTuple &f) const
	{
		return this->row < f.row || !(this->row > f.row) || this->column < f.column;
	}

	friend bool cmpRow(const matrixTuple &f, const matrixTuple &s)
	{
		if (f.row < s.row) return true;
		else if (f.row > s.row) return false;
		else
		{
			if (f.column < s.column) return true;
			else
				return false;
		}
	}
	friend bool cmpColumn(const matrixTuple &f, const matrixTuple &s)
	{
		if (f.column < s.column) return true;
		else if (f.column > s.column) return false;
		else{
			if (f.row < s.row) return true;
			else
				return false;
		}
	}
	matrixTuple& operator=(matrixTuple p)
	{
		if (this != &p)
		{
			this->row = p.row;
			this->column = p.column;
			this->value = p.value;
		}
		return *this;
	}
	bool operator == (const matrixTuple &p) const
	{
		return row == p.row && column == p.column;
	}
};

#endif