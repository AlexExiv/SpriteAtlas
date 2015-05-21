#include "FAtlasBuilder.h"
#include "FAtlasConfig.h"
#include "FFrame.h"
#include "FConsole.h"
#include "FStack.h"
#include "FStringAllocator.h"
#include <string.h>
#include "FBMPResource.h"
#include <Windows.h>
#include <shellapi.h>
#include "FResourceManager.h"
#include "FFile.h"
#include "FJPEGResource.h"
#include "FPNGResource.h"



#define ATLAS_ALPHA_EDIT_FILE FString( "_alpha" )
#define ATLAS_FILE_EXT FString( ".adf" )

#define ATLAS_RLE_MASK_LEN 0x7f
#define ATLAS_RLE_MASK_PACK 0x80
#define ATLAS_PACK_BIT 0x80

struct FConstMap
{
	FString sConst;
	UI32 iConst;
};

const FConstMap cConstMap[] = 
{
	{ ATLAS_CONST_NO, ATLAS_COMPR_NONE },
	{ ATLAS_CONST_JPG, ATLAS_COMPR_JPG },
	{ ATLAS_CONST_PNG, ATLAS_COMPR_PNG },
	{ ATLAS_CONST_RLE, ATLAS_COMPR_RLE }
};

struct FHeightElem
{
	FFrame ** lpFrames;
	UI32 iHeight;
	UI32 iCount;
};

FAtlasBuilder::FAtlasBuilder() : lpAtlasCfg( NULL ), lpAtlasData( NULL )
{
	lpAtlasCfg = new FAtlasConfig();
}

FAtlasBuilder::~FAtlasBuilder()
{
	if( lpAtlasCfg )
		delete lpAtlasCfg;
}


void BuildHeightGroups( FAtlasBuilder::FFrameList lFrameList, FHeightElem ** lpHeightElem, UI32 & iElemCount )
{
	iElemCount = 0;
	while( lFrameList.GetCount() != 0 )
	{
		FAtlasBuilder::FFrameIterator iIt = lFrameList.Begin(), iMaxIt = iIt;
		for(;iIt != lFrameList.End();++iIt )
		{
			if( (*iIt)->GetHeight() > (*iMaxIt)->GetHeight() )
				iMaxIt = iIt;
		}

		lpHeightElem[iElemCount] = new FHeightElem;
		lpHeightElem[iElemCount]->iHeight = (*iMaxIt)->GetHeight();
		lpHeightElem[iElemCount]->iCount = 1;
		lpHeightElem[iElemCount]->lpFrames = (FFrame **)FMalloc( sizeof( FFrame * )*lFrameList.GetCount() );
		lpHeightElem[iElemCount]->lpFrames[0] = *iMaxIt;

		iIt = lFrameList.Begin();
		for(;iIt != lFrameList.End(); )
		{
			if( ((*iIt)->GetHeight() == (*iMaxIt)->GetHeight())&&(iIt != iMaxIt) )
			{
				lpHeightElem[iElemCount]->lpFrames[lpHeightElem[iElemCount]->iCount++] = (*iIt);
				iIt = lFrameList.Erase( iIt );
			}
			else
				iIt++;
		}

		lFrameList.Erase( iMaxIt );
		iElemCount++;
	}
}

int CompareWidth( const void * lpLeft, const void * lpRight )
{
	if( (*(FFrame **)lpLeft)->GetWidth() > (*(FFrame **)lpRight)->GetWidth() )
		return -1;
	else if( (*(FFrame **)lpLeft)->GetWidth() < (*(FFrame **)lpRight)->GetWidth() )
		return 1;
	return 0;
}

void SortByWidth( FFrame ** lpFrames, UI32 iCount )
{
	qsort( (void *)lpFrames, iCount, sizeof( FFrame * ), CompareWidth );
}

UI32 ConstStr2Number( const FString & sConst )
{
	for( UI32 i = 0;i < ARRAY_SIZE( cConstMap );i++ )
	{
		if( cConstMap[i].sConst == sConst )
			return cConstMap[i].iConst;
	}

	return 0;
}

