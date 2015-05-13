#include "FAtlasConfig.h"
#include "FAtlasHeader.h"
#include "FFile.h"
#include "FStringAllocator.h"
#include "FList.h"
#include "FStack.h"
#include "FConsole.h"
#include <string.h>



/////имена полей в конфиг файле
const FString sWorkDir( "WORKDIRECTORY" );
const FString sMaxWidth( "MAX_WIDTH" );
const FString sCompr( "COMPRESS" );
const FString sPack( "PACK" );
const FString sMinAlpha( "MIN_ALPHA" );
const FString sMaxAlpha( "MAX_ALPHA" );
const FString sProdAlpha( "PRODUCE_ALPHA" );
const FString sBrightness( "BRIGHT_MIN" );
const FString sBackColor( "BACKCOLOR" );
const FString sBorderWidth( "BORDERWIDTH" );
const FString sJPEGQuality( "JPEG_QUALITY" );
const FString sSaveDir( "SAVEDIRECTORY" );


/////значения поумолчанию для полей
const FString sDeffWorkDir( "\\" );
const I32 iDeffWidth = 512;
const FString sDeffCompr( ATLAS_CONST_NO );
const FString sDeffPack( ATLAS_CONST_NO );
const I32 iMinAlphaDeff = 0;
const I32 iMaxAlphaDeff = 255;
const FString sProdAlphaDeff( ATLAS_CONST_AUTO );
const I32 iBrightMinDeff = 20;
const FString sBackColorDeff( "BLACK" );
const I32 iBorderWidthDeff = 2;
const I32 iJPEGQualityDeff = 50;
const FString sDeffSaveDir( "\\" );


/////возможные значения констант и возможные интервалы полей
const FString sComprConst [] = { ATLAS_CONST_NO, ATLAS_CONST_JPG, ATLAS_CONST_PNG, ATLAS_CONST_RLE };
const I32 iWidthRange[] = { 10, 16384 };
const FString sPackConst[] = { ATLAS_CONST_NO, ATLAS_CONST_YES };
const FString sProdAlphaConst[] = { ATLAS_CONST_NO, ATLAS_CONST_YES, ATLAS_CONST_AUTO };
const I32 iBrightMinRange[] = { 0, 255 };
const I32 iMinAlphaRange[] = { 0, 255 };
const I32 iMaxAlphaRange[] = { 0, 255 };
const FString sBackColorConst[] = { "BLACK", "WHITE", "RED", "GREEN", "BLUE", "PURPLE", "ORANGE" };
const I32 iBorderWidthRange [] = { 0, 10 };
const I32 iJPEGQualityRange [] = {0 , 100 };


const CHAR_ * lpWorkDirHelp = " - указывает на каталог в котором содержатся файлы кадров";
const CHAR_ * lpMaxWidthHelp = " - максимальная ширина итогогой текстуры атласа";
const CHAR_ * lpComprHelp = " - тип сжатия текстуры аталаса";
const CHAR_ * lpPackHelp = " - упаковка атласа и сопроводительных данных в один файл";
const CHAR_ * lpMinAlphaHelp = " - при генерации альфа канала это минимальное значение которое может быть";
const CHAR_ * lpMaxAlphaHelp = " - при генерации альфа канала это максимальное значение которое может быть";
const CHAR_ * lpProdAlphaHelp = " - генерация альфа канала";
const CHAR_ * lpBrightHelp = " - параметр указывающий степень яркости для генерации альфа канала";
const CHAR_ * lpBackColorHelp = " - цвет заднего фона";
const CHAR_ * lpBorderWidthHelp = " - ширина границы между кадрами в текстуре аталаса";
const CHAR_ * lpJPEGQualityHelp = " - качество сжатия при jpeg сжатии";
const CHAR_ * lpSaveDirHelp = " - указывает на каталог в который будут сохранены все данные";

struct FMapField
{
	UI32 iFieldID;
	FString sFieldName;
};


struct FColorMap
{
	RGBA rColor;
	FString sColor;
};

const FMapField fFieldsMap[] = 
{
	{ FAtlasConfig::FIELD_WORKDIR, sWorkDir },
	{ FAtlasConfig::FIELD_MAXWIDTH, sMaxWidth },
	{ FAtlasConfig::FIELD_COMPR, sCompr },
	{ FAtlasConfig::FIELD_PACK, sPack },
	{ FAtlasConfig::FIELD_MINALPHA, sMinAlpha },
	{ FAtlasConfig::FIELD_MAXALPHA, sMaxAlpha },
	{ FAtlasConfig::FIELD_PRODALPHA, sProdAlpha },
	{ FAtlasConfig::FIELD_BRIGHT, sBrightness },
	{ FAtlasConfig::FIELD_BACKCOLOR, sBackColor },
	{ FAtlasConfig::FIELD_BORDERWIDTH, sBorderWidth },
	{ FAtlasConfig::FIELD_JPEGQUALITY, sJPEGQuality },
	{ FAtlasConfig::FIELD_SAVEDIR, sSaveDir }
};

const FColorMap cColorsMap[] =
{
	{ { 0, 0, 0, 255 }, sBackColorConst[0] },
	{ { 255, 255, 255, 255 }, sBackColorConst[1] },
	{ { 255, 0, 0, 255 } , sBackColorConst[2] },
	{ { 0, 255, 0, 255 }, sBackColorConst[3] },
	{ { 0, 0, 255, 255 }, sBackColorConst[4] },
	{ { 255, 36 ,236, 255 }, sBackColorConst[5] },
	{ { 245, 151, 46, 255 }, sBackColorConst[6] }
};

