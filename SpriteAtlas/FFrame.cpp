#include "FFrame.h"
#include "FResourceManager.h"
#include "FImageResource.h"
#include "FStack.h"
#include "FStringAllocator.h"
#include "FConsole.h"
#include "FAtlasConfig.h"
#include <string.h>



FFrame::FFrame( const FString & sName, UI32 x, UI32 y, UI32 iSrcWidth, UI32 iSrcHeight, UI32 iDstWidth, UI32 iDstHeight, UI32 iIndex, FAtlasConfig * lpAtlasCfg ) : lpData( NULL ), iWidth( iDstWidth ), iHeight( iDstHeight )
	, lpFrameName( NULL ), iIndex( iIndex ), bWrited( false ), x( 0 ), y( 0 ), u0( 0.f ), v0( 0.f ), u1( 0.f ), v1( 0.f ), 
	lpAtlasCfg( lpAtlasCfg ), bLoaded( true )
{
	FImageResource * lpRes = (FImageResource *)FResourceManager::SharedManager()->CreateResource( sName );
	if( !lpRes )
	{
		PUT_ERROR( "Can't find iamge resource with name: %s", sName.GetChar() );
		bLoaded = false;
		return;
	}

	if( x == 0 && y == 0 && iSrcWidth == 0 && iSrcHeight == 0 && iDstWidth != 0 && iDstHeight != 0 )
	{
		iSrcWidth = lpRes->GetWidth();
		iSrcHeight = lpRes->GetHeight();
	}
	else if( iSrcWidth == 0 && iSrcHeight == 0 && iDstWidth == 0 && iDstHeight == 0 )//если все параметры равны нулю значит берется все изображение полностью в качестве кадра
	{
		iDstWidth = lpRes->GetWidth();
		iDstHeight = lpRes->GetHeight();
		iSrcWidth = iDstWidth;
		iSrcHeight = iDstHeight;
		iWidth = iDstWidth;
		iHeight = iDstHeight;
	}
	///проверяем выход за границы изображения
	if( (lpRes->GetWidth() < (x + iSrcWidth)) || (lpRes->GetHeight() < (y + iSrcHeight)) )
	{
		PUT_ERROR( "Dimension of frame is too large then source image, file name: %s", sName.GetChar() );
		delete lpRes;
		return;
	}

	if( (iSrcWidth < iDstWidth) || (iSrcHeight < iDstHeight) )
	{
		PUT_ERROR( "Dimension of scale frame is too large then source frame, file name: %s", sName.GetChar() );
		delete lpRes;
		return;
	}

	FString sAlpha;
	lpAtlasCfg->GetFieldValueString( FAtlasConfig::FIELD_PRODALPHA, sAlpha );

	bool bAlphaExist = false;//альфа данные имеются
	//проверяем наличие аьфа данных в случае если в настройке установлена автоматическая генерация данных либо альфа канал запрещено генерировать
	if( (sAlpha == ATLAS_CONST_NO) || (sAlpha == ATLAS_CONST_AUTO) )
	{
		if( lpRes->GetBpp() != 4 )
		{
			FString sAlphaName = sName.GetPath() + sName.GetName() + "_a" + sName.GetExt( true );
			FImageResource * lpAlphaRes = (FImageResource *)FResourceManager::SharedManager()->CreateResource( sAlphaName );
			if( lpAlphaRes )
			{
				bAlphaExist = true;
				lpRes->AddAlphaData( lpAlphaRes );
				delete lpAlphaRes;
			}
			else if( sAlpha == ATLAS_CONST_AUTO )
			{
				PUT_MESSAGE( "Alpha data for file \"%s\" not found generate alpha data", sName.GetChar() );
			}
		}
		else
			bAlphaExist = true;
	}

	RGBA * lpSrcData = (RGBA *)FStack::GetInstanceStack()->PushBlock( iSrcWidth*iSrcHeight*sizeof( RGBA ) );
	RGBA * lpSrc = lpSrcData;
	for( I32 i = y;i < (y + iSrcHeight);i++ )
	{
		PIXCOMP * lpTmp = (PIXCOMP *)lpRes->GetData() + i*lpRes->GetWidth()*lpRes->GetBpp() + x*lpRes->GetBpp();
		for( I32 j = 0;j < iSrcWidth;j++, lpSrc++, lpTmp += lpRes->GetBpp() )
		{
			switch( lpRes->GetBpp() )
			{
			case 1:
				lpSrc->r = *lpTmp;
				lpSrc->g = 0;
				lpSrc->b = 0;
				lpSrc->a = 255;
				break;
			case 3:
				lpSrc->r = *lpTmp;
				lpSrc->g = *(lpTmp + 1);
				lpSrc->b = *(lpTmp + 2);
				lpSrc->a = 255;
				break;
			case 4:
				lpSrc->r = *lpTmp;
				lpSrc->g = *(lpTmp + 1);
				lpSrc->b = *(lpTmp + 2);
				if( bAlphaExist )
					lpSrc->a = *(lpTmp + 3);
				else
					lpSrc->a = 255;
				break;
			}
		}
	}

	//вычисление альфа данных
	if( ((sAlpha != ATLAS_CONST_NO) && !bAlphaExist) || (sAlpha == ATLAS_CONST_YES) )
		CalcAlpha( lpSrcData, iSrcWidth, iSrcHeight );

	lpData = (RGBA *)FMalloc( iDstWidth*iDstHeight*sizeof( RGBA ) );
	//масштабирование изображения если необходимо
	if( (iSrcWidth != iDstWidth) || (iSrcHeight != iDstHeight) )
		Scale( lpSrcData, iSrcWidth, iSrcHeight, lpData, iDstWidth, iDstHeight );
	else
		memcpy( lpData, lpSrcData, iDstWidth*iDstHeight*sizeof( RGBA ) );

	FStack::GetInstanceStack()->PopBlock();
	delete lpRes;

	lpFrameName = FStringAllocator::GetAllocatorInst()->AllocateStr( sName.GetName() );
}

