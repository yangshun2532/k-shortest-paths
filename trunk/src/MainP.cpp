#pragma warning(disable: 4786)
#include <iostream>

#include "QYFloydWarshallShortestPaths.h"
#include "QYKShortestPaths.h"

using namespace std;
using namespace asu_emit_qyan;

int main(int argc, char* argv[])
{
	// Initiate the graph
	CQYDirectedGraph dg("data/query/ontology_graph.txt", true);

	// Run the KSP algorithm based on the end points
	const char* file_name = "data/query/query.txt";
	ifstream ifs(file_name);
	if (!ifs)
	{
		cout << "The file " << file_name << " can not be opened!" << endl;
		exit(1);
	}

	int start_node_id, end_node_id, top_k;
	ifs >> start_node_id >> end_node_id >> top_k;
	CQYKShortestPaths ksp(dg, start_node_id, end_node_id, top_k);
	vector<CQYDirectedPath*> topK_shortest_paths = ksp.GetTopKShortestPaths();

	// Export the result to a file
	const char* result_file_name = "data/query/query_results.txt";
	ofstream ofs(result_file_name, ios::out);
	if (!ofs)
	{
		cout << "The file " << result_file_name << " can not be opened!" << endl;
		exit(1);
	}
	ofs << topK_shortest_paths.size() << endl;
	for (vector<CQYDirectedPath*>::iterator pos=topK_shortest_paths.begin(); pos!=topK_shortest_paths.end(); ++pos)
	{
		(*pos)->PrintOut(ofs);
		ofs << endl;
	}

	ofs.close();

	// find all shortest paths of each pair of nodes in the graph
	CQYFloydWarshallShortestPaths fwsp(dg);
	fwsp.Execute();
	
	ofs.open("data/query/shortest_paths.txt", ios::out);
	fwsp.PrintOut(ofs);
	ofs.close();
	

//  	for_each (topK_shortest_paths.begin(), topK_shortest_paths.end(), 
//  		bind2nd(mem_fun(&CQYDirectedPath::PrintOut), (std::ostream&)cout));

	return 0;
}