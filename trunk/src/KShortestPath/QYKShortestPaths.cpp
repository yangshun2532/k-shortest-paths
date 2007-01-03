// ____________________________________________________________________________
//
//  General Information:
//
//  File Name:      QYKShortestPaths.cpp
//  Author:         Yan Qi
//  Project:        KShortestPath
//
//  Description:    Implementation of class(es) CQYKShortestPaths
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Revision History:
//
//  11/23/2006   Yan   Initial Version
//
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  Copyright Notice:
//
//  Copyright (c) 2006 Your Company Inc.
//
//  Warning: This computer program is protected by copyright law and 
//  international treaties.  Unauthorized reproduction or distribution
//  of this program, or any portion of it, may result in severe civil and
//  criminal penalties, and will be prosecuted to the maximum extent 
//  possible under the law.
//
// ____________________________________________________________________________
#pragma warning(disable: 4786)
#include "QYKShortestPaths.h"

namespace asu_emit_qyan
{
	using namespace std;
	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////
	
	CQYKShortestPaths::CQYKShortestPaths( const CQYDirectedGraph& rGraph, int nSource, int nTerminal, int nTopk )
		: m_rGraph(rGraph), m_nSourceNodeId(nSource), m_nTargetNodeId(nTerminal), m_nTopK(nTopk)
	{
		m_pIntermediateGraph = NULL;
		m_pShortestPath4IntermediateGraph = NULL;
	}
	
	CQYKShortestPaths::~CQYKShortestPaths()
	{
		for (std::vector<CQYDirectedPath*>::iterator pos=m_vTopShortestPaths.begin(); pos!=m_vTopShortestPaths.end(); ++pos)
		{
			delete *pos;
		}
		//
		if (m_pShortestPath4IntermediateGraph != NULL)
		{
			delete m_pShortestPath4IntermediateGraph;
		}
	}
	
	
	void CQYKShortestPaths::_SearchTopKShortestPaths()
	{
		//////////////////////////////////////////////////////////////////////////
		// first, find the shortest path in the graph
		m_pShortestPath4IntermediateGraph = new CQYShortestPath(m_rGraph);
		CQYDirectedPath* the_shortest_path = m_pShortestPath4IntermediateGraph->GetShortestPath(m_nSourceNodeId, m_nTargetNodeId);
		if (the_shortest_path->GetLength() == 0)
		{
			return;
		}
		the_shortest_path->SetId(0);

		m_candidatePathsSet.insert(the_shortest_path);
		m_pathDeviatedNodeMap.insert(pair<int, int>(0, m_nSourceNodeId));

		//////////////////////////////////////////////////////////////////////////
		// second, start to find the other results
		
		int cur_path_id = 0;
		while (m_candidatePathsSet.size() != 0 && cur_path_id < m_nTopK)
		{
			// Initiate the data
			CQYDirectedPath* cur_path = (*m_candidatePathsSet.begin());
			m_candidatePathsSet.erase(m_candidatePathsSet.begin());
			
			m_vTopShortestPaths.push_back(cur_path);
			++cur_path_id;
			
			//
			int deviated_node_id = m_pathDeviatedNodeMap[cur_path->GetId()];
			vector<int> node_list_in_path = cur_path->GetVertexList();
			
			// Construct the intermediate graph used to determine the next shortest paths 
			m_pIntermediateGraph = new CQYDirectedGraph(m_rGraph);
			
			// Determine the costs of nodes in the graph
			_DetermineCost2Target(node_list_in_path, deviated_node_id);
			
			// Iterations for the restoration of nodes and edges
			int path_length = node_list_in_path.size();
			for (int i=path_length-2; i>=0 && node_list_in_path.at(i) != deviated_node_id; --i)
			{
				_Restore4CostAjustment(node_list_in_path, node_list_in_path.at(i), node_list_in_path.at(i+1));
			}

			// Call _Restore4CostAjustment again for the deviated_node
			_Restore4CostAjustment(node_list_in_path, deviated_node_id, node_list_in_path.at(i+1), true);
			
			delete m_pIntermediateGraph;
		}
	}
	
