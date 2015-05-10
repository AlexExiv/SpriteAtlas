#ifndef __FCONSOLE_H__
#define __FCONSOLE_H__


#include "types.h"

class FConsole
{
public:
	virtual ~FConsole(){}

	virtual void PutError( const CHAR_ * lpFormat, ... ) = 0;
	virtual void PutMessage( const CHAR_ * lpFormat, ... ) = 0;

	static FConsole * GetConsoleInst();
};

#define PUT_ERROR FConsole::GetConsoleInst()->PutError
#define PUT_MESSAGE FConsole::GetConsoleInst()->PutMessage

#endif