#include <string.h>
#include <stdlib.h>
#include "FString.h"
#include <stdarg.h>



#define MAX_STRING 32


static UI32 iCRCTable[256];
static bool bCRCReady = false;


inline UI32 Reflect( UI32 iRef, UI32 iCount )
{
	UI32 iValue = 0;
	for( I32 i = 1;i < (iCount + 1);i++ )
	{
		if( iRef & 1 )
			iValue |= 1 << (iCount - i);
		iRef >>= 1;
	}

	return iValue;
}

void InitTable()
{
	UI32 iPoly = 0x04C11DB7;
	for( I32 i = 0;i < 256;i++ )
	{
		iCRCTable[i] = Reflect( i, 8 ) << 24;
		for( I32 j = 0;j < 8;j++ )
			iCRCTable[i] = (iCRCTable[i] << 1) ^ (iCRCTable[i] & (1 << 31) ? iPoly : 0);
		iCRCTable[i] = Reflect( iCRCTable[i], 32 );
	}
}

UI32 GetCRC( const void * lpMessage, UI32 iLen )
{
	if( !bCRCReady )
	{
		InitTable();
		bCRCReady = true;
	}

	UI32 iCRC = 0xFFFFFFFF;
	UI8 * lpChar = (UI8 *)lpMessage;
	while( iLen-- )
		iCRC = (iCRC >> 8) ^ iCRCTable[(iCRC & 0xFF) ^ *lpChar++];

	return iCRC ^ 0xFFFFFFFF;
}



void FString::ComputeKey()
{
	iKey = 0;
	if( !lpString )
		return;

	iKey = GetCRC( lpString, Length() );
}

FString::FString() : lpString( NULL ), iLen( 0 )
{
	ComputeKey();
}

FString::FString( const CHAR_ * lpString0 ) : iLen( 0 )
{
	iLen = strlen( lpString0 );

	lpString = (CHAR_ *)FMalloc( (iLen + 1) * sizeof( CHAR_ ) );
	strcpy( lpString, lpString0 );

	ComputeKey();
}

FString::FString( CHAR_ cChar )
{
	iLen = 1;
	lpString = (CHAR_ *)FMalloc( 2 * sizeof( CHAR_ ) );
	lpString[0] = cChar;
	lpString[1] = 0;

	ComputeKey();
}

FString::FString( const FString & sString ) : lpString( NULL ), iKey( sString.iKey ), iLen( 0 )
{
	if( sString.lpString == NULL )
		return;

	iLen = sString.Length();

	lpString = (CHAR_ *)FMalloc( (iLen + 1) * sizeof( CHAR_ ) );
	strcpy( lpString, sString.GetChar() );
}

FString::FString( I32 iNum )
{
	CHAR_ cNum[MAX_STRING];

	itoa( iNum, cNum, 10 );
	iLen = strlen( cNum );

	lpString = (CHAR_ *)FMalloc( (iLen + 1) * sizeof( CHAR_ ) );
	strcpy( lpString, cNum );

	ComputeKey();
}

FString::FString( UI32 iNum )
{
	CHAR_ cNum[MAX_STRING];

	ultoa( iNum, cNum, 10 );
	iLen = strlen( cNum );

	lpString = (CHAR_ *)FMalloc( (iLen + 1) * sizeof( CHAR_ ) );
	strcpy( lpString, cNum );

	ComputeKey();
}

FString::FString( F32 fNum )
{
	CHAR_ cNum[MAX_STRING];

	gcvt( fNum, 10, cNum );
	iLen = strlen( cNum );

	lpString = (CHAR_ *)FMalloc( (iLen + 1) * sizeof( CHAR_ ) );
	strcpy( lpString, cNum );

	ComputeKey();
}

FString::~FString()
{
	if( lpString )
		FFree( lpString );
	lpString = NULL;
}

FString & FString::operator = ( const FString & sString0 )
{
	if( lpString )
	{
		FFree( lpString );
		lpString = NULL;
	}

	iKey = sString0.iKey;
	if( sString0.lpString == NULL )
		return *this;

	iLen = sString0.Length();

	lpString = (CHAR_ *)FMalloc( (iLen + 1) * sizeof( CHAR_ ) );
	strcpy( lpString, sString0.GetChar() );

	return *this;
}

FString & FString::operator = ( I32 iNum )
{
	CHAR_ cNum[MAX_STRING];

	itoa( iNum, cNum, 10 );
	iLen = strlen( cNum );

	if( lpString )
		FFree( lpString );

	lpString = (CHAR_ *)FMalloc( (iLen + 1) * sizeof( CHAR_ ) );
	strcpy( lpString, cNum );

	ComputeKey();
	return *this;
}
FString & FString::operator = ( F32 fNum )
{
	CHAR_ cNum[MAX_STRING];

	_gcvt( fNum, MAX_STRING, cNum );
	iLen = strlen( cNum );

	if( lpString )
		FFree( lpString );

	lpString = (CHAR_ *)FMalloc( (iLen + 1) * sizeof( CHAR_ ) );
	strcpy( lpString, cNum );

	ComputeKey();
	return *this;
}