UI32 FAtlasBuilder::BuildAtlas( const FString & sFileName0 )
{
	Clear();
	sFileName = sFileName0;
	if( !lpAtlasCfg->Open( sFileName ) )
	{
		PUT_ERROR( "Can't compile config file" );
		return 0;
	}

	FString sWorkDir;
	lpAtlasCfg->GetFieldValueString( FAtlasConfig::FIELD_WORKDIR, sWorkDir );

	for( UI32 i = 0;i < lpAtlasCfg->GetFrameCount();i++ )
	{
		FFrameParams fFrameParams;
		lpAtlasCfg->GetFrameParams( i, fFrameParams );
		FFrame * lpFrame = new FFrame( sWorkDir + fFrameParams.sFileName, fFrameParams.x, fFrameParams.y, fFrameParams.iWidth, fFrameParams.iHeight, fFrameParams.iScaleW, fFrameParams.iScaleH, i, lpAtlasCfg );
		if( !lpFrame->IsLoaded() )
		{
			delete lpFrame;
			continue;
		}
		lFrameList.PushBack( lpFrame );
	}

	I32 iMaxWidth;
	lpAtlasCfg->GetFieldValueInt( FAtlasConfig::FIELD_MAXWIDTH, iMaxWidth );
	FFrameIterator iIt = lFrameList.Begin();
	UI32 iSquare = 0, iMaxHeight = 0;
	for(;iIt != lFrameList.End();iIt++ )
	{
		if( (*iIt)->GetWidth() > iMaxWidth )
		{
			PUT_ERROR( "MAX_WIDTH field must be no less then maximum width of frame" );
			return 0;
		}

		iSquare += (*iIt)->GetWidth()*(*iIt)->GetHeight();
		iHeight = max( (*iIt)->GetHeight(), iHeight );
	}
	I32 iBorderW = 0;
	lpAtlasCfg->GetFieldValueInt( FAtlasConfig::FIELD_BORDERWIDTH, iBorderW );

	UI32 iEstHeight = iSquare/iMaxWidth;
	if( iEstHeight < (iMaxHeight + iBorderW) )
		iEstHeight = iMaxHeight + iBorderW;

	ReallocData( iMaxWidth, iEstHeight );

	FHeightElem ** lpHeightElem = (FHeightElem **)FStack::GetInstanceStack()->PushBlock( lFrameList.GetCount()*sizeof( FHeightElem * ) );
	UI32 iElemCount = 0;
	BuildHeightGroups( lFrameList, lpHeightElem, iElemCount );//раскладываем кадры в группы по высоте и отсортированые по понижению высоты
	for( UI32 i = 0;i < iElemCount;i++ )//сщртируем группы по понижению ширины
		SortByWidth( lpHeightElem[i]->lpFrames, lpHeightElem[i]->iCount );

	UI32 iCurHeader = 0;
	UI32 x = 0;
	UI32 * iStartY = (UI32 *)FStack::GetInstanceStack()->PushBlock( iMaxWidth*sizeof( UI32 ) );//сюда будем писать макс высоту по х чтобы начинать вставлять данные отсюда
	memset( iStartY, 0, sizeof( UI32 )*iMaxWidth );

	//Рисуем все кадры в одну текстуру: рисуем группы в порядке понижения высоты
	for( UI32 i = 0;i < iElemCount;i++ )
	{
		for( UI32 j = 0;j < lpHeightElem[i]->iCount;j++ )
		{
			FFrame * lpFrame = lpHeightElem[i]->lpFrames[j];
			if( lpFrame->IsWrited() )
				continue;

			if( x == 0 )
			{
				if( (iStartY[0] + lpFrame->GetHeight()) >= iHeight )//еслл высоты атласа не хватает увеличиваем ее
					ReallocData( iMaxWidth, iStartY[0] + lpFrame->GetHeight() );
			}

			if( (x + lpFrame->GetWidth()) > iMaxWidth )//в строку больше не влазиет кадров с этой группы
			{
				for( UI32 k = i + 1;k < iElemCount;k++ )//ищем в других группах кадры которые могут еще поместиться в остаток строки
				{
					for( UI32 l = 0;l < lpHeightElem[k]->iCount;l++ )
					{
						FFrame * lpSecFrame = lpHeightElem[k]->lpFrames[l];
						if( lpSecFrame->IsWrited() )
							continue;

						if( (lpSecFrame->GetWidth() + x) < iMaxWidth )
						{
							lpSecFrame->WriteToDataRGBA( lpAtlasData, x, iStartY[x], iMaxWidth, iHeight );
							for( UI32 t = x;t < (x + lpSecFrame->GetWidth());t++ )
								iStartY[t] += (lpSecFrame->GetHeight() + iBorderW);
							x += (lpSecFrame->GetWidth() + iBorderW);
							lpSecFrame->SetWrited( true );
						}
					}
				}

				if( (iStartY[0] + lpFrame->GetHeight()) >= iHeight )//еслл высоты атласа не хватает увеличиваем ее
					ReallocData( iMaxWidth, iStartY[0] + lpFrame->GetHeight() );

				x = 0;
			}

			lpFrame->WriteToDataRGBA( lpAtlasData, x, iStartY[x], iMaxWidth, iHeight );
			for( UI32 k = x;k < (x + lpFrame->GetWidth());k++ )
				iStartY[k] += (lpFrame->GetHeight() + iBorderW);

			lpFrame->SetWrited( true );
			x += (lpFrame->GetWidth() + iBorderW);
		}
	}


	for( UI32 i = 0;i < iElemCount;i++ )
	{
		FFree( lpHeightElem[i]->lpFrames );
		delete lpHeightElem[i];
	}

	FStack::GetInstanceStack()->PopBlock();
	FStack::GetInstanceStack()->PopBlock();

	return 1;
}

