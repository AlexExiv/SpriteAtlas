#include "FTGAResource.h"

#define PACKED_DATA 0x0A
#define UNPACKED_DATA 0x02
#define PACKED_BLOCK_MASK 0x80
#define BLOCK_LENGTH_MASK 0x7F

struct TGAHeader
{
	CHAR_ cIdLength;
	CHAR_ cColorMap;
	CHAR_ cDataType;
	CHAR_ cColorMapInfo[5];
	I16 iStartX;
	I16 iStartY;
	I16 iWidth;
	I16 iHeight;
	CHAR_ cBpp;
	CHAR_ cDiscrp;
};



FTGAResource::FTGAResource() : FImageResource( FString( "tga" ) )
{
}

FTGAResource::FTGAResource( void * lpData0, UI32 iDataLen ) : FImageResource( lpData0, FString( "tga" ) )
{
	this->Decode( lpData0, iDataLen );
}
FTGAResource::~FTGAResource()
{

}

void FTGAResource::Decode( void * lpDataSrc, UI32 iDataLen )
{
	TGAHeader * lpHeader = (TGAHeader *) lpDataSrc;

	iWidth = lpHeader->iWidth;
	iHeight = lpHeader->iHeight;
	lpData = FMalloc( iWidth*iHeight*sizeof( RGBA ) );
	RGBA * lpDst = (RGBA *)lpData;
	UI8 * lpSrc = (UI8 *)lpDataSrc + sizeof( TGAHeader ) + lpHeader->cIdLength;

	if( lpHeader->cDataType == UNPACKED_DATA )
	{
		for( I32 i = 0;i < iWidth*iHeight;i++, lpDst++ )
		{
			lpDst->b = *lpSrc++;
			lpDst->g = *lpSrc++;
			lpDst->r = *lpSrc++;
			if( lpHeader->cBpp == 32 )
				lpDst->a = *lpSrc++;
			else
				lpDst->a = 255;
		}
	}
	else if( lpHeader->cDataType == PACKED_DATA )
	{
		for( I32 i = 0;i < iWidth*iHeight;)
		{
			UI8 cPackedBlock = *lpSrc & PACKED_BLOCK_MASK;
			UI8 cBlockLength = *lpSrc++ & BLOCK_LENGTH_MASK;
			if( cPackedBlock )
			{
				UI8 b = *lpSrc++,
					g = *lpSrc++,
					r = *lpSrc++,
					a = 255;

				if( lpHeader->cBpp == 32 )
					a = *lpSrc++;

				for( I32 j = 0;j < cBlockLength;j++, lpDst++, i++ )
				{
					lpDst->r = r;
					lpDst->g = g;
					lpDst->b = b;
					lpDst->a = a;
				}
			}
			else
			{
				lpDst->r = *lpSrc++;
				lpDst->g = *lpSrc++;
				lpDst->b = *lpSrc++;
				if( lpHeader->cBpp == 32 )
					lpDst->a = *lpSrc++;
				else
					lpDst->a = 255;
				lpDst++;
				i++;
			}
		}
	}

	iFormat = IMAGE_RGBA;
}

FResource * FTGAResource::Make( void * lpData, UI32 iDataLen )
{
	return new FTGAResource( lpData, iDataLen );
}