///типы опций конфигураций
#define CONFIG_SETTINGS 1 //настройки генерации атласа
#define CONFIG_ANIMATIONS 2 //задание имен и кадров анимации
#define CONFIG_FRAMES 3 //задание кадров и имен файлов для создания аталаса
#define CONFIG_ADDITIONALS 4

struct FConfigType
{
	FString sName;
	UI32 iCfgType;
};

const FConfigType cConfigType[] = 
{
	{ "SETTINGS", CONFIG_SETTINGS },
	{ "ANIMATIONS", CONFIG_ANIMATIONS },
	{ "FRAMES", CONFIG_FRAMES },
	{ "ADDITIONALS", CONFIG_ADDITIONALS }
};

enum
{
	ANIM_UNK_CONST_ = 0,
	ANIM_LOOPED_
};

struct FAnimConst
{
	FString sConstName;
	UI32 iConst;
};

const FAnimConst aAnimConsts[] =
{
	{ "LOOP", ANIM_LOOPED_ }
};

///символы для слов
const CHAR_ cWordSym [] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '_' };

//символы для чисел
const CHAR_ cNumberSym [] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.' };

//типы токенов
#define TOKEN_WORD 1 //слово
#define TOKEN_NUMBER 2 //число
#define TOKEN_SQBRACK0 3 //квадратная скобка [
#define TOKEN_SQBRACK1 4 //квадратная скобка ]
#define TOKEN_EOF 5 //конец файла
#define TOKEN_STRING 6 //строка
#define TOKEN_EOL 7 //конец строки
#define TOKEN_SUBTRACT 8 //тире


///макросы для упрощения вывода ошибок в парсинге
#define CHECK_EOFE( t, action ) if( t == TOKEN_EOF ) { PUT_ERROR( "Unexpected end of file" ); action; }
#define CHECK_EOLE( t, action ) if( t == TOKEN_EOL ) { PUT_ERROR( "Unexpected end of line" ); action; }
#define CHECK_EOLFE( t, action ) CHECK_EOFE( t, action ) CHECK_EOLE( t, action )
#define CHECK_EOLF( t, action ) if( (t != TOKEN_EOL) && (t != TOKEN_EOF) ) { PUT_ERROR( "Unexpected token in the end of line" ); action; }
#define CHECK_STR( t, buffer, action ) if( t != TOKEN_STRING ) { PUT_ERROR( "Imposible token: %", buffer ); action; }
#define CHECK_NUM( t, buffer, action ) if( t != TOKEN_NUMBER ) { PUT_ERROR( "Token must be a number: %", buffer ); action; }
#define CHECK_INT( t, buffer, action ) if( t != FAtlasConfig::TYPE_INT ) { PUT_ERROR( "Number must be a integer: %", buffer ); action; }
#define CHECK_EOF_NUM( t, buffer, action ) CHECK_EOFE( t, action ) CHECK_NUM( t, buffer, action )
#define CHECK_UNK( t, buffer, action ) if( t == 0 ) {PUT_ERROR( "Unknown token: %s", buffer ); action; }
#define CHECK_WORD( t, buffer, action ) if( t != TOKEN_WORD ) { PUT_ERROR( "Token must be a single word: \"%s\"", buffer ); action; }
#define CHECK_CONST( t, buffer, action ) if( (t != TOKEN_WORD) && (t != TOKEN_NUMBER) && (t != TOKEN_STRING) ) { PUT_ERROR( "Token must be a constant value or string or number: \"%s\"", buffer ); action; }
#define CHECK_SUBTRACT( t, buffer, action ) if( t != TOKEN_SUBTRACT ) { PUT_ERROR( "Expression must have a single value or a range values", buffer ); action; }


const FString & RemapNameByID( UI32 iID )
{
	for( I32 i = 0;i < ARRAY_SIZE( fFieldsMap );i++ )
	{
		if( fFieldsMap[i].iFieldID == iID )
			return fFieldsMap[i].sFieldName;
	}

	return NULL;
}


FAtlasConfig::FAtlasConfig()
{
	for( I32 i = 0;i < MAX_HASH;i++ )
		lpFieldHash[i] = NULL;

	InitFields();
}

FAtlasConfig::~FAtlasConfig()
{
	Clear();
}

///проверка является ли символ символом слова
bool IsWordSym( CHAR_ cChar )
{
	for( I32 i = 0;i < ARRAY_SIZE( cWordSym );i++ )
	{
		if( cWordSym[i] == cChar )
			return true;
	}

	return false;
}

//проверка является ли символ символом числа
bool IsNumberSym( CHAR_ cChar )
{
	for( I32 i = 0;i < ARRAY_SIZE( cNumberSym );i++ )
	{
		if( cNumberSym[i] == cChar )
			return true;
	}

	return false;
}

//прогпустить пробелы и табуляцию
UI32 SkipSpaces( CHAR_ * lpStr )
{
	UI32 iInd = 0;
	while( (lpStr[iInd] == ' ') || (lpStr[iInd] == '\t') )
		iInd++;

	return iInd;
}

//пропустить пустые строки
UI32 SkipRet( CHAR_ * lpStr )
{
	UI32 iInd = 0;
	while( (lpStr[iInd] == '\r') || (lpStr[iInd] == '\n') )
		iInd++;

	return iInd;
}

