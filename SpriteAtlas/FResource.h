#ifndef __FRESOURCE_H__
#define __FRESOURCE_H__

#include "FString.h"

class FResourceManager;

class FResource
{
	friend class FResourceManager;

	FString sExtStr, sResClass;
protected:
	void * lpData;
	bool bRemoveData;

	virtual FResource * Make( void * lpData, UI32 iDataLen ) = 0;
	virtual void SaveResource( void ** lpData, UI32 & iDataSize ) = 0;

	bool IsExtEqual( const FString & sExtStr );
	FResource( void * lpData, const FString & sExtStr );
	FResource( const FString & sExtStr, const FString & sResClass );

public:
	virtual ~FResource();

	const FString & GetExt()const;
	void * GetData()const;
};

#endif