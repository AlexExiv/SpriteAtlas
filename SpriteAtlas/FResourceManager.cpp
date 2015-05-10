#include "FResource.h"
#include "FResourceManager.h"
#include "FBMPResource.h"
#include "FTGAResource.h"
#include "FJPEGResource.h"
#include "FPNGResource.h"
#include "FFile.h"
#include "FConsole.h"


FResourceManager * FResourceManager::lpManager = NULL;

FResourceManager::FResourceManager()
{
	RegisterResource( new FBMPResource() );
	RegisterResource( new FTGAResource() );
	RegisterResource( new FJPEGResource() );
	RegisterResource( new FPNGResource() );
}

FResourceManager::FResourceManager( const FResourceManager & )
{
}

FResourceManager & FResourceManager::operator = ( const FResourceManager& )
{
	return *this;
}

void FResourceManager::RegisterResource( FResource * lpResource )
{
	FResourceIterator iIt = lResources.Begin();
	for(;iIt != lResources.End();iIt++ )
	{
		if( (*iIt)->IsExtEqual( lpResource->sExtStr ) )
			return;
	}

	lResources.PushBack( lpResource );
}

FResourceManager::~FResourceManager()
{
	FResourceIterator iIt = lResources.Begin();
	for(;iIt != lResources.End();iIt++ )
		delete *iIt;
}

FResourceManager * FResourceManager::SharedManager()
{
	if( !lpManager )
		lpManager = new FResourceManager();
	return lpManager;
}

FResource * FResourceManager::CreateResource( const FString & sFileName )
{
	FFile * lpFile = FFile::MakeFile( sFileName, FFile::FILE_READ );
	void * lpData = NULL;
	if( !lpFile->OpenSucc() )
	{
		PUT_ERROR( "Can't find file with name: %s", sFileName.GetChar() );
		return NULL;
	}
	lpData = lpFile->GetData();

	FResource * lpRes = NULL;
	FResourceIterator iIt = lResources.Begin();
	FString sFileExt = sFileName.GetExt();
	for(;iIt != lResources.End();iIt++ )
	{
		if( (*iIt)->IsExtEqual( sFileExt ) )
		{
			lpRes = (*iIt)->Make( lpData, lpFile->GetSize() );
			break;
		}
	}

	if( iIt == lResources.End() )
		PUT_ERROR( "Unknown file type with name: %s", sFileName.GetChar() );

	FFree( lpData );
	FFile::ReleaseFile( lpFile );

	return lpRes;
}

void FResourceManager::SaveResource( const FString & sFileName, FResource * lpRes )
{
	FString sFullName = sFileName + "." + lpRes->sExtStr;
	void * lpBuffer;
	UI32 iDataSize;
	lpRes->SaveResource( &lpBuffer, iDataSize );

	FFile * lpFile = FFile::MakeFile( sFullName, FFile::FILE_CREATE );
	if( lpFile->OpenSucc() )
		lpFile->Write( lpBuffer, iDataSize );
	else
		PUT_ERROR( "Can't create file \"%s\"", sFullName.GetChar() );

	FFile::ReleaseFile( lpFile );
	FFree( lpBuffer );
}

void FResourceManager::SaveResource( void ** lpBuffer, UI32 & iWritedLen, FResource * lpRes )
{
	lpRes->SaveResource( lpBuffer, iWritedLen );
}