//пропустить пустые строки, пробелы и табуляцию
UI32 SkipSpacesRet( CHAR_ * lpStr )
{
	UI32 iInd = 0;
	while( (lpStr[iInd] == '\r') || (lpStr[iInd] == ' ') || (lpStr[iInd] == '\t') || (lpStr[iInd] == '\n') )
		iInd++;

	return iInd;
}

//перевод имя опции конфига во внутрений код
UI32 GetConfigType( const FString & sName )
{
	for( I32 i = 0;i < ARRAY_SIZE( cConfigType );i++ )
	{
		if( cConfigType[i].sName == sName )
			return cConfigType[i].iCfgType;
	}

	return 0;
}

//UI32 GetConfigType( CHAR_ * lpStr, CHAR_ * lpBuffer )
//{
//	UI32 iInd = 0;
//	if( lpStr[iInd++] != '[' )
//	{
//		iCfgType = 0;
//		return iInd - 1;
//	}
//
//	I32 i = 0;
//	CHAR_ cBuffer[MAX_BUFFER];
//	while( lpStr[iInd] != ']' )
//	{
//		if( lpStr[iInd] == '\r' )
//		{
//			iCfgType = 0;
//			return iInd - 1;
//		}
//		cBuffer[i++] = lpStr[iInd++];
//	}
//	cBuffer[i] = 0;
//	FString sCfgType( cBuffer );
//	iCfgType = GetConfigType( sCfgType );
//
//	return iInd;
//}

//проверить правильность символов числа
bool CheckNumber( CHAR_ * lpStr )
{
	UI32 iInd = 0;
	bool bSep = false;

	while( lpStr[iInd] != 0 )
	{
		if( !IsNumberSym( lpStr[iInd] ) )
			return false;
		if( lpStr[iInd] == '.' )
		{
			if( bSep )
				return false;
			bSep = true;
		}
		iInd++;
	}

	return true;
}

//получить тип константы целое число, с плавающей точкой или строка
UI32 GetTokenType( const CHAR_ * lpStr )
{
	if( IsNumberSym( *lpStr ) )
	{
		while( *lpStr != 0 )
		{
			if( *lpStr == '.' )
				return FAtlasConfig::TYPE_FLOAT;
			lpStr++;
		}
		return FAtlasConfig::TYPE_INT;
	}

	return FAtlasConfig::TYPE_STR;
}

//получить следущий токен вход строка, на выходе токен в буфере, его код и последний индекс символа
UI32 GetToken( CHAR_ * lpStr, CHAR_ * lpBuffer, UI32 & iTokenType )
{
	UI32 iInd = SkipSpaces( lpStr );//пропускаем пробелы

	
	if( IsNumberSym( lpStr[iInd] ) )//если первый символ является символом числа то токен является числом
		iTokenType = TOKEN_NUMBER;
	else if( IsWordSym( lpStr[iInd] ) )//если первый токен является символом слова то токен является словом
		iTokenType = TOKEN_WORD;
	else
	{
		UI32 i = 0;
		lpBuffer[0] = lpStr[iInd];
		lpBuffer[1] = 0;
		switch( lpStr[iInd] )
		{
		case '[':
			iTokenType = TOKEN_SQBRACK0;
			return iInd + 1;
		case ']':
			iTokenType = TOKEN_SQBRACK1;
			return iInd + 1;
		case 0:
			iTokenType = TOKEN_EOF;
			return iInd;
		case '"'://токен является строкой
			iTokenType = TOKEN_STRING;
			iInd++;
			while( (lpStr[iInd] != 0)&&(lpStr[iInd] != '\r')&&(lpStr[iInd] != '"') )
				lpBuffer[i++] = lpStr[iInd++];
			lpBuffer[i] = 0;
			if( lpStr[iInd] == '"' )
			{
				iInd++;
				return iInd;
			}

			PUT_ERROR( "Unexpected end of string: %s", lpBuffer );
			iTokenType = 0;
			return iInd;
		case '\r':
			iTokenType = TOKEN_EOL;
			return iInd + 1;
		case '-':
			iTokenType = TOKEN_SUBTRACT;
			return iInd + 1;
		default:
			iTokenType = 0;
			PUT_ERROR( "Unnown symbol: \"%c\"", lpStr[iInd] );
			return 0;
		}
	}

	//копируем символы м буффер
	UI32 i = 0;
	while( IsNumberSym( lpStr[iInd] )||IsWordSym( lpStr[iInd] ) )
		lpBuffer[i++] = lpStr[iInd++];
	lpBuffer[i] = 0;

	if( iTokenType == TOKEN_NUMBER )//если токен число то проверяем правильность записи
	{
		if( !CheckNumber( lpBuffer ) )
		{
			PUT_ERROR( "Impossible characters including in number: %s", lpBuffer );
			iTokenType = 0;
		}
	}

	return iInd;
}

UI32 GetAnimatedConsts( const FString & sConst )
{
	for( UI32 i = 0;i < ARRAY_SIZE( aAnimConsts );i++ )
	{
		if( aAnimConsts[i].sConstName == sConst )
			return aAnimConsts[i].iConst;
	}

	return ANIM_UNK_CONST_;
}

