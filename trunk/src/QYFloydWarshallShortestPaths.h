// QYFloydWarshallShortestPaths.h: interface for the CQYFloydWarshallShortestPaths class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QYFLOYDWARSHALLSHORTESTPATHS_H__05FE0F31_336A_4DA1_9676_F201AA250B79__INCLUDED_)
#define AFX_QYFLOYDWARSHALLSHORTESTPATHS_H__05FE0F31_336A_4DA1_9676_F201AA250B79__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "QYDirectedGraph.h"
#include "QYDirectedPath.h"

namespace asu_emit_qyan
{
	class CQYFloydWarshallShortestPaths  
	{
		typedef std::map<std::pair<int, int>, CQYDirectedPath*> PATH_OF_NODE_PAIRS_TYPE;
		typedef std::map<std::pair<int, int>, CQYDirectedPath*>::iterator PATH_OF_NODE_PAIRS_ITERATOR_TYPE;
		
		PATH_OF_NODE_PAIRS_TYPE m_allPairsOfShortestPaths;
		
		int m_nVerticesNumber;
		CQYDirectedGraph& m_rGraph;
		
	public:
		CQYFloydWarshallShortestPaths(CQYDirectedGraph& pGraph);
		virtual ~CQYFloydWarshallShortestPaths();
		
		bool Execute();
		
		CQYDirectedPath* GetShortestPath(int i, int j) {return m_allPairsOfShortestPaths[std::pair<int, int>(i,j)]; }
		void PrintOut(std::ostream& os);
		
	private:
		void _Init();
		
	};
}

#endif // !defined(AFX_QYFLOYDWARSHALLSHORTESTPATHS_H__05FE0F31_336A_4DA1_9676_F201AA250B79__INCLUDED_)
