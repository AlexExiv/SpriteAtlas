#include "FConsole.h"
#include <stdarg.h>


static FConsole * lpConsole = NULL;


class FSIOConsole : public FConsole
{
public:
	FSIOConsole()
	{
	}

	~FSIOConsole()
	{
	}

	void PutError( const CHAR_ * lpFormat, ... )
	{
		va_list lpArgs;
		va_start( lpArgs, lpFormat );

		printf( "Error: " );
		vprintf( lpFormat, lpArgs );
		printf( "\n" );

		va_end( lpArgs );
	}

	void PutMessage( const CHAR_ * lpFormat, ... )
	{
		va_list lpArgs;
		va_start( lpArgs, lpFormat );

		printf( "Message: " );
		vprintf( lpFormat, lpArgs );
		printf( "\n" );

		va_end( lpArgs );
	}

};


FConsole * FConsole::GetConsoleInst()
{
	if( !lpConsole )
		lpConsole = new FSIOConsole();
	return lpConsole;
}