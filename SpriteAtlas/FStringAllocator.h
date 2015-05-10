#ifndef __FSTRING_ALLOCATOR_H__
#define __FSTRING_ALLOCATOR_H__


#include "FString.h"

class FFile;

class FStringAllocator
{
	CHAR_ * lpStringPool;
	UI32 iPoolSize, iCurAddr;

public:
	FStringAllocator( UI32 iPoolSize );
	~FStringAllocator();

	void Clear();

	CHAR_ * AllocateStr( const FString & sStr );
	UI32 WriteToFile( FFile * lpFile );
	UI32 GetOffset( const CHAR_ * lpAddr )const;

	static FStringAllocator * GetAllocatorInst();
};

#endif