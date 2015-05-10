#include "FPNGResource.h"
#include "FString.h"
#include <malloc.h>
#include <string.h>
#include "FStack.h"


extern "C"
{
	#define		Byte	z_Byte					// to avoid conflcits in zconf.h
	#include	"zlib\zlib.h"
	#undef		Byte
	#include	"png\png.h"
}

struct PngRead
{
	UI8 * lpData;
	size_t iSize;
};

struct PngWrite
{
	UI8 * lpData;
	size_t iSize;
};

static void PngReadFunc( png_structp lpPng, png_bytep lpData, png_size_t iLength )
{
	PngRead * lpPngRead = (PngRead *)png_get_io_ptr( lpPng );
	if( lpPngRead->iSize < iLength )
		png_error( lpPng, "Read error" );
	else
	{
		memcpy( lpData, lpPngRead->lpData, iLength );
		lpPngRead->lpData += iLength;
		lpPngRead->iSize -= iLength;
	}
}


static void PngWriteFunc( png_structp lpPng, png_bytep lpData, png_size_t iLength )
{
	PngWrite * lpPngWrite = (PngWrite *)png_get_io_ptr( lpPng );
	if( lpPngWrite->iSize < iLength )
		png_error( lpPng, "Write error" );
	else
	{
		memcpy( lpPngWrite->lpData, lpData, iLength );
		lpPngWrite->lpData += iLength;
		lpPngWrite->iSize -= iLength;
	}
}

static void PngFlushFunc( png_structp lpPng )
{

}

FPNGResource::FPNGResource() : FImageResource( FString( "png" ) )
{
}

FPNGResource::FPNGResource( void * lpData, UI32 iDataLen ) : FImageResource( lpData, FString( "png" ) )
{
	Decode( lpData, iDataLen );
}

FPNGResource::FPNGResource( UI32 iWidth, UI32 iHeight, void * lpData, UI32 iFormat ) : FImageResource( iWidth, iHeight, lpData, iFormat, FString( "png" ) )
{
}

FPNGResource::~FPNGResource()
{
}

void FPNGResource::Decode( void * lpData0, UI32 iDataLen )
{
	png_byte * lpSign = (png_byte *)lpData0;
	if( !png_check_sig( lpSign, 8 ) )
		return;

	png_structp lpPng = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	if( !lpPng )
		return;
	png_infop lpInfo = png_create_info_struct( lpPng );
	if( !lpInfo )
	{
		png_destroy_read_struct( &lpPng, NULL, NULL );
		return;
	}

	png_set_sig_bytes( lpPng, 8 );
	png_read_info( lpPng, lpInfo );

	iWidth = png_get_image_width( lpPng, lpInfo );
	iHeight = png_get_image_height( lpPng, lpInfo );
	iBpp = png_get_bit_depth( lpPng, lpInfo );
	I32 iImFmt = png_get_color_type( lpPng, lpInfo );

	if( iBpp == 16 )
		png_set_strip_16( lpPng );

	switch ( iImFmt )
	{
	case PNG_COLOR_TYPE_PALETTE:
		if( iBpp < 8 )
			png_set_palette_to_rgb( lpPng );
	case PNG_COLOR_TYPE_RGB:
		iBpp = 3;
		iFormat = IMAGE_RGB;
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		iBpp = 4;
		iFormat = IMAGE_RGBA;
		break;
	case PNG_COLOR_TYPE_GRAY:
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		if( iBpp < 8 )
			png_set_expand_gray_1_2_4_to_8( lpPng );
		iBpp = 1;
		iFormat = IMAGE_A;
		break;
	default:
		break;
	}

	F64 fGamma = 0.f;
	if( png_get_gAMA( lpPng, lpInfo, &fGamma ) )
		png_set_gamma( lpPng, 2.2, fGamma );
	else
		png_set_gamma( lpPng, 2.2, 0.45455 );

	png_read_update_info( lpPng, lpInfo );
	I32 iRowBytes = png_get_rowbytes( lpPng, lpInfo );
	iBpp = png_get_bit_depth( lpPng, lpInfo );
	PngRead sPngRead = { (UI8 *)lpData0, iDataLen };
	png_set_read_fn( lpPng, &sPngRead, PngReadFunc );

	lpData = FMalloc( iWidth*iHeight*iBpp );
	png_bytep * lpRow = (png_bytep *)FStack::GetInstanceStack()->PushBlock( iHeight*sizeof( png_bytep * ) );
	for( I32 i = 0;i < iHeight;i++ )
		lpRow[iHeight - i - 1] = ((UI8 *)lpData) + i*iWidth*iBpp;
	png_read_image( lpPng, lpRow );

	FStack::GetInstanceStack()->PopBlock();
	png_destroy_read_struct( &lpPng, &lpInfo, NULL );
}

FResource * FPNGResource::Make( void * lpData, UI32 iDataLen )
{
	return new FPNGResource( lpData, iDataLen );
}

void FPNGResource::SaveResource( void ** lpData0, UI32 & iImgSize )
{
	png_structp lpPng = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	if( !lpPng )
		return;

	png_infop lpInfo = png_create_info_struct( lpPng );
	if( !lpInfo )
	{
		png_destroy_write_struct( &lpPng, NULL );
		return;
	}

	if( setjmp( png_jmpbuf( lpPng ) ) )
	{
		png_destroy_write_struct( &lpPng, &lpInfo );
		return;
	}

	UI32 iSize = iWidth*iHeight*sizeof( RGBA );
	UI8 * lpCompData = (UI8 *)FMalloc( iSize );
	PngWrite pPngWr = { lpCompData, iSize };
	png_set_write_fn( lpPng, (void *)&pPngWr, PngWriteFunc, PngFlushFunc );

	I32 iColorType = 0, iBps = 0;
	switch(iFormat)
	{
	case IMAGE_RGB:
		iColorType = PNG_COLOR_TYPE_RGB;
		iBps = sizeof( RGB );
		break;
	case IMAGE_RGBA:
		iColorType = PNG_COLOR_TYPE_RGB_ALPHA;
		iBps = sizeof( RGBA );
		break;
	case IMAGE_A:
	case IMAGE_L:
		iColorType = PNG_COLOR_TYPE_GRAY;
		iBps = sizeof( UI8 );
		break;
	default:
		break;
	}

	png_set_IHDR( lpPng, lpInfo, iWidth, iHeight, 8, iColorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );
	png_write_info( lpPng, lpInfo );
	png_bytep * lpRows = (png_bytep *)FStack::GetInstanceStack()->PushBlock( iHeight*sizeof( png_bytep ) );
	for( UI32 i = 0;i < iHeight;i++ )
		lpRows[i] = ((UI8 *)lpData) + i*iWidth*iBps;
	png_write_image( lpPng, lpRows );
	png_write_end( lpPng, lpInfo );
	png_destroy_write_struct( &lpPng, &lpInfo );

	*lpData0 = lpCompData;
	iImgSize = iSize - pPngWr.iSize;
	FStack::GetInstanceStack()->PopBlock();
}
