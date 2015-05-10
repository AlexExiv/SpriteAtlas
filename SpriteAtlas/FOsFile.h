#ifndef __FOS_FILE_H__
#define __FOS_FILE_H__

#include <windows.h>
#include "FFile.h"

class FOsFile : public FFile
{
	HANDLE hFile;
protected:
	virtual I32 Open();
	virtual void Close();

public:
	FOsFile( const FString & sName, I32 iOpenFlags );
	~FOsFile();

	UI32 Read( void * lpData, UI32 iBytes2Read );
	UI32 Write( void * lpData, UI32 iBytes2Write );
	
	void * GetData();
	UI32 GetSize()const;
	void SetPointer( UI32 iPoint );
	void Delete();
};

#endif