UI32 FAtlasBuilder::EditAlphaStart()
{
	I32 iMaxWidth;
	lpAtlasCfg->GetFieldValueInt( FAtlasConfig::FIELD_MAXWIDTH, iMaxWidth );

	RGB * lpAlphaData = (RGB *)FStack::GetInstanceStack()->PushBlock( iMaxWidth*iHeight*sizeof( RGB ) );
	RGB * lpDst = lpAlphaData;
	RGBA * lpSrc = lpAtlasData;

	for( UI32 i = 0;i < iHeight;i++ )
	{
		for( UI32 j = 0;j < iMaxWidth;j++, lpSrc++, lpDst++ )
		{
			lpDst->r = lpSrc->a;
			lpDst->g = lpSrc->a;
			lpDst->b = lpSrc->a;
		}
	}

	FBMPResource * lpRes = new FBMPResource( iMaxWidth, iHeight, lpAlphaData, FImageResource::IMAGE_RGB );
	FResourceManager::SharedManager()->SaveResource( sFileName + ATLAS_ALPHA_EDIT_FILE, lpRes );
	delete lpRes;

	FStack::GetInstanceStack()->PopBlock();

	return 1;
}

UI32 FAtlasBuilder::EditAlphaDone()
{
	I32 iMaxWidth;
	lpAtlasCfg->GetFieldValueInt( FAtlasConfig::FIELD_MAXWIDTH, iMaxWidth );

	FImageResource * lpAlphaRes = (FImageResource *)FResourceManager::SharedManager()->CreateResource( sFileName + ATLAS_ALPHA_EDIT_FILE + FString( ".bmp" ) );
	if( iMaxWidth != lpAlphaRes->GetWidth() && iHeight != lpAlphaRes->GetHeight() && lpAlphaRes->GetFormat() != FImageResource::IMAGE_RGB )
	{
		delete lpAlphaRes;
		PUT_ERROR( "Alpha file resolution has changed" );
		return 0;
	}

	RGB * lpSrc = (RGB *)lpAlphaRes->GetData();
	RGBA * lpDst = lpAtlasData;

	for( UI32 i = 0;i < iHeight;i++ )
		for( UI32 j = 0;j < iMaxWidth;j++, lpDst++, lpSrc++ )
			lpDst->a = lpSrc->r;

	delete lpAlphaRes;
	FFile* lpFile = FFile::MakeFile( sFileName + ATLAS_ALPHA_EDIT_FILE + FString( ".bmp" ), FFile::FILE_READ );
	lpFile->Delete();
	FFile::ReleaseFile( lpFile );

	return 1;
}