	void CQYKShortestPaths::_DetermineCost2Target(vector<int> vertices_list, int deviated_node_id)
	{
		// first: generate a temporary graph with only parts of the original graph
		int count4vertices = m_pIntermediateGraph->GetNumberOfVertices();

		/// remove edges according to the algorithm
		int count = vertices_list.size();
		for (int i=0; i<count-1; ++i) // i<count-1: because the final element (i.e, the terminal) should be kept. 
		{
			int remove_node_id = vertices_list.at(i);
			for (int j=0; j<count4vertices; ++j)
			{
				int cur_edges_count = m_pIntermediateGraph->GetNumberOfEdges();
				if (m_pIntermediateGraph->GetWeight(remove_node_id, j) < CQYDirectedGraph::DISCONNECT)
				{
					m_pIntermediateGraph->SetWeight(remove_node_id, j, CQYDirectedGraph::DISCONNECT);
					--cur_edges_count;
				}
				m_pIntermediateGraph->SetNumberOfEdges(cur_edges_count);
			}
		}

		/// reverse the direction of edges in the temporary graph
		_ReverseEdgesInGraph(*m_pIntermediateGraph);
		
		// second: run the shortest paths algorithm, but with the target as m_nSource.
		/// run the shortest paths algorithm to get the cost of each nodes in the rest of the graph
		if (m_pShortestPath4IntermediateGraph != NULL)
		{
			delete m_pShortestPath4IntermediateGraph;
		}
		m_pShortestPath4IntermediateGraph = new CQYShortestPath(*m_pIntermediateGraph);
		m_pShortestPath4IntermediateGraph->ConstructPathTree(m_nTargetNodeId);
		
		// third: reverse the edges in the graph, restore the original status of the graph
		_ReverseEdgesInGraph(*m_pIntermediateGraph);
	}

	void CQYKShortestPaths::_Restore4CostAjustment(vector<int> vertices_list, int start_node_id, int end_node_id, bool is_deviated_node)
	{
		// first: restore the arcs from 'start_node_id' except that reaches 'end_node_id';
		/// restore the arcs and recalculate the cost of relative nodes
		int count4vertices = m_pIntermediateGraph->GetNumberOfVertices();
		for (int i=0; i<count4vertices; ++i)
		{
			double edge_weight = m_rGraph.GetWeight(start_node_id, i);
			double node_cost = m_pShortestPath4IntermediateGraph->GetDistance(i);
			if (edge_weight < CQYDirectedGraph::DISCONNECT && i!=end_node_id && (is_deviated_node ? !_EdgeHasBeenUsed(start_node_id, i) : true)) ///????
			{
				// restore the edge from start_node_id to i;
				m_pIntermediateGraph->SetWeight(start_node_id, i, edge_weight);
				//
				if ( node_cost < CQYDirectedGraph::DISCONNECT && edge_weight+node_cost < m_pShortestPath4IntermediateGraph->GetDistance(start_node_id))
				{
					m_pShortestPath4IntermediateGraph->SetDistance(start_node_id, edge_weight+node_cost);
					m_pShortestPath4IntermediateGraph->SetNextNodeId(start_node_id, i);
				}
			}
		}

		/// if possible, correct the labels and update the paths pool
		double cost_of_start_node = m_pShortestPath4IntermediateGraph->GetDistance(start_node_id);
		list<CQYDirectedGraph*> intermediate_path_list;
		if ( cost_of_start_node < CQYDirectedGraph::DISCONNECT)
		{
			_Update4CostUntilNode(start_node_id);

			//// construct the new path into result vector.
			int i;
			int path_length = vertices_list.size();
			vector<int> new_path; // the next shortest path: the order of nodes is from the source to the terminal.
			for (i=0; vertices_list.at(i) != start_node_id; ++i)
			{
				new_path.push_back(vertices_list.at(i));
			}
			// stop is the cost of the new path is too large, it's required that its cost before deviated node is small enougth. 
			int next_node_id = start_node_id;
			do 
			{
				new_path.push_back(next_node_id);
				next_node_id = m_pShortestPath4IntermediateGraph->GetNextNodeId(next_node_id);

			} while(next_node_id != m_nTargetNodeId);
			new_path.push_back(m_nTargetNodeId);
			
			// calculate the cost of the new path
			double cost_new_path = 0;
			int length_new_path = new_path.size();
			for (i=0; i<length_new_path-1; ++i)
			{
				cost_new_path += m_rGraph.GetWeight(new_path.at(i), new_path.at(1+i));
			}

			// Update the list of order the shortest paths
			int new_node_id = m_candidatePathsSet.size() + m_vTopShortestPaths.size();
			m_candidatePathsSet.insert(new CQYDirectedPath(new_node_id, cost_new_path, new_path));
			m_pathDeviatedNodeMap.insert(pair<int, int>(new_node_id, start_node_id));
		}

		// second: restore the arc from 'start_node_id' to 'end_node_id';
		double edge_weight = m_rGraph.GetWeight(start_node_id, end_node_id);
		double cost_of_end_node = m_pShortestPath4IntermediateGraph->GetDistance(end_node_id);

		m_pIntermediateGraph->SetWeight(start_node_id, end_node_id, edge_weight);

		if (cost_of_start_node > edge_weight+cost_of_end_node)
		{
			m_pShortestPath4IntermediateGraph->SetDistance(start_node_id, edge_weight+cost_of_end_node);
			m_pShortestPath4IntermediateGraph->SetNextNodeId(start_node_id, end_node_id);
			//
			_Update4CostUntilNode(start_node_id);
		}
	}

