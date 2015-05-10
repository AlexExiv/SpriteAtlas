
#include "FOsFile.h"



I32 FOsFile::Open()
{
	if( iFlags & FILE_READ )
		hFile = ::CreateFile( sFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
									FILE_ATTRIBUTE_NORMAL, NULL );
	else if( iFlags & FILE_CREATE )
		hFile = ::CreateFile( sFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
									0, NULL );
	else
	{
		return -1;
	}

	if( hFile == INVALID_HANDLE_VALUE )
	{
		return -1;
	}
	return 1;
}

void FOsFile::Close()
{
	if( hFile && (hFile != INVALID_HANDLE_VALUE) )
	{
		CloseHandle( hFile );
		hFile = NULL;
	}
}

FOsFile::FOsFile( const FString & sName, I32 iOpenFlags ) : FFile( sName, iOpenFlags ), hFile( NULL )
{
	if( Open() == -1 )
		iFlags |= FILE_OPEN_ERROR;
}

FOsFile::~FOsFile()
{
	Close();
}

UI32 FOsFile::Read( void * lpData, UI32 iBytes2Read )
{
	if( !hFile )
		return 0;
	//AssertFatal( hFile, "Handle value is NULL can't read file" );
	DWORD iRead = 0;
	::ReadFile( hFile, lpData, iBytes2Read, &iRead, NULL );

	return iRead;
}

UI32 FOsFile::Write( void * lpData, UI32 iBytes2Write )
{
	if( !hFile )
		return 0;
	//AssertFatal( hFile, "Handle value is NULL can't write file" );
	DWORD iWrite = 0;
	::WriteFile( hFile, lpData, iBytes2Write, &iWrite, NULL );

	return iWrite;
}


void * FOsFile::GetData()
{
	void * lpData = FMalloc( GetSize() );
	SetPointer( 0 );
	Read( lpData, GetSize() );

	return lpData;
}

UI32 FOsFile::GetSize()const
{
	return GetFileSize( hFile, NULL );
}
void FOsFile::SetPointer( UI32 iPoint )
{
	SetFilePointer( hFile, iPoint, NULL, FILE_BEGIN );
}

void FOsFile::Delete()
{
	Close();
	DeleteFile( sFileName );
}