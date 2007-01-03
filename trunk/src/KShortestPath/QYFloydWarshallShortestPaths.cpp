// QYFloydWarshallShortestPaths.cpp: implementation of the CQYFloydWarshallShortestPaths class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include <vector>
#include "QYFloydWarshallShortestPaths.h"

namespace asu_emit_qyan
{
	using namespace std;
	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////

	CQYFloydWarshallShortestPaths::CQYFloydWarshallShortestPaths( CQYDirectedGraph& pGraph )
		: m_rGraph(pGraph)
	{
		_Init();
	}

	CQYFloydWarshallShortestPaths::~CQYFloydWarshallShortestPaths()
	{
		for (PATH_OF_NODE_PAIRS_ITERATOR_TYPE pos=m_allPairsOfShortestPaths.begin(); pos!=m_allPairsOfShortestPaths.end(); ++pos)
		{
			if (pos->second != NULL)
			{
				delete pos->second;
			}
		}
	}

	bool CQYFloydWarshallShortestPaths::Execute()
	{
		bool ret = false;

		int i,j,k;
		// Initiate the data structure for the result
		for (i=0; i<m_nVerticesNumber; ++i)
		{
			for (j=0; j<m_nVerticesNumber; ++j)
			{
				if (m_rGraph.GetWeight(i,j) < CQYDirectedGraph::DISCONNECT)
				{
					CQYDirectedPath* new_path = new CQYDirectedPath();
					new_path->SetCost(m_rGraph.GetWeight(i,j));
					new_path->SetSourceNodeId(i);
					new_path->SetTerminalNodeId(j);
					vector<int> path_list;
					path_list.push_back(i);
					path_list.push_back(j);
					//path_list.push_back(j);
					//path_list.push_back(i);
					new_path->SetVertexList(path_list);
					//
					m_allPairsOfShortestPaths.insert(pair<pair<int, int>, CQYDirectedPath*>(pair<int, int>(i,j), new_path));
				}
			}
		}
		
		// Run the algorithm to find all shortest paths of each pair of vertices.
		for (k=0; k<m_nVerticesNumber; k++)
		{
			for (i=0; i<m_nVerticesNumber; i++)
			{
				for (j=0; j<m_nVerticesNumber; j++)
				{
					if (!m_allPairsOfShortestPaths.count(pair<int,int>(i,k)) 
						|| !m_allPairsOfShortestPaths.count(pair<int,int>(k,j))
						|| !m_allPairsOfShortestPaths.count(pair<int,int>(i,j)))
					{
						continue;
					}

					double dist_i_k = m_allPairsOfShortestPaths[pair<int,int>(i,k)]->GetCost();
					double dist_k_j = m_allPairsOfShortestPaths[pair<int,int>(k,j)]->GetCost();
					double dist_i_j = m_allPairsOfShortestPaths[pair<int,int>(i,j)]->GetCost();

					if ( dist_i_j > dist_i_k + dist_k_j )
					{
						CQYDirectedPath* mod_path = m_allPairsOfShortestPaths[pair<int, int>(i,j)];					
						mod_path->SetCost(dist_i_k+dist_k_j);
						//
						vector<int> sub_path_list_1 = m_allPairsOfShortestPaths[pair<int, int>(i,k)]->GetVertexList();
						vector<int> sub_path_list_2 = m_allPairsOfShortestPaths[pair<int, int>(k,j)]->GetVertexList();
						vector<int> new_path_list(sub_path_list_1);
						int length_of_list_2 = sub_path_list_2.size();
						for (int l=1; l<length_of_list_2; ++l)
						{
							new_path_list.push_back(sub_path_list_2.at(l));
						}
						//
						mod_path->SetVertexList(new_path_list);
					}
				}
			}
		} 
		return ret;
	}


	////////////////////////////////////////////////////////////////////////
	// Private methods in the class
	////////////////////////////////////////////////////////////////////////

	void CQYFloydWarshallShortestPaths::_Init()
	{
		m_nVerticesNumber = m_rGraph.GetNumberOfVertices();	
	}

	void CQYFloydWarshallShortestPaths::PrintOut( std::ostream& os )
	{
		for (PATH_OF_NODE_PAIRS_ITERATOR_TYPE pos=m_allPairsOfShortestPaths.begin(); pos!=m_allPairsOfShortestPaths.end(); ++pos)
		{
			int source_id = pos->first.first;
			int terminal_id = pos->first.second;
			CQYDirectedPath* cur_path = pos->second;
			cur_path->PrintOut(os);
		}
	}

}
