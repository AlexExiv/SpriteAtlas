#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "FAtlasBuilder.h"
#include "FConsole.h"
#include "FStack.h"

const FString sHelpComm( "-help" );
const FString sExitComm( "-exit" );

void main()
{
	FStack::Init();
	CHAR_ cBuffer[MAX_BUFFER];
	FAtlasBuilder aBuilder;

	std::cout << "Max length of input information is 255 symbols" << std::endl;
	std::cout << "To view help information input " << sHelpComm.GetChar() << " command" << std::endl;
	std::cout << "To exit from programm input " << sExitComm.GetChar() << " command" << std::endl;
	
	while( true )
	{
		std::cout << "Name of config file: ";
		std::cin >> cBuffer;
		std::cout << std::endl;

		if( cBuffer[0] == '-' )
		{
			FString sComm = cBuffer;

			if( sComm == sHelpComm )
			{
				CHAR_ * lpBuffer = (CHAR_ *)FStack::GetInstanceStack()->PushBlock( 4096*sizeof( CHAR_ ) );
				aBuilder.GetHelpInfo( lpBuffer );
				std::cout << lpBuffer << std::endl;
				FStack::GetInstanceStack()->PopBlock();
			}
			else if( sComm == sExitComm )
				break;

			continue;
		}

		if( !aBuilder.BuildAtlas( cBuffer ) )
			continue;

		if( aBuilder.IsPacked() )
		{
			std::cout << "Do you want to edit alpha data(y/n):  ";
			std::cin >> cBuffer;
			std::cout << std::endl;

			if( cBuffer[0] == 'y' )
			{
				aBuilder.EditAlphaStart();
				std::cout << "Press any key when end edit alpha to continue";
				std::cin >> cBuffer;
				std::cout << std::endl;
				if( !aBuilder.EditAlphaDone() )
					continue;
			}
		}

		aBuilder.SaveAtlas();
	}

	FStack::Deinit();
}