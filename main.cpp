#include "stdafx.h"
#include "test.h"

int main(int argc, char* argv[])
{
	test *t = new test();
	//t->testBuildRowRTreeMatrix(AIDS, "./single/AIDS", "./single/AIDS", 42687);
	//t->testBuildSparseDense(AIDS, "./single/AIDS", "./single/AIDS", 42687, 0.06);
	//t->testBuildInverIndex("./single/AIDS", 42687);
	//t->testBuildRowMajorMatirx(AIDS, "./single/AIDS", "./single/AIDS", 42687);
	//t->testBuildMultiIndex(AIDS, "./multi/AIDS", "./multi/AIDS", 42687, 4, 0.06);
	
	//the query:
	cout << "1.row-Rtree 2. InvertIndex 3. sparseDense 4. multiQuery" << endl; 
	//t->testRowQuery(AIDS_Query, "./single/AIDS", "./single/AIDS", 42687);
	//t->testQueryInvertIndex(AIDS_Query, "./single/AIDS", "./single/AIDS", 42687);
	//t->testSparseDenseQuery(AIDS_Query, "./single/AIDS", "./single/AIDS", 42687);
	t->testMultiQuery(AIDS_Query, "./multi/AIDS", "./multi/AIDS");
	return 0;
}

