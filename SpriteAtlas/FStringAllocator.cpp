#include "FStringAllocator.h"
#include <string.h>
#include "FFile.h"



static FStringAllocator * lpStringAllocator = NULL;
#define STR_ALLOC_POOL_SIZE 2000000l

FStringAllocator::FStringAllocator( UI32 iPoolSize ) : iPoolSize( iPoolSize ), iCurAddr( 0 ), lpStringPool( NULL )
{
	lpStringPool = (CHAR_ *)FMalloc( iPoolSize );
}

FStringAllocator::~FStringAllocator()
{
	if( lpStringPool )
		FFree( lpStringPool );
	lpStringPool = NULL;
}


void FStringAllocator::Clear()
{
	iCurAddr = 0;
}


CHAR_ * FStringAllocator::AllocateStr( const FString & sStr )
{
	if( (iCurAddr + sStr.Length()) >= iPoolSize )
		return NULL;

	CHAR_ * lpStr = lpStringPool + iCurAddr;
	iCurAddr += sStr.Length() + 1;
	strcpy( lpStr, sStr.GetChar() );

	return lpStr;
}

UI32 FStringAllocator::WriteToFile( FFile * lpFile )
{
	return lpFile->Write( lpStringPool, iCurAddr );
}

UI32 FStringAllocator::GetOffset( const CHAR_ * lpAddr )const
{
	return lpAddr - lpStringPool;
}


FStringAllocator * FStringAllocator::GetAllocatorInst()
{
	if( !lpStringAllocator )
		lpStringAllocator = new FStringAllocator( STR_ALLOC_POOL_SIZE );
	return lpStringAllocator;
}