//парсим настройки атласа
/*
Настройки записываются ввиде "ключ" "значение"(например MAX_WIDTH 512) в одну строку
*/
UI32  FAtlasConfig::ParseSettings( CHAR_ * lpStr )
{
	UI32 iInd = 0;
	CHAR_ cBuffer[MAX_BUFFER];
	UI32 iTokenType = 0;

	while( true )
	{
		iInd += SkipSpacesRet( &lpStr[iInd] );
		iInd += GetToken( &lpStr[iInd], cBuffer, iTokenType );
		if( iTokenType == TOKEN_EOF )
			return iInd;
		if( iTokenType == TOKEN_SQBRACK0 )
			return iInd - 1;

		CHECK_WORD( iTokenType, cBuffer, return 0 );

		FString sFieldName = cBuffer;

		iInd += GetToken( &lpStr[iInd], cBuffer, iTokenType );
		CHECK_EOLFE( iTokenType, return 0 );
		CHECK_CONST( iTokenType, cBuffer, return 0 );

		UI32 iFieldType = GetTokenType( cBuffer );
		void * lpValue = NULL;
		F32 fFloat;
		I32 iInt;
		FString sStr;

		switch( iFieldType )
		{
		case TYPE_FLOAT:
			fFloat = atof( cBuffer );
			lpValue = (void *)&fFloat;
			break;
		case TYPE_INT:
			iInt = atoi( cBuffer );
			lpValue = (void *)&iInt;
			break;
		case TYPE_STR:
			sStr = cBuffer;
			lpValue = (void *)&sStr;
			break;
		}

		iInd += GetToken( &lpStr[iInd], cBuffer, iTokenType );
		CHECK_EOLF( iTokenType, return 0 );

		SetFieldData( sFieldName, iFieldType, lpValue );
		iInd += SkipSpacesRet( &lpStr[iInd] );
	}

	return iInd;
}

//парсим список анимаций
/*анимация записывается ввиде "имя" "первый кадр" - "последний кадр" (например WALK 0 - 7)
либо если это просто кадр "имя" "номер кадра"(например WAIT 8)
кадры идут включая т.е. [0, 7] и описание одной анимации идет в одну строку
*/
UI32 FAtlasConfig::ParseAnimations( CHAR_ * lpStr )
{
	CHAR_ cBuffer[MAX_BUFFER];
	UI32 iTokenType, iInd = 0;

	while( true )
	{
		iInd += SkipSpacesRet( &lpStr[iInd] );
		iInd += GetToken( &lpStr[iInd], cBuffer, iTokenType );
		if( iTokenType == TOKEN_EOF )
			return iInd;
		if( iTokenType == TOKEN_SQBRACK0 )
			return iInd - 1;
		CHECK_WORD( iTokenType, cBuffer, return 0 );
		FString sAnimName = cBuffer;

		iInd += GetToken( &lpStr[iInd], cBuffer, iTokenType );
		CHECK_EOLFE( iTokenType, return 0 );
		CHECK_EOF_NUM( iTokenType, cBuffer, return 0 );
		UI32 iNumType = GetTokenType( cBuffer );
		CHECK_INT( iNumType, cBuffer, return 0 );
		UI32 iStartFrm = atoi( cBuffer ), iEndFrm;
		bool bLooped = false;

		iInd += GetToken( &lpStr[iInd], cBuffer, iTokenType );
		if( iTokenType == TOKEN_EOL || iTokenType == TOKEN_EOF )
		{
			iEndFrm = iStartFrm;
		}
		else
		{
			CHECK_SUBTRACT( iTokenType, sAnimName.GetChar(), return 0 );
			iInd += GetToken( &lpStr[iInd], cBuffer, iTokenType );
			CHECK_EOLFE( iTokenType, return 0 );
			CHECK_EOF_NUM( iTokenType, cBuffer, return 0 );
			iNumType = GetTokenType( cBuffer );
			CHECK_INT( iNumType, cBuffer, return 0 );
			iEndFrm = atoi( cBuffer );
			iInd += GetToken( &lpStr[iInd], cBuffer, iTokenType );

			if( iTokenType == TOKEN_WORD )
			{
				UI32 iConst = GetAnimatedConsts( cBuffer );
				switch( iConst )
				{
				case ANIM_LOOPED_:
					bLooped = true;
					break;
				default:
					PUT_ERROR( "Unknown animation constant: \"%s\"", cBuffer );
					return 0;
				}
				iInd += GetToken( &lpStr[iInd], cBuffer, iTokenType );
			}
		}

		CHECK_EOLF( iTokenType, return 0 );

		FAnimationParams * lpAnim = new FAnimationParams;
		lpAnim->sAnimName = sAnimName;
		lpAnim->iStartFrm = iStartFrm;
		lpAnim->iEndFrm = iEndFrm;
		lpAnim->bLooped = bLooped;

		lAnimationList.PushBack( lpAnim );
	}

	return iInd;
}