bool FString::operator == ( const FString & sString )
{
	if( iKey != sString.iKey )
		return false;

	return (strcmp( lpString, sString.GetChar() ) == 0);
}

bool FString::operator != ( const FString & sString )
{
	if( iKey == sString.iKey )
		return false;

	return (strcmp( lpString, sString.GetChar() ) != 0);
}

bool FString::operator == ( const FString & sString )const
{
	if( iKey != sString.iKey )
		return false;

	return (strcmp( lpString, sString.GetChar() ) == 0);
}

bool FString::operator != ( const FString & sString )const
{
	if( iKey == sString.iKey )
		return false;

	return (strcmp( lpString, sString.GetChar() ) != 0);
}

FString & FString::operator += ( const FString & sString )
{
	if( (sString.Length() == 0) || (!sString.lpString) )
		return *this;

	iLen = Length() + sString.Length();
	CHAR_ * lpStrNew = (CHAR_ *)FMalloc( (iLen + 1) * sizeof( CHAR_ ) );
	if( lpString )
	{
		strcpy( lpStrNew, lpString );
		FFree( lpString );
		strcat( lpStrNew, sString.GetChar() );
		lpString = lpStrNew;
	}
	else
	{
		strcpy( lpStrNew, sString.GetChar() );
		lpString = lpStrNew;
	}

	ComputeKey();
	return *this;
}

FString & FString::operator += ( const CHAR_ * lpString0 )
{
	FString sString( lpString0 );

	if( (sString.Length() == 0) || (!lpString0) )
		return *this;

	iLen = Length() + sString.Length();
	CHAR_ * lpStrNew = (CHAR_ *)FMalloc( (iLen + 1) * sizeof( CHAR_ ) );
	if( lpString )
	{
		strcpy( lpStrNew, lpString );
		FFree( lpString );
		strcat( lpStrNew, sString.GetChar() );
		lpString = lpStrNew;
	}
	else
	{
		strcpy( lpStrNew, sString.GetChar() );
		lpString = lpStrNew;
	}


	ComputeKey();
	return *this;
}

FString & FString::operator -= ( const FString & sString )
{
	if( (sString.Length() == 0) || (!lpString) || (sString.Length() > Length()) )
		return *this;

	CHAR_ * lpSub = strstr( lpString, sString.GetChar() );
	if( lpSub == NULL )
		return *this;

	I32 iNum = lpSub - lpString;
	iLen = Length() - sString.Length();

	CHAR_ * lpStrNew = (CHAR_ *)FMalloc( (iLen + 1) * sizeof( CHAR_ ) );

	strncpy( lpStrNew, lpString, iNum );
	strcat( lpStrNew, &lpString[iNum + sString.Length()] );

	FFree( lpString );
	lpString = lpStrNew;


	ComputeKey();
	return *this;
}

FString & FString::operator += ( CHAR_ cChar )
{
	iLen = Length() + 1;
	CHAR_ * lpStrNew = (CHAR_ *)FMalloc( (iLen + 1) * sizeof( CHAR_ ) );
	if( lpString )
	{
		strcpy( lpStrNew, lpString );
		lpStrNew[iLen - 1] = cChar;
		lpStrNew[iLen] = 0;
		FFree( lpString );
		lpString = lpStrNew;
	}
	else
	{
		lpString = lpStrNew;
		lpString[0] = cChar;
		lpString[1] = 0;
	}


	ComputeKey();
	return *this;
}

FString::operator I32 ()
{
	return atoi( lpString );
}

FString::operator F32 ()
{
	return (F32)atof( lpString );
}

I32 FString::Find( const CHAR_ * lpSubStr )
{
	CHAR_ * lpStr = strstr( lpString, lpSubStr );
	if( !lpStr )
		return -1;
	return lpStr - lpString - 1;
}

I32 FString::Find( const FString & sSubStr ) const
{
	CHAR_ * lpStr = strstr( lpString, sSubStr.GetChar() );
	return lpStr - lpString - 1;
}

I32 FString::Length() const
{
	return iLen;
}

void FString::SetSize( UI32 iSize )
{
	if( lpString )
		FFree( lpString );
	lpString = (CHAR_ *)FMalloc( iSize*sizeof( CHAR_ ) );
	lpString[0] = 0;
	iLen = iSize;
}