/*
Сохранение в RLE сжатии
*/
void SaveRLEComp( UI32 iWidth, UI32 iHeight, RGBA * lpData, void * lpRGBAData, UI32 & iDataSize )
{
	UI8 * lpDst = (UI8 *)lpRGBAData;
	RGBA * lpSrc = lpData;
	iDataSize = 0;

	for( UI32 i = 0;i < iHeight*iWidth; )
	{
		UI32 j;
		for( j = 1;(j < iWidth*iHeight)&&(j < ATLAS_RLE_MASK_LEN);j++ )
		{
			if( (lpSrc[0].r != lpSrc[j].r) || (lpSrc[0].g != lpSrc[j].g) || (lpSrc[0].b != lpSrc[j].b) || (lpSrc[0].a != lpSrc[j].a) )
				break;
		}
		if( j == 1 )
		{
			lpDst[0] = 0;
			lpDst[1] = lpSrc->r;
			lpDst[2] = lpSrc->g;
			lpDst[3] = lpSrc->b;
			lpDst[4] = lpSrc->a;

			lpDst += 5;
			lpSrc++;
			i++;
		}
		else
		{
			lpDst[0] = j&ATLAS_RLE_MASK_LEN | ATLAS_RLE_MASK_PACK;
			lpDst[1] = lpSrc->r;
			lpDst[2] = lpSrc->g;
			lpDst[3] = lpSrc->b;
			lpDst[4] = lpSrc->a;
			lpDst += 5;
			lpSrc += (j - 1);
			i += (j - 1);
		}
		iDataSize += 5;
	}
}

/*
Сохранение в JPEG сжатии
*/
void SaveJPEGComp( UI32 iWidth, UI32 iHeight, RGBA * lpData, void * lpRGBData, I32 iQuality, void * lpAlphaData, UI32 & iRGBSize, UI32 & iAlphaSize )
{
	RGB * lpRGB = (RGB *)FStack::GetInstanceStack()->PushBlock( iWidth*iHeight*sizeof( RGB ) );
	UI8 * lpAlpha = (UI8 *)FStack::GetInstanceStack()->PushBlock( iWidth*iHeight*sizeof( UI8 ) );

	for( UI32 i = 0;i < iWidth*iHeight;i++ )
	{
		lpRGB[i].r = lpData[i].r;
		lpRGB[i].g = lpData[i].g;
		lpRGB[i].b = lpData[i].b;
		lpAlpha[i] = lpData[i].a;
	}

	FJPEGResource jRgb( iWidth, iHeight, lpRGB, FImageResource::IMAGE_RGB );
	FJPEGResource jAlpha( iWidth, iHeight, lpAlpha, FImageResource::IMAGE_A );
	jRgb.SetQuality( iQuality );
	jAlpha.SetQuality( iQuality );

	void * lpRGBBuff, * lpAlphaBuff;
	FResourceManager::SharedManager()->SaveResource( &lpRGBBuff, iRGBSize, &jRgb );
	FResourceManager::SharedManager()->SaveResource( &lpAlphaBuff, iAlphaSize, &jAlpha );


	memcpy( lpRGBData, lpRGBBuff, iRGBSize );
	memcpy( lpAlphaData, lpAlphaBuff, iAlphaSize );

	FFree( lpRGBBuff );
	FFree( lpAlphaBuff );

	FStack::GetInstanceStack()->PopBlock();
	FStack::GetInstanceStack()->PopBlock();
}


/*
Сохранение в PNG сжатии в iRGBASize сохраняется размер сжатых RGBA данных, в lpRGBAData собственно сами сжатые данные(lpRGBAData должен солержать уже выделеную область данных)
*/
void SavePNGComp( UI32 iWidth, UI32 iHeight, RGBA * lpData, void * lpRGBAData, UI32 & iRGBASize )
{
	FPNGResource pRgba( iWidth, iHeight, lpData, FImageResource::IMAGE_RGBA );

	void * lpRGBABuff;
	FResourceManager::SharedManager()->SaveResource( &lpRGBABuff, iRGBASize, &pRgba );
	memcpy( lpRGBAData, lpRGBABuff, iRGBASize );

	FFree( lpRGBABuff );
}