//Парсим кадры изображений
/*
запись кадра осуществляется ввиде "имя кадра" "левый угол начала изобр в файле" "верх угол начала изобр в файле" "высота изобр" "ширина" (например "frame0.bmp" 0 0 128 128)
либо ввиде "имя кадра" "левый угол начала изобр в файле" "верх угол начала изобр в файле" "высота изобр" "ширина" "необзодимая высота" "необходимая ширина" (например "frame0.bmp" 0 0 128 128 64 64)
возможно только уменьшение
*/
UI32 FAtlasConfig::ParseFrames( CHAR_ * lpStr )
{
	CHAR_ cBuffer[MAX_BUFFER];
	UI32 iInd = 0, iTokenType = 0;
	UI32 iParams[6];
	FString sFilenName;

	while( true )
	{
		iInd += SkipSpacesRet( &lpStr[iInd] );
		iInd += GetToken( &lpStr[iInd], cBuffer, iTokenType );

		if( iTokenType == TOKEN_EOF )
			return iInd;
		if( iTokenType == TOKEN_SQBRACK0 )
			return iInd - 1;

		CHECK_STR( iTokenType, cBuffer, return 0 );
		UI32 i = 0;
		sFilenName = cBuffer;

		iInd += GetToken( &lpStr[iInd], cBuffer, iTokenType );
		for(;(iTokenType != TOKEN_EOL)&&(i < 6)&&(iTokenType != TOKEN_EOF);i++ )
		{
			CHECK_EOF_NUM( iTokenType, cBuffer, return 0 );
			UI32 iNumType = GetTokenType( cBuffer );
			CHECK_INT( iNumType, cBuffer, return 0 );
			iParams[i] = atoi( cBuffer );
			iInd += GetToken( &lpStr[iInd], cBuffer, iTokenType );
		}

		CHECK_EOLF( iTokenType, return 0 );

		if( i == 0)
		{
			iParams[0] = 0;
			iParams[1] = 0;
			iParams[2] = 0;
			iParams[3] = 0;
			iParams[4] = 0;
			iParams[5] = 0;
		}
		if( i == 2 )
		{
			iParams[4] = iParams[0];
			iParams[5] = iParams[1];
			iParams[0] = 0;
			iParams[1] = 0;
			iParams[2] = 0;
			iParams[3] = 0;
		}
		else if( i == 4 )
		{
			iParams[4] = iParams[2];
			iParams[5] = iParams[3];
		}
		else if( i != 6 )
		{
			PUT_ERROR( "Number of parameters in frames must be 4 or 6" );
			return 0;
		}

		FFrameParams * lpFrame = new FFrameParams;
		lpFrame->x = iParams[0];
		lpFrame->y = iParams[1];
		lpFrame->iWidth = iParams[2];
		lpFrame->iHeight = iParams[3];
		lpFrame->iScaleW = iParams[4];
		lpFrame->iScaleH = iParams[5];
		lpFrame->sFileName = sFilenName;

		lFrameList.PushBack( lpFrame );
	}

	return iInd;
}

UI32 FAtlasConfig::ParseAdditionals( CHAR_ * lpStr )
{
	CHAR_ cBuffer[MAX_BUFFER];
	UI32 iInd = 0, iTokenType = 0;

	while( true )
	{
		iInd += SkipSpacesRet( &lpStr[iInd] );
		iInd += GetToken( &lpStr[iInd], cBuffer, iTokenType );

		if( iTokenType == TOKEN_EOF )
			return iInd;
		if( iTokenType == TOKEN_SQBRACK0 )
			return iInd - 1;

		CHECK_WORD( iTokenType, cBuffer, return 0 );
		FString sFieldName = cBuffer;

		iInd += GetToken( &lpStr[iInd], cBuffer, iTokenType );
		CHECK_EOLFE( iTokenType, return 0 );

		FAddField * lpField = new FAddField;
		lpField->sFieldName = sFieldName;

		UI32 iType = GetTokenType( cBuffer );
		switch( iType )
		{
		case TYPE_INT:
			lpField->iValType = TYPE_INT;
			lpField->iInt = atoi( cBuffer );
			break;
		case TYPE_FLOAT:
			lpField->iValType = TYPE_FLOAT;
			lpField->fFloat = atof( cBuffer );
			break;
		case TYPE_STR:
			lpField->iValType = TYPE_STR;
			lpField->lpStr = new FString( cBuffer );
			break;
		}
		lAddFieldList.PushBack( lpField );

		iInd += GetToken( &lpStr[iInd], cBuffer, iTokenType );
		CHECK_EOLF( iTokenType, return 0 );
	}

	return iInd;
}

//открыте и парсинг конфиг файла [] скобками отделяется тип опции
bool FAtlasConfig::Open( const FString & sCfgName )
{
	FString sCfgFullName = sCfgName + ATLAS_CONFIG_EXT;
	FFile * lpFile = FFile::MakeFile( sCfgFullName, FFile::FILE_READ );
	if( !lpFile->OpenSucc() )
		return false;

	CHAR_ * lpData = (CHAR_ *)FStack::GetInstanceStack()->PushBlock( lpFile->GetSize() + 1 );
	lpFile->Read( lpData, lpFile->GetSize() );
	lpData[lpFile->GetSize()] = 0;

	UI32 iInd = 0;
	CHAR_ cBuffer[MAX_BUFFER];
	bool bError = false;
	while( true )
	{
		UI32 iCfgType, iTokenType;
		iInd += SkipSpacesRet( &lpData[iInd] );
		iInd += GetToken( &lpData[iInd], cBuffer, iTokenType );
		if( iTokenType == TOKEN_EOF )
			break;

		if( iTokenType != TOKEN_SQBRACK0 )
		{
			PUT_ERROR( "Unexpected token: %s", cBuffer );
			bError = true;
			break;
		}
		iInd += GetToken( &lpData[iInd], cBuffer, iTokenType );
		CHECK_EOFE( iTokenType, break );

		iCfgType = GetConfigType( cBuffer );
		if( iCfgType == 0 )
		{
			PUT_ERROR( "Unknown config type: %s", cBuffer );
			bError = true;
			break;
		}

		iInd += GetToken( &lpData[iInd], cBuffer, iTokenType );
		CHECK_EOFE( iTokenType, break );

		if( iTokenType != TOKEN_SQBRACK1 )
		{
			PUT_ERROR( "Unexpected token: %s", cBuffer );
			bError = true;
			break;
		}

		UI32 iParseInd = 0;
		switch( iCfgType )
		{
		case CONFIG_SETTINGS:
			iParseInd += ParseSettings( &lpData[iInd] );
			break;
		case CONFIG_ANIMATIONS:
			iParseInd += ParseAnimations( &lpData[iInd] );
			break;
		case CONFIG_FRAMES:
			iParseInd += ParseFrames( &lpData[iInd] );
			break;
		case CONFIG_ADDITIONALS:
			iParseInd += ParseAdditionals( &lpData[iInd] );
			break;
		}
		if( iParseInd == 0 )
		{
			bError = true;
			break;
		}

		iInd += iParseInd;
	}

	FStack::GetInstanceStack()->PopBlock();
	FFile::ReleaseFile( lpFile );

	return !bError;
}

