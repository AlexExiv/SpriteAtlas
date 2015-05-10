#ifndef __FATLAS_CONFIG_H__
#define __FATLAS_CONFIG_H__


#include "FString.h"
#include "FList.h"

#define ATLAS_CONFIG_EXT FString( ".acf" ) // расширение конфиг файла
#define ATLAS_CONST_YES FString( "YES" )
#define ATLAS_CONST_NO FString( "NO" )
#define ATLAS_CONST_AUTO FString( "AUTO" )
#define ATLAS_CONST_JPG FString( "JPEG" )
#define ATLAS_CONST_PNG FString( "PNG" )
#define ATLAS_CONST_RLE FString( "RLE" )


struct FFrameParams
{
	UI32 x, y;
	UI32 iWidth, iHeight;
	UI32 iScaleW, iScaleH;
	FString sFileName;
};

struct FAnimationParams
{
	UI32 iStartFrm, iEndFrm;
	bool bLooped;
	FString sAnimName;
};

struct FAddField
{
	FString sFieldName;
	UI32 iValType;
	union
	{
		I32 iInt;
		F32 fFloat;
		FString * lpStr;
	};
};


class FAtlasConfig
{
public:
	enum
	{
		FIELD_MAXWIDTH = 1,
		FIELD_WORKDIR,
		FIELD_COMPR,
		FIELD_PACK,
		FIELD_MINALPHA,
		FIELD_MAXALPHA,
		FIELD_BRIGHT,
		FIELD_PRODALPHA,
		FIELD_BACKCOLOR,
		FIELD_BORDERWIDTH,
		FIELD_JPEGQUALITY,
		FIELD_SAVEDIR
	};

	enum
	{
		TYPE_FLOAT = 1,
		TYPE_INT,
		TYPE_UINT,
		TYPE_STR
	};

private:
	enum
	{
		FIELD_ANY = 0,
		FIELD_CONST,
		FIELD_RANGE_FLOAT,
		FIELD_RANGE_INT
	};

	struct FField
	{
		FField * lpNext;

		FString sFieldName;//имя поля
		UI32 iFieldType;//тип поля(TYPE_FLOAT, TYPE_INT, TYPE_STR)
		union//собственно данные
		{
			F32 fFloat;
			I32 iInt;
			FString * lpStr;
		};

		UI32 iAvaibleValType, iAvaibleValCount;//тип возможных значений(FIELD_ANY, FIELD_CONST, FIELD_RANGE_FLOAT, FIELD_RANGE_INT) и их количество
		union
		{
			const I32 * lpIntRange;
			const F32 * lpFloatRange;
			const FString * lpConsts;
		};

		const CHAR_ * lpHelpData;//сопроводительная информация
	};


	typedef FList<FFrameParams *> FFrameList;
	typedef FList<FFrameParams *>::Iterator FFrameIterator;
	typedef FList<FAnimationParams *> FAnimationList;
	typedef FList<FAnimationParams *>::Iterator FAnimationIterator;
	typedef FList<FAddField *> FAddFieldList;
	typedef FList<FAddField *>::Iterator FAddFieldIterator;

	FFrameList lFrameList;
	FAnimationList lAnimationList;
	FAddFieldList lAddFieldList;
	FField * lpFieldHash[MAX_HASH];

	FField * GetField( const FString & sFieldName )const;
	void AddField( FField * lpField );
	void RegisterField( const FString & sFieldName, UI32 iFieldType, const void * lpDeffValue, UI32 iAvaibleValType, const void * lpAvaibleVal, UI32 iAvaibleValCount, const CHAR_ * lpHelpData );
	void SetFieldData( const FString & sFieldName, UI32 iFieldType, const void * lpValue );

	UI32  ParseSettings( CHAR_ * lpStr );
	UI32 ParseAnimations( CHAR_ * lpStr );
	UI32 ParseFrames( CHAR_ * lpStr );
	UI32 ParseAdditionals( CHAR_ * lpStr );

	bool GetFieldValueFloat( const FString & sFieldName, F32 & fFloat );
	bool GetFieldValueInt( const FString & sFieldName, I32 & iInt );
	bool GetFieldValueString( const FString & sFieldName, FString & sStr );

	void InitFields();
	void Clear();
	
public:
	FAtlasConfig();
	~FAtlasConfig();

	bool Open( const FString & sCfgName );
	void Close();

	void GetHelpInfo( CHAR_ * lpBuffer );
	UI32 GetFrameCount()const;
	UI32 GetAnimationCount()const;
	UI32 GetAddFieldsCount()const;
	bool GetFrameParams( UI32 iFrameInd, FFrameParams & fFrameParams );
	bool GetAnimParams( UI32 iAnimInd, FAnimationParams & aAnimParams );
	bool GetAddData( UI32 iAddInd, FAddField & aFieldParams );

	bool GetFieldValueFloat( UI32 iFieldID, F32 & fFloat );
	bool GetFieldValueInt( UI32 iFieldID, I32 & iInt );
	bool GetFieldValueString( UI32 iFieldID, FString & sStr );

	bool Constant2Color( const FString & sColor, RGBA & rColor );
};

#endif