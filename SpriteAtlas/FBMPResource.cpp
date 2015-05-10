#include <windows.h>
#include "FBMPResource.h"
#include "FFile.h"



FBMPResource::FBMPResource() : FImageResource( FString( "bmp" ) )
{
}

FBMPResource::FBMPResource( void * lpData0, UI32 iDataLen ) : FImageResource( lpData0, FString( "bmp" ) )
{
	Decode( lpData0, iDataLen );
}

FBMPResource::FBMPResource( UI32 iWidth, UI32 iHeight, void * lpData, UI32 iFormat ) : FImageResource( iWidth, iHeight, lpData, iFormat, FString( "bmp" ) )
{
}

FBMPResource::~FBMPResource()
{
}

void FBMPResource::Decode( void * lpData0, UI32 iDataLen )
{
	BITMAPFILEHEADER * lpFileBmp = (BITMAPFILEHEADER *)lpData0;
	BITMAPINFOHEADER * lpBmp = (BITMAPINFOHEADER *)((I8 *)lpData0 + sizeof(BITMAPFILEHEADER));

	I8 * lpBmpData = (I8 *)lpData0 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	iWidth = lpBmp->biWidth;
	iHeight = lpBmp->biHeight;
	iFormat = IMAGE_RGB;
	iBpp = 3;

	lpData = FMalloc( iWidth * iHeight * sizeof( RGB ) );
	RGB * lpDst = (RGB *)lpData;

	for( I32 i = 0;i < iHeight;i++ )
	{
		RGB * lpSrc = (RGB *)(lpBmpData + (lpBmp->biHeight - i - 1)*DWORD_ALIG( lpBmp->biWidth ));
		for( I32 j = 0;j < iWidth;j++ )
		{
			lpDst->r = lpSrc->b;
			lpDst->g = lpSrc->g;
			lpDst->b = lpSrc->r;

			lpDst++;
			lpSrc++;
		}
	}
}

char * MakeDwordAligImage( UI32 iWidth, UI32 iHeight, void * lpData, UI32 iFormat )
{
	char * lpAligImage = (char *) FMalloc( DWORD_ALIG(iWidth) * iHeight );
	if( iFormat == FImageResource::IMAGE_RGBA )
	{
		RGBA * lpSrc = (RGBA *)lpData;
		for( int i = 0;i < iHeight;i++ )
		{
			RGB * lpDst = (RGB *)(lpAligImage + (iHeight - i - 1)*DWORD_ALIG(iWidth));
			for( int j = 0;j < iWidth;j++ )
			{
				I32 iData = *(I32 *)lpSrc;
				lpDst->b = lpSrc->r;
				lpDst->g = lpSrc->g;
				lpDst->r = lpSrc->b;
				lpSrc++;
				lpDst++;
			}
		}
	}
	else if( iFormat == FImageResource::IMAGE_RGB )
	{
		RGB * lpSrc = (RGB *)lpData;
		for( int i = 0;i < iHeight;i++ )
		{
			RGB * lpDst = (RGB *)(lpAligImage + (iHeight - i - 1)*DWORD_ALIG(iWidth));
			for( int j = 0;j < iWidth;j++ )
			{
				I32 iData = *(I32 *)lpSrc;
				lpDst->b = lpSrc->r;
				lpDst->g = lpSrc->g;
				lpDst->r = lpSrc->b;
				lpSrc++;
				lpDst++;
			}
		}
	}
	else
	{
	}

	return lpAligImage;
}

void FBMPResource::SaveResource( void ** lpBuffer, UI32 & iImgSize )
{
	iImgSize = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) + DWORD_ALIG(iWidth) * iHeight;
	*lpBuffer = FMalloc( iImgSize );

	BITMAPFILEHEADER * lpFileBmp = (BITMAPFILEHEADER *)(*lpBuffer);
	BITMAPINFOHEADER * lpBmp = (BITMAPINFOHEADER *)(((UI8 *)(*lpBuffer)) + sizeof( BITMAPFILEHEADER ));

	lpBmp->biSize = sizeof( BITMAPINFOHEADER );
	lpBmp->biWidth = iWidth;
	lpBmp->biHeight = iHeight;
	lpBmp->biPlanes = 1;
	lpBmp->biBitCount = 24;
	lpBmp->biCompression = BI_RGB;
	lpBmp->biSizeImage = DWORD_ALIG(iWidth) * iHeight; 
	lpBmp->biClrImportant = 0;
	lpBmp->biClrUsed = 0;

	lpFileBmp->bfType = 0x4d42;
	lpFileBmp->bfSize = sizeof( BITMAPFILEHEADER ) + lpBmp->biSize + lpBmp->biSizeImage;
	lpFileBmp->bfReserved1 = 0;
	lpFileBmp->bfReserved2 = 0;
	lpFileBmp->bfOffBits = sizeof( BITMAPFILEHEADER ) + lpBmp->biSize;

	CHAR_ * lpAligData = MakeDwordAligImage( iWidth, iHeight, lpData, iFormat );
	memcpy( ((UI8 *)(*lpBuffer)) + sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ), lpAligData, lpBmp->biSizeImage );

	FFree( lpAligData );
}

FResource * FBMPResource::Make( void *lpData, UI32 iDataLen )
{
	return new FBMPResource( lpData, iDataLen );
}