void FAtlasConfig::Clear()
{
	for( I32 i = 0;i < MAX_HASH;i++ )
	{
		FField * lpField = lpFieldHash[i];
		while( lpField )
		{
			FField * lpTmp = lpField;
			lpField = lpField->lpNext;
			if( lpTmp->iFieldType == TYPE_STR )
				delete lpTmp->lpStr;
			delete lpTmp;
		}
		lpFieldHash[i] = NULL;
	}

	FFrameIterator iIt = lFrameList.Begin();
	for(;iIt != lFrameList.End();iIt++ )
		delete *iIt;
	lFrameList.Clear();

	FAnimationIterator iItA = lAnimationList.Begin();
	for(;iItA != lAnimationList.End();iItA++ )
		delete *iItA;
	lAnimationList.Clear();

	FAddFieldIterator iItF = lAddFieldList.Begin();
	for(;iItF != lAddFieldList.End();iItF++ )
	{
		if( (*iItF)->iValType == TYPE_STR )
			delete (*iItF)->lpStr;
		delete *iItF;
	}
	lAddFieldList.Clear();
}

void FAtlasConfig::Close()
{
	Clear();
	InitFields();
}

void FAtlasConfig::InitFields()
{
	RegisterField( sWorkDir, TYPE_STR, &sDeffWorkDir, FIELD_ANY, NULL, 0, lpWorkDirHelp );
	RegisterField( sSaveDir, TYPE_STR, &sDeffSaveDir, FIELD_ANY, NULL, 0, lpSaveDirHelp );
	RegisterField( sMaxWidth, TYPE_INT, &iDeffWidth, FIELD_RANGE_INT, iWidthRange, ARRAY_SIZE( iWidthRange ), lpMaxWidthHelp );
	RegisterField( sCompr, TYPE_STR, &sDeffCompr, FIELD_CONST, &sComprConst, ARRAY_SIZE( sComprConst ), lpComprHelp );
	RegisterField( sPack, TYPE_STR, &sDeffPack, FIELD_CONST, &sPackConst, ARRAY_SIZE( sPackConst ), lpPackHelp );
	RegisterField( sMinAlpha, TYPE_INT, &iMinAlphaDeff, FIELD_RANGE_INT, &iMinAlphaRange, ARRAY_SIZE( iMinAlphaRange ), lpMinAlphaHelp );
	RegisterField( sMaxAlpha, TYPE_INT, &iMaxAlphaDeff, FIELD_RANGE_INT, &iMaxAlphaRange, ARRAY_SIZE( iMaxAlphaRange ), lpMaxAlphaHelp );
	RegisterField( sProdAlpha, TYPE_STR, &sProdAlphaDeff, FIELD_CONST, &sProdAlphaConst, ARRAY_SIZE( sProdAlphaConst ), lpProdAlphaHelp );
	RegisterField( sBrightness, TYPE_INT, &iBrightMinDeff, FIELD_RANGE_INT, &iBrightMinRange, ARRAY_SIZE( iBrightMinRange ), lpBrightHelp );
	RegisterField( sBackColor, TYPE_STR, &sBackColorDeff, FIELD_CONST, &sBackColorConst, ARRAY_SIZE( sBackColorConst ), lpBackColorHelp );
	RegisterField( sBorderWidth, TYPE_INT, &iBorderWidthDeff, FIELD_RANGE_INT, &iBorderWidthRange, ARRAY_SIZE( iBorderWidthRange ), lpBorderWidthHelp );
	RegisterField( sJPEGQuality, TYPE_INT, &iJPEGQualityDeff, FIELD_RANGE_INT, &iJPEGQualityRange, ARRAY_SIZE( iJPEGQualityRange ), lpJPEGQualityHelp );
}

FAtlasConfig::FField * FAtlasConfig::GetField( const FString & sFieldName )const
{
	FField * lpField = lpFieldHash[sFieldName.GetKey()%MAX_HASH];
	if( !lpField )
		return NULL;

	while( lpField->sFieldName != sFieldName )
		lpField = lpField->lpNext;

	return lpField;
}

