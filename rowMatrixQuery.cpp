#include "editDistance.h"
#include "store.h"
#include "rowQuery.h"

int main(int argc, char *argv[])
{
	if(argc < 7){cout << "parameter error: query prefix hp total threshold blobPath" << endl; return 0;}
	string query = argv[1];
	string prefix = argv[2];
	string hp = argv[3];
	int total = atoi(argv[4]);
	int d = atoi(argv[5]);
	
	struct timeval start,end; 
	float timeuse; 
	gettimeofday(&start,NULL); 
	
	__int64 io = 0;
	int sum = 0;
	vector<vector<int > > verifyGraphSet;

	vector<graph> queryGraph = graph::readGraphMemory(query.c_str(), 50);	
	int graphNumber = queryGraph.size();
	rowQuery *sv = new rowQuery(prefix, hp, total);

	for (int j = 0; j < graphNumber; j++)
	{
		vector<int> result;
		sv->queryGraph(queryGraph[j], io, result, d);
		sum += result.size();
		verifyGraphSet.push_back(result);
		result.clear();
	}
	gettimeofday(&end, NULL); 
	timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec; 
	timeuse = timeuse * 1.0 / 1000000; 

	cout << "average Size and query time (s, io):" << d << " " << sum * 1.0 / graphNumber<< " " << timeuse << "s" << " " << io * 1.0 / graphNumber << endl;
	if(sv) delete sv;	

	string path = argv[6];
	store st(path + ".idx");
	gettimeofday(&start, NULL); 

	int real = 0;
	for (int j = 0; j < graphNumber; j++)
	{
		vector<int> result;
		string vout = "./verify/" + common::inToString(j);	
		st.extractGraphBlob((path +".blb").c_str(), vout.c_str(), verifyGraphSet[j]);
		editDistance::verifyEditDistance(vout, verifyGraphSet[j].size(), queryGraph[j], d, result);
		real += result.size();
	}
	gettimeofday(&end, NULL); 
	timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec; 
	timeuse = timeuse * 1.0 / 1000000; 

	cout << "real size:" << real * 1.0 / graphNumber << ", " << timeuse << "s" << endl;

	return 0;
}