FFrame::~FFrame()
{
	if( lpData )
		FFree( lpData );
}

//Вычисление альфа канала осуществляется по яркости изображения, задаваемый параметр iMinLig задается границу для определения невидимости пикселя
void FFrame::CalcAlpha( RGBA * lpData, UI32 iWidth, UI32 iHeight )
{
	I32 iMinLig, iMinAlpha, iMaxAlpha;
	lpAtlasCfg->GetFieldValueInt( FAtlasConfig::FIELD_BRIGHT, iMinLig );
	lpAtlasCfg->GetFieldValueInt( FAtlasConfig::FIELD_MINALPHA, iMinAlpha );
	lpAtlasCfg->GetFieldValueInt( FAtlasConfig::FIELD_MAXALPHA, iMaxAlpha );

	for( int i = 0;i < iHeight;i++ )
	{
		RGBA * lpSrc = lpData + i*iWidth;

		for( int j = 0;j < iWidth;j++, lpSrc++ )
		{
			I32 iMax = max( lpSrc->r, max( lpSrc->g, lpSrc->b ) );
			I32 iMin = min( lpSrc->r, min( lpSrc->g, lpSrc->b ) );
			I32 iLig = (iMin + iMax)/2;
			if( iLig < iMinLig )
				lpSrc->a = max( iLig*7, iMinAlpha );
			else
				lpSrc->a = iMaxAlpha;
		}
	}

}

I32 FFrame::SumR( I32 v, I32 u, I32 iStepX, I32 iStepY, I32 iSrcW, I32 iSrcH, RGBA * lpSrc )
{
	I32 iStartX = v - iStepX;
	if( iStartX < 0 )
		iStartX = 0;

	I32 iStartY = u - iStepY;
	if( iStartY < 0 )
		iStartY = 0;

	I32 iEndX = v + iStepX;
	if( iEndX > iSrcW )
		iEndX = iSrcW;

	I32 iEndY = u + iStepY;
	if( iEndY > iSrcH )
		iEndY = iSrcH;

	I32 iSumR = 0, iPixelCount = 0;
	for( I32 i = iStartY;i < iEndY;i++ )
	{
		RGBA * lpSrc0 = lpSrc + i*iSrcW + iStartX;
		for( I32 j = iStartX;j < iEndX;j++ )
		{
			iSumR += lpSrc0->r;
			iPixelCount++;
			lpSrc0++;
		}
	}

	return iSumR/iPixelCount;
}