void FAtlasConfig::AddField( FField * lpField )
{
	FField * lpTmp = GetField( lpField->sFieldName );
	if( lpTmp )
		return;
	FField * lpLast = lpFieldHash[lpField->sFieldName.GetKey()%MAX_HASH];
	lpField->lpNext = lpLast;
	lpFieldHash[lpField->sFieldName.GetKey()%MAX_HASH] = lpField;
}

void FAtlasConfig::RegisterField( const FString & sFieldName, UI32 iFieldType, const void * lpDeffValue, UI32 iAvaibleValType, const void * lpAvaibleVal, UI32 iAvaibleValCount, const CHAR_ * lpHelpData )
{
	if( GetField( sFieldName ) )
	{
		PUT_ERROR( "Field with name \"%s\" is already exist", sFieldName );
		return;
	}

	FField * lpField = new FField;
	lpField->sFieldName = sFieldName;
	lpField->iFieldType = iFieldType;
	switch( iFieldType )
	{
	case TYPE_FLOAT:
		lpField->fFloat = *(F32 *) lpDeffValue;
		break;
	case TYPE_INT:
		lpField->iInt = *(I32 *) lpDeffValue;
		break;
	case TYPE_STR:
		lpField->lpStr = new FString;
		*lpField->lpStr = *(FString *)lpDeffValue;
		break;
	}

	lpField->iAvaibleValType = iAvaibleValType;
	lpField->iAvaibleValCount = iAvaibleValCount;
	switch ( iAvaibleValType )
	{
	case FIELD_RANGE_FLOAT:
		lpField->lpFloatRange = (const F32 *)lpAvaibleVal;
		break;
	case FIELD_RANGE_INT:
		lpField->lpIntRange = (const I32 *)lpAvaibleVal;
		break;
	case FIELD_CONST:
		lpField->lpConsts = (const FString *)lpAvaibleVal;
		break;
	};

	lpField->lpHelpData = lpHelpData;
	AddField( lpField );
}

void FAtlasConfig::SetFieldData( const FString & sFieldName, UI32 iFieldType, const void * lpValue )
{
	FField * lpField = GetField( sFieldName );
	if( !lpField )
	{
		PUT_ERROR( "Undefined field with name: %s", sFieldName );
		return ;
	}

	if( lpField->iFieldType != iFieldType )
	{
		switch( lpField->iFieldType )
		{
		case TYPE_FLOAT:
			PUT_ERROR( "Field \"%s\" must have a float type", sFieldName );
			break;
		case TYPE_INT:
			PUT_ERROR( "Field \"%s\" must have a integer type", sFieldName );
			break;
		case TYPE_STR:
			PUT_ERROR( "Field \"%s\" must have a string type", sFieldName );
			break;
		}
	}

	if( lpField->iAvaibleValType )
	{
		FString sVal;
		F32 fVal;
		I32 iVal;
		I32 i = 0;
		switch( lpField->iAvaibleValType )
		{
		case FIELD_CONST:
			sVal = *(FString *)lpValue;
			for(;i < lpField->iAvaibleValCount;i++ )
			{
				if( lpField->lpConsts[i] == sVal )
					break;
			}
			if( i == lpField->iAvaibleValCount )
			{
				PUT_ERROR( "Incorrect value constants: %s at field \"%s\"", sVal.GetChar(), sFieldName.GetChar() );
				return;
			}
			break;
		case FIELD_RANGE_FLOAT:
			fVal = *(F32 *)lpValue;
			if( (fVal < lpField->lpFloatRange[0]) || (fVal > lpField->lpFloatRange[1]) )
			{
				PUT_ERROR( "Value of field \"%s\" is out of range, must be betweeen %f - %f", sFieldName.GetChar(), lpField->lpFloatRange[0], lpField->lpFloatRange[1] );
				return;
			}
			break;
		case FIELD_RANGE_INT:
			iVal = *(I32 *)lpValue;
			if( (iVal < lpField->lpIntRange[0]) || (iVal > lpField->lpIntRange[1]) )
			{
				PUT_ERROR( "Value of field \"%s\" is out of range, must be betweeen %f - %f", sFieldName.GetChar(), lpField->lpIntRange[0], lpField->lpIntRange[1] );
				return;
			}
			break;
		}
	}

	switch( iFieldType )
	{
	case TYPE_FLOAT:
		lpField->fFloat = *(F32 *) lpValue;
		break;
	case TYPE_INT:
		lpField->iInt = *(I32 *) lpValue;
		break;
	case TYPE_STR:
		*lpField->lpStr = *(FString *)lpValue;
		break;
	}
}

