#pragma once
#ifndef _GRAPH_H
#define _GRAPH_H

#include "stdafx.h"
#include "common.h"
#include "branchHash.h"

//the label of vertices and edges are dispatched with number
class graph
{
public:
	struct edge
	{
		int to;
		int einfo; //edgeLabel->e(from, to)
		edge(int t, int e)
		{
			to = t;
			einfo = e;
		}
	};
public:
	int v;
	vector<int> V;
	vector<vector<edge>> E;
	int graph_id;
	int e;

public:
	graph(){}
	~graph(){}
	int edgeinfo(int from, int to) //获取边(from, to)上的属性
	{
		int s;
		vector<edge> ef = E[from];
		int size = ef.size();
		int i = 0;
		for (; i < size; i++)
		{
			if (ef[i].to == to)
			{
				s = ef[i].einfo;
				break;
			}
		}
		if (i >= size)
		{
			cout << "desition error " << endl;
			cout << from << " " << to << endl;
		}
		return s;
	}
	//一介branch分支结构
	static vector<graph> readGraphMemory(const char *file, int total)
	{
		ifstream fr(file, ios::in);
		vector<graph> vg;
		int gid, v, e;
		int f, t, l;
		int vd;
		int count = 0;

		while (fr >> gid)
		{
			fr >> v >> e;
			vector<int> vc;
			for (int i = 0; i < v; i++)
			{
				fr >> vd;
				vc.push_back(vd); //insert the vertex label into vc
			}
			vector<graph::edge> te;
			vector<vector<graph::edge  > > ve(v, te);
			for (int i = 0; i < e; i++)
			{
				fr >> f >> t >> l;
				graph::edge ef(t, l); graph::edge et(f, l);
				ve[f].push_back(ef);
				ve[t].push_back(et);
			}
			graph g(gid, v, vc, ve, e);
			vg.push_back(g);
			count++;
			if (count >= total)
				break;
		}
		fr.close();
		return vg;

	}
	
	vector<vector<int > > starSet(int &maxDegree)
	{
		vector<vector<int > > star;
		maxDegree = E[0].size();
		star.push_back(starVertex(0));

		for (int i = 1; i < v; i++)
		{
			if (maxDegree < E[i].size())
				maxDegree = E[i].size();
			star.push_back(starVertex(i));
		}
		return star;
	}
	//negelet the label in the edge: according to the paper Cstar
	vector<int> starVertex(int id)
	{
		assert(id < v);
		vector<int> star;
		for (int i = 0; i < E[id].size(); i++)
		{
			int endVertex = V[E[id][i].to];
			star.push_back(endVertex);
		}
		sort(star.begin(), star.end());
		star.push_back(V[id]);
		return star;
	}

	string branchGramVertex(int id)
	{
		assert(id < v);
		int s = V[id];
		vector<edge> ve = E[id];
		vector<string> vs;
		string branchSequence = common::inToString(s);

		for (int i = 0; i < ve.size(); i++)
		{
			string temp = common::inToString(ve[i].einfo);
			vs.push_back(temp);
		}
		sort(vs.begin(), vs.end());
		for (int i = 0; i < vs.size(); i++)
			branchSequence += vs[i];
		return branchSequence;
	}
	//the <branch, freq> tuples
	map<string, int> branchGramSet()
	{
		map<string, int> tg;
		map<string, int>::iterator iter;
		for (int i = 0; i < v; i++)
		{
			string s = branchGramVertex(i);
			iter = tg.find(s);
			if (iter == tg.end())
			{
				tg.insert(pair<string, int>(s, 1));
			}
			else
				iter->second++;
		}
		return tg;
	}

	int maxDegree()
	{
		int max = 0;
		for (int i = 0; i < v; i++)
			if (max < E[i].size())
				max = E[i].size();
		return max;
	}

	graph(int gid, int num, vector<int> &VINFO, vector<vector<edge>> &einfo, int en)
	{
		graph_id = gid;
		v = num;
		V = VINFO;
		E = einfo;
		e = en;
	}
	graph(const graph &g)
	{
		this->graph_id = g.graph_id;
		this->v = g.v;
		this->e = g.e;
		this->V = g.V;
		this->E = g.E;
	}
	//the vertexLabel set of graph 
	map<string, int> vertexLabel()
	{
		map<string, int> mvl;
		map<string, int>::iterator iter;
		for (int i = 0; i < v; i++)
		{
			string str = "V" + common::inToString(V[i]);
			iter = mvl.find(str);
			if (iter == mvl.end())
				mvl.insert(pair<string, int>(str, 1));
			else
			{
				iter->second++;
			}
		}
		return mvl;
	}

	map<string, int> edgeLabel()
	{
		map<string, int> mvl;
		map<string, int>::iterator iter;
		for (int j = 0; j < v; j++)
		{
			vector<edge> ve = E[j];
			for (int i = 0; i < ve.size(); i++)
			{
				edge temp = ve[i];
				string einfo = "E" + common::inToString(temp.einfo);
				if (j < temp.to)
				{
					iter = mvl.find(einfo);
					if (iter == mvl.end())
						mvl.insert(pair<string, int>(einfo, 1));
					else
					{
						iter->second++;
					}
				}
			}
		}
		return mvl;
	}


	//the tree Gram using in the KAT
	string treeGramVertex(int id, int &degree)
	{
		assert(id < v);
		string s = common::inToString(V[id]);
		degree = E[id].size();
		vector<edge> ve = E[id];
		vector<string> vs;
		for (int i = 0; i < degree; i++)
		{
			string t = common::inToString(ve[i].einfo) + common::inToString(V[ve[i].to]);
			vs.push_back(t);
		}
		sort(vs.begin(), vs.end());
		for (int i = 0; i < vs.size(); i++)
			s += vs[i];
		return s;
	}
	//treeGram: 
	map<string, int > treeGramSet(int &maxDegree)
	{
		map<string, int> tg;
		map<string, int>::iterator iter;
		int temp;
		maxDegree = 0;
		for (int i = 0; i < v; i++)
		{
			string s = treeGramVertex(i, temp);
			if (maxDegree < temp)
				maxDegree = temp;
			iter = tg.find(s);
			if (iter == tg.end())
			{
				tg.insert(pair<string, int>(s, 1));
			}
			else
				iter->second++;
		}
		return tg;
	}
	map<int, int> weightBranchVector(branchHash *eh)
	{
		map<string, int> wbv = this->branchGramSet();
		map<int, int> qv = eh->queryVector(wbv);
		return qv;
	}
	map<int, int> weightLabelVector(branchHash *eh)
	{
		map<string, int> vl = this->vertexLabel();
		map<string, int> el = this->edgeLabel();
		map<int, int> vlw = eh->queryVector(vl);
		map<int, int> elw = eh->queryVector(el);

		map<int, int>::iterator iter;
		map<int, int> vew;
		for (iter = vlw.begin(); iter != vlw.end(); ++iter)
			vew.insert(pair<int, int>(iter->first, iter->second));
		for (iter = elw.begin(); iter != elw.end(); ++iter)
			vew.insert(pair<int, int>(iter->first, iter->second));
		return vew;
	}

};
#endif