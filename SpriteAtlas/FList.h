#ifndef __FLIST_H__
#define __FLIST_H__

#include "types.h"

template <class T>
class FList
{
	struct Node
	{
		Node * lpPrev, * lpNext;
		T tData;
	};

	Node * lpNodeList;
	I32 iCount;

public:
	class Iterator
	{
		Node * lpCurNode;
	public:
		Iterator() : lpCurNode(NULL){}
		Iterator( const Iterator & iIt ) : lpCurNode( iIt.lpCurNode ){}
		Iterator( Node * lpNode ) : lpCurNode( lpNode ) {}

		Iterator & operator = ( const Iterator & iIt )
		{
			lpCurNode = iIt.lpCurNode;
			return *this;
		}

		T * operator -> ()
		{
			return &lpCurNode->tData;
		}

		operator T * ()
		{
			return &lpCurNode->tData;
		}

		Iterator & operator ++ ()
		{
			if( lpCurNode )
				lpCurNode = lpCurNode->lpNext;
			return *this;
		}

		Iterator & operator -- ()
		{
			if( lpCurNode )
				lpCurNode = lpCurNode->lpPrev;
			return *this;
		}

		Iterator operator ++ (I32)
		{
			Iterator iIt = *this;

			if( lpCurNode )
				lpCurNode = lpCurNode->lpNext;
			return iIt;
		}

		Iterator operator -- (I32)
		{
			Iterator iIt = *this;

			if( lpCurNode )
				lpCurNode = lpCurNode->lpPrev;
			return iIt;
		}

		bool operator == ( const Iterator & iIt )
		{
			return lpCurNode == iIt.lpCurNode;
		}

		bool operator != ( const Iterator & iIt )
		{
			return lpCurNode != iIt.lpCurNode;
		}

		Node * GetNode()
		{
			return lpCurNode;
		}
	};

	friend class Iterator;

	FList() : lpNodeList( NULL ), iCount( 0 )
	{
		lpNodeList = new Node;
		lpNodeList->lpNext = lpNodeList;
		lpNodeList->lpPrev = lpNodeList;
	}
	FList( const FList<T> & lList ) : iCount( 0 )
	{
		lpNodeList = new Node;
		lpNodeList->lpNext = lpNodeList;
		lpNodeList->lpPrev = lpNodeList;

		for( Iterator iIt = lList.Begin();iIt != lList.End();iIt++ )
			PushBack( *iIt );
	}
	~FList()
	{
		Clear();
		delete lpNodeList;
	}
	FList & operator = ( const FList<T> & lList )
	{
		Clear();
		for( Iterator iIt = lList.Begin();iIt != lList.End();iIt++ )
			PushBack( *iIt );
		return *this;
	}

	void Clear()
	{
		Iterator iIt = Begin();
		while( iIt != End() )
			iIt = Erase( iIt );
		iCount = 0;
	}

	Iterator Begin()
	{
		return Iterator( lpNodeList->lpNext );
	}

	Iterator Begin()const
	{
		return Iterator( lpNodeList->lpNext );
	}

	Iterator End()
	{
		return Iterator( lpNodeList );
	}

	Iterator End()const
	{
		return Iterator( lpNodeList );
	}

	Iterator First()
	{
		return Begin();
	}

	Iterator First()const
	{
		return Begin();
	}

	Iterator Last()
	{
		return Iterator( lpNodeList->lpPrev );
	}

	Iterator Last()const
	{
		return Iterator( lpNodeList->lpPrev );
	}

	I32 GetCount()const
	{
		return iCount;
	}

	bool IsEmpy()const
	{
		return iCount == 0;
	}

	void Insert( const T & tData, Iterator iIt )
	{
		Node * lpNode = iIt.GetNode();
		Node * lpNewNode = new Node;
		lpNewNode->tData = tData;
		Node * lpTemp = lpNode->lpNext;
		lpNode->lpNext = lpNewNode;
		lpNewNode->lpPrev = lpNode;
		lpNewNode->lpNext = lpTemp;
		lpTemp->lpPrev = lpNewNode;
		iCount++;
	}

	Iterator Erase( Iterator iIt )
	{
		Node * lpNode = iIt.GetNode();
		lpNode->lpPrev->lpNext = lpNode->lpNext;
		lpNode->lpNext->lpPrev = lpNode->lpPrev;
		Node * lpNext = lpNode->lpNext;

		delete lpNode;
		iCount--;

		return Iterator( lpNext );
	}

	void PushBack( const T & tData )
	{
		Insert( tData, --End() );
	}

	void PushFront( const T & tData )
	{
		Insert( tData, --Begin() );
	}

	void PopBack()
	{
		Erase( --End() );
	}

	void PopFront()
	{
		Erase( Begin() );
	}
};

#endif