void FAtlasConfig::GetHelpInfo( CHAR_ * lpBuffer )
{
	const CHAR_ * lpLineEnd = "\r\n";

	UI32 iBuffLen = 0;
	for( UI32 i = 0;i < MAX_HASH;i++ )
	{
		FField * lpField = lpFieldHash[i];
		while( lpField )
		{
			iBuffLen += sprintf( lpBuffer + iBuffLen, "%s %s доступные значения: ", lpField->sFieldName.GetChar(), lpField->lpHelpData );
			switch( lpField->iAvaibleValType )
			{
			case FIELD_RANGE_FLOAT:
				iBuffLen += sprintf( lpBuffer + iBuffLen, "из вещественного диапазона от %f до %f%s", lpField->lpFloatRange[0], lpField->lpFloatRange[1], lpLineEnd );
				break;
			case FIELD_RANGE_INT:
				iBuffLen += sprintf( lpBuffer + iBuffLen, "из целого диапазона от %i до %i%s", lpField->lpIntRange[0], lpField->lpIntRange[1], lpLineEnd );
				break;
			case FIELD_ANY:
				iBuffLen += sprintf( lpBuffer + iBuffLen, "любое значение%s", lpLineEnd );
				break;
			case FIELD_CONST:
				iBuffLen += sprintf( lpBuffer + iBuffLen, "%s", lpField->lpConsts[0].GetChar() );
				for( UI32 j = 1;j < lpField->iAvaibleValCount;j++ )
					iBuffLen += sprintf( lpBuffer + iBuffLen, ", %s", lpField->lpConsts[j].GetChar() );
				iBuffLen += sprintf( lpBuffer + iBuffLen, lpLineEnd );
				break;
			}
			lpField = lpField->lpNext;
		}
	}

	lpBuffer[iBuffLen] = 0;
}

UI32 FAtlasConfig::GetFrameCount()const
{
	return lFrameList.GetCount();
}

UI32 FAtlasConfig::GetAnimationCount()const
{
	return lAnimationList.GetCount();
}

UI32 FAtlasConfig::GetAddFieldsCount()const
{
	return lAddFieldList.GetCount();
}

bool FAtlasConfig::GetFrameParams( UI32 iFrameInd, FFrameParams & fFrameParams )
{
	if( iFrameInd >= GetFrameCount() )
		return false;

	FFrameIterator iIt = lFrameList.Begin();
	for( I32 i = 0;i != iFrameInd;i++, iIt++ );

	fFrameParams.x = (*iIt)->x;
	fFrameParams.y = (*iIt)->y;
	fFrameParams.iWidth = (*iIt)->iWidth;
	fFrameParams.iHeight = (*iIt)->iHeight;
	fFrameParams.iScaleW = (*iIt)->iScaleW;
	fFrameParams.iScaleH = (*iIt)->iScaleH;
	fFrameParams.sFileName = (*iIt)->sFileName;

	return true;
}

bool FAtlasConfig::GetAnimParams( UI32 iAnimInd, FAnimationParams & aAnimParams )
{
	if( iAnimInd >= GetAnimationCount() )
		return false;

	FAnimationIterator iIt = lAnimationList.Begin();
	for( I32 i = 0;i != iAnimInd;i++, iIt++ );

	aAnimParams.iStartFrm = (*iIt)->iStartFrm;
	aAnimParams.iEndFrm = (*iIt)->iEndFrm;
	aAnimParams.sAnimName = (*iIt)->sAnimName;
	aAnimParams.bLooped = (*iIt)->bLooped;

	return true;
}

bool FAtlasConfig::GetAddData( UI32 iAddInd, FAddField & aAddParams )
{
	if( iAddInd >= GetAddFieldsCount() )
		return false;

	FAddFieldIterator iIt = lAddFieldList.Begin();
	for( UI32 i = 0;i != iAddInd;i++, iIt++ );

	aAddParams.sFieldName = (*iIt)->sFieldName;
	aAddParams.iValType = (*iIt)->iValType;
	switch( aAddParams.iValType )
	{
	case TYPE_FLOAT:
		aAddParams.iInt = (*iIt)->iInt;
		break;
	case TYPE_INT:
		aAddParams.fFloat = (*iIt)->fFloat;
		break;
	case TYPE_STR:
		aAddParams.lpStr = (*iIt)->lpStr;
		break;
	}

	return true;
}

bool FAtlasConfig::GetFieldValueFloat( const FString & sFieldName, F32 & fFloat )
{
	FField * lpField = GetField( sFieldName );

	if( !lpField )
		return false;
	if( lpField->iFieldType != TYPE_FLOAT )
		return false;

	fFloat = lpField->fFloat;
	return true;
}

bool FAtlasConfig::GetFieldValueInt( const FString & sFieldName, I32 & iInt )
{
	FField * lpField = GetField( sFieldName );

	if( !lpField )
		return false;
	if( lpField->iFieldType != TYPE_INT )
		return false;

	iInt = lpField->iInt;
	return true;
}

bool FAtlasConfig::GetFieldValueString( const FString & sFieldName, FString & sStr )
{
	FField * lpField = GetField( sFieldName );

	if( !lpField )
		return false;
	if( lpField->iFieldType != TYPE_STR )
		return false;

	sStr = *lpField->lpStr;
	return true;
}

bool FAtlasConfig::GetFieldValueFloat( UI32 iFieldID, F32 & fFloat )
{
	return GetFieldValueFloat( RemapNameByID( iFieldID ), fFloat );
}

bool FAtlasConfig::GetFieldValueInt( UI32 iFieldID, I32 & iInt )
{
	return GetFieldValueInt( RemapNameByID( iFieldID ), iInt );
}

bool FAtlasConfig::GetFieldValueString( UI32 iFieldID, FString & sStr )
{
	return GetFieldValueString( RemapNameByID( iFieldID ), sStr );
}

bool FAtlasConfig::Constant2Color( const FString & sColor, RGBA & rColor )
{
	for( UI32 i = 0;i < ARRAY_SIZE( cColorsMap );i++ )
	{
		if( cColorsMap[i].sColor == sColor )
		{
			rColor = cColorsMap[i].rColor;
			return true;
		}
	}

	return false;
}