I32 FFrame::SumG( I32 v, I32 u, I32 iStepX, I32 iStepY, I32 iSrcW, I32 iSrcH, RGBA * lpSrc )
{
	I32 iStartX = v - iStepX;
	if( iStartX < 0 )
		iStartX = 0;

	I32 iStartY = u - iStepY;
	if( iStartY < 0 )
		iStartY = 0;

	I32 iEndX = v + iStepX;
	if( iEndX > iSrcW )
		iEndX = iSrcW;

	I32 iEndY = u + iStepY;
	if( iEndY > iSrcH )
		iEndY = iSrcH;

	I32 iSumG = 0, iPixelCount = 0;
	for( I32 i = iStartY;i < iEndY;i++ )
	{
		RGBA * lpSrc0 = lpSrc + i*iSrcW + iStartX;
		for( I32 j = iStartX;j < iEndX;j++ )
		{
			iSumG += lpSrc0->g;
			iPixelCount++;
			lpSrc0++;
		}
	}

	return iSumG/iPixelCount;
}

I32 FFrame::SumB( I32 v, I32 u, I32 iStepX, I32 iStepY, I32 iSrcW, I32 iSrcH, RGBA * lpSrc )
{
	I32 iStartX = v - iStepX;
	if( iStartX < 0 )
		iStartX = 0;

	I32 iStartY = u - iStepY;
	if( iStartY < 0 )
		iStartY = 0;

	I32 iEndX = v + iStepX;
	if( iEndX > iSrcW )
		iEndX = iSrcW;

	I32 iEndY = u + iStepY;
	if( iEndY > iSrcH )
		iEndY = iSrcH;

	I32 iSumB = 0, iPixelCount = 0;
	for( I32 i = iStartY;i < iEndY;i++ )
	{
		RGBA * lpSrc0 = lpSrc + i*iSrcW + iStartX;
		for( I32 j = iStartX;j < iEndX;j++ )
		{
			iSumB += lpSrc0->b;
			iPixelCount++;
			lpSrc0++;
		}
	}

	return iSumB/iPixelCount;
}

I32 FFrame::SumA( I32 v, I32 u, I32 iStepX, I32 iStepY, I32 iSrcW, I32 iSrcH, RGBA * lpSrc )
{
	I32 iStartX = v - iStepX;
	if( iStartX < 0 )
		iStartX = 0;

	I32 iStartY = u - iStepY;
	if( iStartY < 0 )
		iStartY = 0;

	I32 iEndX = v + iStepX;
	if( iEndX > iSrcW )
		iEndX = iSrcW;

	I32 iEndY = u + iStepY;
	if( iEndY > iSrcH )
		iEndY = iSrcH;

	I32 iSumA = 0, iPixelCount = 0;
	for( I32 i = iStartY;i < iEndY;i++ )
	{
		RGBA * lpSrc0 = lpSrc + i*iSrcW + iStartX;
		for( I32 j = iStartX;j < iEndX;j++ )
		{
			iSumA += lpSrc0->a;
			iPixelCount++;
			lpSrc0++;
		}
	}

	return iSumA/iPixelCount;
}