UI32 FAtlasBuilder::SaveAtlas()
{
	I32 iMaxWidth;
	FString sPacked, sCompr, sWorkDir, sSaveDir;
	lpAtlasCfg->GetFieldValueInt( FAtlasConfig::FIELD_MAXWIDTH, iMaxWidth );
	lpAtlasCfg->GetFieldValueString( FAtlasConfig::FIELD_PACK, sPacked );
	lpAtlasCfg->GetFieldValueString( FAtlasConfig::FIELD_COMPR, sCompr );
	lpAtlasCfg->GetFieldValueString( FAtlasConfig::FIELD_WORKDIR, sWorkDir );
	lpAtlasCfg->GetFieldValueString( FAtlasConfig::FIELD_SAVEDIR, sSaveDir );

	if( sSaveDir == FString( "\\" ) )
		sSaveDir = sWorkDir;
	FString sSaveFile = sSaveDir + sFileName;

	FAtlasHeader aHeader = { 0 };
	UI32 iAtlasHSize = sizeof( aHeader ), iFrameHSize = sizeof( FFrameHeader ), iAnimHSize = sizeof( FAnimationHeader ), iAddHSize = sizeof( FAddDataHeader );
	aHeader.iVersion = 1;
	aHeader.iFileType = ATLAS_FILE_TYPE;
	aHeader.iCompr = ConstStr2Number( sCompr );
	if( sPacked == FString( "YES" ) )
		aHeader.iFlags |= ATLAS_PACKED;
	aHeader.iTexWidth = iMaxWidth;
	aHeader.iTexHeight = iHeight;

	void * lpRGBAData = NULL, * lpRGBData = NULL, * lpAlphaData = NULL;
	UI32 iRGBASize = 0, iRGBSize = 0, iAlphaSize = 0;
	const CHAR_ * lpRGBName = NULL, * lpAlphaName = NULL;

	if( aHeader.iFlags & ATLAS_PACKED )
	{
		I32 iQuality = 0;
		switch( aHeader.iCompr )
		{
		case ATLAS_COMPR_NONE:
			aHeader.iFlags |= ATLAS_RGBA;
			iRGBASize = iMaxWidth*iHeight*sizeof( RGBA );
			lpRGBAData = (void *)lpAtlasData;
			break;
		case ATLAS_COMPR_JPG:
			aHeader.iFlags |= ATLAS_RGB;
			aHeader.iFlags |= ATLAS_ALPHA;
			lpAtlasCfg->GetFieldValueInt( FAtlasConfig::FIELD_JPEGQUALITY, iQuality );
			lpRGBData = FStack::GetInstanceStack()->PushBlock( iMaxWidth*iHeight*sizeof( RGB ) );
			lpAlphaData = FStack::GetInstanceStack()->PushBlock( iMaxWidth*iHeight*sizeof( UI8 ) );
			SaveJPEGComp( iMaxWidth, iHeight, lpAtlasData, lpRGBData, iQuality, lpAlphaData, iRGBSize, iAlphaSize );
			break;
		case ATLAS_COMPR_PNG:
			aHeader.iFlags |= ATLAS_RGBA;
			lpRGBAData = FStack::GetInstanceStack()->PushBlock( iMaxWidth*iHeight*sizeof( RGBA ) );
			SavePNGComp( iMaxWidth, iHeight, lpAtlasData, lpRGBAData, iRGBASize );
			break;
		case ATLAS_COMPR_RLE:
			aHeader.iFlags |= ATLAS_RGBA;
			lpRGBAData = FStack::GetInstanceStack()->PushBlock( iMaxWidth*iHeight*sizeof( RGBA ) );
			SaveRLEComp( iMaxWidth, iHeight, lpAtlasData, lpRGBAData, iRGBASize );
			break;
		}
		aHeader.iTexAtlasDataStart = iAtlasHSize + iAnimHSize*lpAtlasCfg->GetAnimationCount() + iFrameHSize*lpAtlasCfg->GetFrameCount() + iAddHSize*lpAtlasCfg->GetAddFieldsCount();
		if( aHeader.iFlags & ATLAS_RGBA )
			aHeader.iRGBDataLen = iRGBASize;
		if( aHeader.iFlags & ATLAS_RGB )
			aHeader.iRGBDataLen = iRGBSize;
		if( aHeader.iFlags & ATLAS_ALPHA )
			aHeader.iAlphaDataLen = iAlphaSize;
	}
	else
	{
		FImageResource * lpRGBRes = NULL, * lpAlphaRes = NULL;
		RGB * lpRGBData = (RGB *)FStack::GetInstanceStack()->PushBlock( iMaxWidth*iHeight*sizeof( RGB ) );
		RGB * lpAlphaData = (RGB *)FStack::GetInstanceStack()->PushBlock( iMaxWidth*iHeight*sizeof( RGB ) );
		for( UI32 i = 0;i < (iMaxWidth*iHeight);i++ )
		{
			lpRGBData[i].r = lpAtlasData[i].r;
			lpRGBData[i].g = lpAtlasData[i].g;
			lpRGBData[i].b = lpAtlasData[i].b;

			lpAlphaData[i].r = lpAtlasData[i].a;
			lpAlphaData[i].g = lpAtlasData[i].a;
			lpAlphaData[i].b = lpAtlasData[i].a;
		}

		I32 iQuality = 0;
		switch( aHeader.iCompr )
		{
		case ATLAS_COMPR_RLE:
		case ATLAS_COMPR_NONE:
			lpRGBRes = new FBMPResource( iMaxWidth, iHeight, lpRGBData, FImageResource::IMAGE_RGB );
			lpAlphaRes = new FBMPResource( iMaxWidth, iHeight, lpAlphaData, FImageResource::IMAGE_RGB );
			break;
		case ATLAS_COMPR_JPG:
			lpAtlasCfg->GetFieldValueInt( FAtlasConfig::FIELD_JPEGQUALITY, iQuality );
			lpRGBRes = new FJPEGResource( iMaxWidth, iHeight, lpRGBData, FImageResource::IMAGE_RGB );
			lpAlphaRes = new FJPEGResource( iMaxWidth, iHeight, lpAlphaData, FImageResource::IMAGE_RGB );
			((FJPEGResource *)lpRGBRes)->SetQuality( iQuality );
			((FJPEGResource *)lpAlphaRes)->SetQuality( iQuality );
			break;
		case ATLAS_COMPR_PNG:
			lpRGBRes = new FPNGResource( iMaxWidth, iHeight, lpRGBData, FImageResource::IMAGE_RGB );
			lpAlphaRes = new FPNGResource( iMaxWidth, iHeight, lpAlphaData, FImageResource::IMAGE_RGB );
			break;
		}

		lpRGBName = FStringAllocator::GetAllocatorInst()->AllocateStr( sSaveFile.GetName() + FString( "." ) + lpRGBRes->GetExt() );
		lpAlphaName = FStringAllocator::GetAllocatorInst()->AllocateStr( sSaveFile.GetName() +FString( "_a." ) + lpAlphaRes->GetExt() );

		FResourceManager::SharedManager()->SaveResource( sSaveFile, lpRGBRes );
		FResourceManager::SharedManager()->SaveResource( sSaveFile + FString( "_a" ), lpAlphaRes );

		if( lpRGBRes )
			delete lpRGBRes;
		if( lpAlphaRes )
			delete lpAlphaRes;

		FStack::GetInstanceStack()->PopBlock();
		FStack::GetInstanceStack()->PopBlock();
	}


	aHeader.iAnimationCount = lpAtlasCfg->GetAnimationCount();
	aHeader.iAnimationDataStart = iAtlasHSize;
	aHeader.iFrameCount = lpAtlasCfg->GetFrameCount();
	aHeader.iFrameDataStart = aHeader.iAnimationDataStart + iAnimHSize*lpAtlasCfg->GetAnimationCount();
	aHeader.iAddDataCount = lpAtlasCfg->GetAddFieldsCount();
	aHeader.iAdditionalDataStart = aHeader.iFrameDataStart + iFrameHSize*lpAtlasCfg->GetFrameCount();

	UI32 iStrTableStart = iAtlasHSize + iAnimHSize*lpAtlasCfg->GetAnimationCount() + iFrameHSize*lpAtlasCfg->GetFrameCount() + iAddHSize*lpAtlasCfg->GetAddFieldsCount() + iRGBASize + iRGBSize + iAlphaSize;//вычисляем начало строковых данных в файле
	aHeader.iStringDataStart = iStrTableStart;

	if( lpRGBName )
	{
		aHeader.iFlags |= ATLAS_RGB;
		aHeader.iAtlasNameOff = iStrTableStart + FStringAllocator::GetAllocatorInst()->GetOffset( lpRGBName );
		aHeader.iAtlasNameLen = strlen( lpRGBName );
	}

	if( lpAlphaName )
	{
		aHeader.iFlags |= ATLAS_ALPHA;
		aHeader.iAtlasAlphaNameOff = iStrTableStart + FStringAllocator::GetAllocatorInst()->GetOffset( lpAlphaName );
		aHeader.iAtlasAlphaNameLen = strlen( lpAlphaName );
	}

	FFrameHeader * lpFrameHeaders = (FFrameHeader *)FStack::GetInstanceStack()->PushBlock( lpAtlasCfg->GetFrameCount()*iFrameHSize );
	FAnimationHeader * lpAnimHeaders = (FAnimationHeader *)FStack::GetInstanceStack()->PushBlock( lpAtlasCfg->GetAnimationCount()*iAnimHSize );
	FAddDataHeader * lpAddHeaders = (FAddDataHeader *)FStack::GetInstanceStack()->PushBlock( lpAtlasCfg->GetAddFieldsCount()*iAddHSize );

	FFrameIterator iFrameIt = lFrameList.Begin();
	FFrameHeader * lpCurFrameH = lpFrameHeaders;
	for(;iFrameIt != lFrameList.End();iFrameIt++, lpCurFrameH++ )
	{
		UI32 x, y;
		F32 u0, v0, u1, v1;
		(*iFrameIt)->GetParams( x, y, u0, v0, u1, v1 );

		lpCurFrameH->iFrameNum = (*iFrameIt)->GetIndex();
		lpCurFrameH->x = x;
		lpCurFrameH->y = y;
		lpCurFrameH->u0 = u0;
		lpCurFrameH->v0 = v0;
		lpCurFrameH->u1 = u1;
		lpCurFrameH->v1 = v1;
		lpCurFrameH->iWidth = (*iFrameIt)->GetWidth();
		lpCurFrameH->iHeight = (*iFrameIt)->GetHeight();
		lpCurFrameH->iNameOff = FStringAllocator::GetAllocatorInst()->GetOffset( (*iFrameIt)->GetFrameName() ) + iStrTableStart;//вычисляем начало имени кадра в файле
		lpCurFrameH->iNameLen = strlen( (*iFrameIt)->GetFrameName() );
	}

	FAnimationHeader * lpCurAnimH = lpAnimHeaders;
	for( UI32 i = 0;i < lpAtlasCfg->GetAnimationCount();i++, lpCurAnimH++ )
	{
		FAnimationParams aAnimParams = { 0 };
		lpAtlasCfg->GetAnimParams( i, aAnimParams );
		CHAR_ * lpAnimName = FStringAllocator::GetAllocatorInst()->AllocateStr( aAnimParams.sAnimName );

		lpCurAnimH->iStartFrame = aAnimParams.iStartFrm;
		lpCurAnimH->iEndFrame = aAnimParams.iEndFrm;
		lpCurAnimH->iNameOff = FStringAllocator::GetAllocatorInst()->GetOffset( lpAnimName ) + iStrTableStart;//вычисляем начало имени анимации
		lpCurAnimH->iNameLen = strlen( lpAnimName );
		if( aAnimParams.bLooped )
			lpCurAnimH->iAnimFlags |= ATLAS_ANIM_LOOPED;
	}

	FAddDataHeader * lpCurAddH = lpAddHeaders;
	for( UI32 i = 0;i < lpAtlasCfg->GetAddFieldsCount();i++, lpCurAddH++ )
	{
		FAddField aAddField = { 0 };
		lpAtlasCfg->GetAddData( i, aAddField );
		CHAR_ * lpAddName = FStringAllocator::GetAllocatorInst()->AllocateStr( aAddField.sFieldName );

		lpCurAddH->iNameOff = FStringAllocator::GetAllocatorInst()->GetOffset( lpAddName ) + iStrTableStart;//вычисляем начало имени доп данных
		lpCurAddH->iNameLen = strlen( lpAddName );
		CHAR_ * lpStr;
		switch( aAddField.iValType )
		{
		case FAtlasConfig::TYPE_INT:
			lpCurAddH->iValType = ATLAS_TYPE_INT;
			lpCurAddH->iInt = aAddField.iInt;
			break;
		case FAtlasConfig::TYPE_FLOAT:
			lpCurAddH->iValType = ATLAS_TYPE_FLOAT;
			lpCurAddH->fFloat = aAddField.fFloat;
			break;
		case FAtlasConfig::TYPE_STR:
			lpCurAddH->iValType = ATLAS_TYPE_STR;
			lpStr = FStringAllocator::GetAllocatorInst()->AllocateStr( *aAddField.lpStr );
			lpCurAddH->iStrOff = FStringAllocator::GetAllocatorInst()->GetOffset( lpStr ) + iStrTableStart;
			lpCurAddH->iStrLen = strlen( lpStr );
			break;
		}
	}
	/*
	Сохраняем данные в файле, снчало заголовок, потом анимационные данные, потом данные кадров, потом дополнительные данные, далее атлас и строковую таблицу
	*/
	FFile * lpFile = FFile::MakeFile( sSaveFile + ATLAS_FILE_EXT, FFile::FILE_CREATE );
	if( lpFile->OpenSucc() )
	{
		lpFile->Write( (void *)&aHeader, sizeof( aHeader ) );
		lpFile->Write( lpAnimHeaders, sizeof( FAnimationHeader )*lpAtlasCfg->GetAnimationCount() );
		lpFile->Write( lpFrameHeaders, sizeof( FFrameHeader ) *lpAtlasCfg->GetFrameCount() );
		lpFile->Write( lpAddHeaders, sizeof( FAddDataHeader ) *lpAtlasCfg->GetAddFieldsCount() );

		if( aHeader.iFlags & ATLAS_PACKED )
		{
			if( lpRGBAData )
				lpFile->Write( lpRGBAData, iRGBASize );
			if( lpRGBData )
				lpFile->Write( lpRGBData, iRGBSize );
			if( lpAlphaData )
				lpFile->Write( lpAlphaData, iAlphaSize );
		}
		FStringAllocator::GetAllocatorInst()->WriteToFile( lpFile );//записываем строковые данные в файл
	}
	else
	{
		PUT_ERROR( "Can't create file \"%s\"", (sSaveFile + ATLAS_FILE_EXT).GetChar() );
	}

	delete lpFile;
	FStack::GetInstanceStack()->PopBlock();
	FStack::GetInstanceStack()->PopBlock();
	FStack::GetInstanceStack()->PopBlock();

	if( aHeader.iFlags & ATLAS_PACKED )
	{
		if( lpRGBAData && (lpAtlasData != lpRGBAData) )
			FStack::GetInstanceStack()->PopBlock();
		if( lpRGBData )
			FStack::GetInstanceStack()->PopBlock();
		if( lpAlphaData )
			FStack::GetInstanceStack()->PopBlock();
	}

	return 1;
}