FString FString::GetExt( bool bPoint )const
{
	CHAR_ cBuffer[256];
	CHAR_ * lpStr = lpString;

	I32 iLen = Length() - 1,
		i = iLen;

	for(;i != -1; i-- )
	{
		if( lpStr[i] == '.' )
			break;

		cBuffer[iLen - i] = lpStr[i];
	}
	if( bPoint )
	{
		cBuffer[iLen - i] = '.';
		i--;
	}

	cBuffer[iLen - i] = 0;

	return FString( cBuffer ).GetMirrored();
}

FString FString::GetName()const
{
	I32 iStart = 0, iEnd = 0;
	I32 i = Length() - 1;

	for( ;i != -1;i-- )
	{
		if( lpString[i] == '.' )
		{
			iEnd = i;
			break;
		}
	}
	for(;i != -1;i-- )
	{
		if( lpString[i] == '\\' )
		{
			iStart = i + 1;
			break;
		}
	}

	CHAR_ cBuffer[256];
	for( i = 0;i < (iEnd - iStart);i++ )
		cBuffer[i] = lpString[i + iStart];
	cBuffer[i] = 0;

	return FString( cBuffer );
}

FString FString::GetPath()const
{
	FString sPath( *this );
	I32 i = Length() - 1;
	for(;i != -1;i-- )
	{
		if( lpString[i] == '\\' )
			break;
	}
	sPath.SetChar( i + 1, 0 );

	return sPath;
}

FString FString::GetMirrored()const
{
	FString sNewStr = *this;
	sNewStr.Mirrored();
	return sNewStr;
}

void FString::Mirrored()
{
	I32 iLen = Length();

	for( I32 i = 0;i < iLen/2;i++ )
	{
		CHAR_ t = lpString[iLen - i - 1];
		lpString[iLen - i - 1] = lpString[i];
		lpString[i] = t;
	}
	ComputeKey();
}

UI32 FString::GetKey()const
{
	return iKey;
}

I32 SkipSpace( const CHAR_ * lpStr, I32 iStartInd )
{
	I32 iSkipLen = 0;
	while( lpStr[iStartInd] == ' ' || lpStr[iStartInd] != 0 )
	{
		iSkipLen++;
		iStartInd++;
	}

	return iSkipLen;
}

I32 ParseString( const CHAR_ * lpStr, FString * lpParsedStr, I32 iStartInd )
{
	I32 iLen;
	CHAR_ cBuffer[MAX_BUFFER];
	if( lpStr[iStartInd++] != '"' )
		return 0;
	while( lpStr[iStartInd] != '"' )
	{
		if( lpStr[iStartInd] == 0 )
			return iLen;

		cBuffer[iLen++] = lpStr[iStartInd++];
	}
	cBuffer[iLen] = 0;
	*lpParsedStr = cBuffer;

	return iLen;
}

I32 ParseInt( const CHAR_ * lpStr, I32 * lpInteger, I32 iStartInd )
{
	I32 iLen;
	CHAR_ cBuffer[MAX_BUFFER];

	while( (lpStr[iStartInd] != ' ') || (lpStr[iStartInd] != 0) )
		cBuffer[iLen++] = lpStr[iStartInd++];

	cBuffer[iLen] = 0;
	*lpInteger = atoi( cBuffer );

	return iLen;
}

I32 ParseFloat( const CHAR_ * lpStr, F32 * lpFloat, I32 iStartInd )
{
	I32 iLen;
	CHAR_ cBuffer[MAX_BUFFER];

	while( (lpStr[iStartInd] != ' ') || (lpStr[iStartInd] != 0) )
		cBuffer[iLen++] = lpStr[iStartInd++];

	cBuffer[iLen] = 0;
	*lpFloat = atof( cBuffer );

	return iLen;
}

void FString::ScanString( const CHAR_ * lpFormat, ... )
{
	va_list lpArgs = NULL;
	va_start( lpArgs, lpFormat );
	I32 iFCurSym = 0, iCurSym = 0;
	while( lpFormat[iFCurSym] != 0 )
	{
		if( lpFormat[iFCurSym++] == '%' )
		{
			void * lpArg = va_arg( lpArgs, void * );
			if( lpFormat[iFCurSym] == 0 )
				break;

			switch( lpFormat[iFCurSym++] )
			{
			case 'c':
				*(CHAR_ *)lpArg = lpString[iCurSym++];
				break;
			case 's':
				iCurSym += ParseString( lpString, (FString *)lpArg, iCurSym );
				break;
			case 'f':
				iCurSym += ParseFloat( lpString, (F32 *)lpArg, iCurSym );
				break;
			case 'i':
				iCurSym += ParseInt( lpString, (I32 *)lpArg, iCurSym );
				break;
			}

			iCurSym += SkipSpace( lpString, iCurSym );
		}
	}

	va_end( lpArgs );

}