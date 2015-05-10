#include "FFile.h"
#include "FOsFile.h"

I32 FFile::Open()
{
	return -1;
}

void FFile::Close()
{
}

FFile::FFile( const FString & sName, I32 iOpenFlags ): sFileName( sName ), iFlags( iOpenFlags )
{
	//if( this->Open() == -1 )
	//	iFlags |= FILE_OPEN_ERROR;
}

FFile::~FFile()
{
	Close();
}

bool FFile::OpenSucc()const
{
	return !(iFlags & FILE_OPEN_ERROR);
}

UI32 FFile::Read( void * lpData, UI32 iBytes2Read )
{
	return 0;
}

UI32 FFile::Write( void * lpData, UI32 iBytes2Read )
{
	return 0;
}

void * FFile::GetData()
{
	return NULL;
}

UI32 FFile::GetSize()const
{
	return 0;
}

void FFile::SetPointer( UI32 iPoint )
{
	iPoint;
}

FFile * FFile::MakeFile( const FString & sName, I32 iOpenFlags )
{
	if( sName.Find( ".rar" ) == -1 )
		return NULL;
	return new FOsFile( sName, iOpenFlags );
}

void FFile::ReleaseFile( FFile * lpFile )
{
	delete lpFile;
}