void FAtlasBuilder::ReallocData( UI32 iWidth, UI32 iEstHeight )
{
	RGBA * lpNewData = (RGBA *)FMalloc( iWidth*iEstHeight*sizeof( RGBA ) );
	RGBA * lpDst = lpNewData;

	FString sBackColor;
	lpAtlasCfg->GetFieldValueString( FAtlasConfig::FIELD_BACKCOLOR, sBackColor );
	RGBA rBackColor;
	lpAtlasCfg->Constant2Color( sBackColor, rBackColor );

	for( UI32 i = 0;i < iEstHeight;i++ )
	{
		for( UI32 j = 0;j < iWidth;j++, lpDst++ )
		{
			lpDst->r = rBackColor.r;
			lpDst->g = rBackColor.g;
			lpDst->b = rBackColor.b;
			lpDst->a = 255;
		}
	}

	if( lpAtlasData )
	{
		RGBA * lpDst = lpNewData, * lpSrc = lpAtlasData;

		for( UI32 i = 0;i < iHeight;i++ )
		{
			for( UI32 j = 0;j < iWidth;j++, lpSrc++, lpDst++ )
			{
				lpDst->r = lpSrc->r;
				lpDst->g = lpSrc->g;
				lpDst->b = lpSrc->b;
				lpDst->a = lpSrc->a;
			}
		}

		FFree( lpAtlasData );
	}
	lpAtlasData = lpNewData;
	iHeight = iEstHeight;
}

void FAtlasBuilder::Clear()
{
	lpAtlasCfg->Close();
	if( lpAtlasData )
		FFree( lpAtlasData );
	lpAtlasData = NULL;
	FFrameIterator iIt = lFrameList.Begin();
	for(;iIt != lFrameList.End();iIt++ )
		delete *iIt;
	lFrameList.Clear();
	FStringAllocator::GetAllocatorInst()->Clear();
}

bool FAtlasBuilder::IsPacked()const
{
	FString sPack;
	lpAtlasCfg->GetFieldValueString( FAtlasConfig::FIELD_PACK, sPack );
	if( sPack == ATLAS_CONST_YES )
		return true;
	return false;
}

void FAtlasBuilder::GetHelpInfo( CHAR_ * lpBuffer )
{
	lpAtlasCfg->GetHelpInfo( lpBuffer );
}