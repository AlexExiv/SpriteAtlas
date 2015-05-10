#ifndef __FRESOURCE_MANAGER_H__
#define __FRESOURCE_MANAGER_H__

#include "FList.h"

class FResource;
class FResourceManager
{
	typedef FList<FResource *> FResourcesList;
	typedef FList<FResource *>::Iterator FResourceIterator;

	static FResourceManager * lpManager;
	FResourcesList lResources;

	FResourceManager();
	FResourceManager( const FResourceManager & );
	FResourceManager & operator = ( const FResourceManager& );

	void RegisterResource( FResource * lpResource );

public:
	~FResourceManager();

	static FResourceManager * SharedManager();

	FResource * CreateResource( const FString & sFileName );
	void SaveResource( const FString & sFileName, FResource * lpRes );
	void SaveResource( void ** lpBuffer, UI32 & iWritedLen, FResource * lpRes );
};

#endif