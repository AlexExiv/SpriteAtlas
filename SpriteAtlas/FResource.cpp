#include <stdlib.h>
#include "FResource.h"
#include "FFile.h"
#include "FBMPResource.h"



FResource::FResource( void * lpData, const FString & sExtStr ) : lpData( lpData ), bRemoveData( true ), sExtStr( sExtStr )
{
}

FResource::FResource( const FString & sExtStr, const FString & sResClass ) : sExtStr( sExtStr ), sResClass( sResClass )
{

}

FResource::~FResource()
{
	if( lpData && bRemoveData )
		FFree( lpData );
}

const FString & FResource::GetExt()const
{
	return sExtStr;
}

void * FResource::GetData()const
{
	return lpData;
}

bool FResource::IsExtEqual( const FString & sExtStr0 )
{
	return sExtStr == sExtStr0;
}
