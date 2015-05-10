#ifndef __FFILE_H__
#define __FFILE_H__

#include "FString.h"


class FFile
{
public:
	enum
	{
		FILE_OPEN_ERROR = 0x1,
		FILE_READ = 0x2,
		FILE_CREATE = 0x4
	};

protected:
	I32 iFlags;
	FString sFileName;

	virtual I32 Open();
	virtual void Close();

public:
	FFile( const FString & sName, I32 iOpenFlags );
	virtual ~FFile();

	bool OpenSucc()const;
	virtual UI32 Read( void * lpData, UI32 iBytes2Read );
	virtual UI32 Write( void * lpData, UI32 iBytes2Write );
	
	virtual void * GetData();
	virtual UI32 GetSize()const;
	virtual void SetPointer( UI32 iPoint );
	virtual void Delete() = 0;

	static FFile * MakeFile( const FString & sName, I32 iOpenFlags );
	static void ReleaseFile( FFile * lpFile );
};

#endif