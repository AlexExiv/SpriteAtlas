#include "FImageResource.h"
#include "FResourceManager.h"


FImageResource::FImageResource( const FString & sExtStr ) : FResource( sExtStr, "Image" )
{

}

FImageResource::FImageResource( void * lpData, const FString & sExtStr ) : FResource( lpData, sExtStr ), iFormat( IMAGE_UNKNOWN )
{
}

FImageResource::FImageResource( UI32 iWidth, UI32 iHeight, void * lpData, UI32 iFormat, const FString & sExtStr  ): iWidth( iWidth ), 
	iHeight( iHeight ), iFormat( iFormat ), FResource( lpData, sExtStr )
{
	bRemoveData = false;
	switch( iFormat )
	{
	case IMAGE_RGB:
		iBpp = 3;
		break;
	case IMAGE_RGBX:
	case IMAGE_RGBA:
		iBpp = 4;
		break;
	case IMAGE_A:
	case IMAGE_L:
		iBpp = 1;
		break;
	case IMAGE_DXT1:
		iBpp = 3;
		break;
	case IMAGE_DXT2:
	case IMAGE_DXT3:
	case IMAGE_DXT4:
	case IMAGE_DXT5:
		iBpp = 4;
		break;
	};
}

FImageResource::~FImageResource()
{
}

UI32 FImageResource::GetWidth()const
{
	return iWidth;
}

UI32 FImageResource::GetHeight()const
{
	return iHeight;
}

UI32 FImageResource::GetFormat()const
{
	return iFormat;
}

UI32 FImageResource::GetBpp()const
{
	return iBpp;
}

void * FImageResource::GetDataLevel( I32 iLevel )const
{
	return lpData;
}

UI32 FImageResource::GetLevelSize( I32 iLevel )const
{
	return 0;
}

bool FImageResource::IsCompressed()const
{
	return (iFormat == IMAGE_DXT1)||(iFormat == IMAGE_DXT2)||(iFormat == IMAGE_DXT3)||(iFormat == IMAGE_DXT4)||(iFormat == IMAGE_DXT5);
}


void FImageResource::AddAlphaData( void * lpAlphaData, I32 iFormat )
{
	if( iFormat == IMAGE_RGBA )
	{
		RGBA * lpDst = (RGBA *)GetData();
		RGBA * lpSrc = (RGBA *)lpAlphaData;

		for( I32 i = 0;i < iWidth*iHeight;i++, lpDst++, lpSrc++ )
			lpDst->a = lpSrc->r;
	}
	else if( iFormat == IMAGE_A )
	{
		RGBA * lpDst = (RGBA *)GetData();
		UI8 * lpSrc = (UI8 *)lpAlphaData;

		for( I32 i = 0;i < iWidth*iHeight;i++, lpDst++, lpSrc++ )
			lpDst->a = *lpSrc;
	}
}

void FImageResource::AddAlphaData( FImageResource * lpImage )
{
	if( (lpImage->GetWidth() != iWidth) || (lpImage->GetHeight() != iHeight) )
		return;

	RGBA * lpSrc = (RGBA *)lpImage->GetData();
	RGBA * lpDst = (RGBA *)lpData;

	for( I32 i = 0;i < (iHeight*iWidth);i++, lpSrc++, lpDst++ )
		lpDst->a = lpSrc->r;
	
}

FResource * FImageResource::Make( void * lpData, UI32 iDataLen )
{
	return NULL;
}