	void CQYKShortestPaths::_Update4CostUntilNode(int node_id)
	{
		int count4vertices = m_pIntermediateGraph->GetNumberOfVertices();
		std::vector<int> candidate_node_list;
		int cur_pos = 0;
		candidate_node_list.push_back(node_id);

		do 
		{
			int cur_node_id = candidate_node_list.at(cur_pos++);
			
			for (int i=0; i<count4vertices; ++i)
			{
				double edge_weight = m_pIntermediateGraph->GetWeight(i, cur_node_id);
				double cost_node = m_pShortestPath4IntermediateGraph->GetDistance(i);
				double cost_cur_node = m_pShortestPath4IntermediateGraph->GetDistance(cur_node_id);
				if (edge_weight < CQYDirectedGraph::DISCONNECT	&& cost_node > cost_cur_node + edge_weight)
				{
					m_pShortestPath4IntermediateGraph->SetDistance(i, cost_cur_node+edge_weight);
					m_pShortestPath4IntermediateGraph->SetNextNodeId(i, cur_node_id);
					//
					if(std::find(candidate_node_list.begin(), candidate_node_list.end(), i) == candidate_node_list.end())
					{
						candidate_node_list.push_back(i);
					}
				}
			}
		} while(cur_pos < candidate_node_list.size());
		
	}

	void CQYKShortestPaths::_ReverseEdgesInGraph( CQYDirectedGraph& g )
	{
		int i;
		int count4vertices = g.GetNumberOfVertices();
		for (i=0; i<count4vertices; ++i)
		{
			for (int j=0; j<i; ++j)
			{
				if(g.GetWeight(i,j) < CQYDirectedGraph::DISCONNECT 
					|| g.GetWeight(j,i) < CQYDirectedGraph::DISCONNECT )
				{
					double dTmp = g.GetWeight(i,j);
					g.SetWeight(i, j, g.GetWeight(j,i));
					g.SetWeight(j, i, dTmp);
				}
			}
		}
	}

	bool CQYKShortestPaths::_EdgeHasBeenUsed( int start_node_id, int end_node_id )
	{
		int count_of_shortest_paths = m_vTopShortestPaths.size();
		for (int i=0; i<count_of_shortest_paths; ++i)
		{
			CQYDirectedPath* cur_shortest_path = m_vTopShortestPaths.at(i);
			vector<int> cur_path_list = cur_shortest_path->GetVertexList();
			vector<int>::iterator loc_of_start_id = std::find(cur_path_list.begin(), cur_path_list.end(), start_node_id);
			if (loc_of_start_id == cur_path_list.end())
			{
				continue;
			}else
			{
				++loc_of_start_id;
				if (*loc_of_start_id == end_node_id)
				{
					return true;
				}
			}
		}
		return false;
	}
	
	vector<CQYDirectedPath*> CQYKShortestPaths::GetTopKShortestPaths()
	{
		_SearchTopKShortestPaths();
		return m_vTopShortestPaths;
	}
} // namespace