void FFrame::Scale( RGBA * lpSrc, UI32 iSrcW, UI32 iSrcH, RGBA * lpDst, UI32 iDstW, UI32 iDstH )
{
	F32 fStepX = F32(iSrcW)/F32(iDstW),
		fStepY = F32(iSrcH)/F32(iDstH);

	I32 iStepX = iSrcW/iDstW,
		iStepY = iSrcH/iDstH;

	F32 fErrX = fStepX - F32(iStepX),
		fErrY = fStepY - F32(iStepY);

	F32 fErrAuxX = 0, fErrAuxY = 0;

	RGBA * lpDst0 = lpDst;
	I32 iAuxY = 0;

	I32 u = 0, v = 0;
	for( I32 i = 0;i < iDstH;i++, u += (iStepY + iAuxY) )
	{
		fErrAuxY += fErrY;
		if( fErrAuxY >= 1.f )
		{
			iAuxY = 1;
			fErrAuxY -= 1.f;
		}
		else
			iAuxY = 0;

		I32 iAuxX = 0;
		v = 0;
		for( I32 j = 0;j < iDstW;j++, v += (iStepX + iAuxX), lpDst0++ )
		{
			fErrAuxX += fErrX;
			if( fErrAuxX >= 1.f )
			{
				iAuxX = 1;
				fErrAuxX -= 1.f;
			}
			else
				iAuxX = 0;

			lpDst0->r = SumR( v, u, iStepX + iAuxX, iStepY + iAuxY, iSrcW, iSrcH, lpSrc );
			lpDst0->g = SumG( v, u, iStepX + iAuxX, iStepY + iAuxY, iSrcW, iSrcH, lpSrc );
			lpDst0->b = SumB( v, u, iStepX + iAuxX, iStepY + iAuxY, iSrcW, iSrcH, lpSrc );
			lpDst0->a = SumA( v, u, iStepX + iAuxX, iStepY + iAuxY, iSrcW, iSrcH, lpSrc );
		}
	}
}

void FFrame::WriteToDataRGB( RGB * lpDstData, UI32 x, UI32 y, UI32 iWidth0, UI32 iHeight0 )
{
	for( I32 i = 0;i < iHeight;i++ )
	{
		RGB * lpDst = lpDstData + (i + y)*iWidth0 + x;
		RGBA * lpSrc = lpData + i*iWidth;
		for( I32 j = 0;j < iWidth;j++, lpDst++, lpSrc++ )
		{
			lpDst->r = lpSrc->r;
			lpDst->g = lpSrc->g;
			lpDst->b = lpSrc->b;
		}
	}
}

void FFrame::WriteToDataRGBA( RGBA * lpDstData, UI32 x, UI32 y, UI32 iWidth0, UI32 iHeight0 )
{
	this->x = x;
	this->y = y;
	this->u0 = F32( x )/F32( iWidth0 );
	this->v0 = F32( y )/F32( iHeight0 );
	this->u1 = F32( x + GetWidth() )/F32( iWidth0 );
	this->v1 = F32( y + GetHeight() )/F32( iHeight0 );

	for( I32 i = 0;i < iHeight;i++ )
	{
		RGBA * lpDst = lpDstData + (i + y)*iWidth0 + x;
		RGBA * lpSrc = lpData + i*iWidth;
		for( I32 j = 0;j < iWidth;j++, lpDst++, lpSrc++ )
		{
			lpDst->r = lpSrc->r;
			lpDst->g = lpSrc->g;
			lpDst->b = lpSrc->b;
			lpDst->a = lpSrc->a;
		}
	}
}

void FFrame::WriteToDataAlpha( PIXCOMP * lpDstData, UI32 x, UI32 y, UI32 iWidth0, UI32 iHeight0 )
{
	for( I32 i = 0;i < iHeight;i++ )
	{
		PIXCOMP * lpDst = lpDstData + (i + y)*iWidth0 + x;
		RGBA * lpSrc = lpData + i*iWidth;
		for( I32 j = 0;j < iWidth;j++, lpDst++, lpSrc++ )
			*lpDst = lpSrc->a;
	}
}

UI32 FFrame::GetWidth()const
{
	return iWidth;
}

UI32 FFrame::GetHeight()const
{
	return iHeight;
}

const CHAR_ * FFrame::GetFrameName()const
{
	return lpFrameName;
}

UI32 FFrame::GetIndex()const
{
	return iIndex;
}

void FFrame::GetParams( UI32 & x_, UI32 & y_, F32 & u0_, F32 & v0_, F32 & u1_, F32 & v1_ )
{
	x_ = x;
	y_ = y;
	u0_ = u0;
	v0_ = v0;
	u1_ = u1;
	v1_ = v1;
}


bool FFrame::IsWrited()const
{
	return bWrited;
}

bool FFrame::IsLoaded()const
{
	return bLoaded;
}


void FFrame::SetWrited( bool bWrited0 )
{
	bWrited = bWrited0;
}
