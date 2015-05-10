#include "FJPEGResource.h"
#include "FString.h"
#include "FStack.h"
#include <setjmp.h>
#include	<stdio.h>

extern "C"
{
	#include "jpeg\jpeglib.h"
	#include "jpeg\jerror.h"
}


struct MyErrorMgr
{
	jpeg_error_mgr jPub;
	jmp_buf jJmpBuf;
};

static void InitSource( j_decompress_ptr lpDecom )
{
}

static boolean FillInputBuffer( j_decompress_ptr lpDecom )
{
	return true;
}

static void SkipInputData( j_decompress_ptr lpDecom, long iCount )
{
	jpeg_source_mgr * lpSrc = lpDecom->src;
	if( iCount > 0 )
	{
		lpSrc->bytes_in_buffer -= iCount;
		lpSrc->next_input_byte += iCount;
	}
}

static void TermSource( j_decompress_ptr )
{
}

static void MyErrorExit( j_common_ptr lpCom )
{
	MyErrorMgr * lpMyErr = (MyErrorMgr *)lpCom->err;
	longjmp( lpMyErr->jJmpBuf, 1 );
}

FJPEGResource::FJPEGResource() : FImageResource( FString( "jpg" ) )
{
}

FJPEGResource::FJPEGResource( void * lpData, UI32 iDataLen ) : FImageResource( lpData, FString( "jpg" ) ) 
{
	Decode( lpData, iDataLen );
}

FJPEGResource::FJPEGResource( UI32 iWidth, UI32 iHeight, void * lpData, UI32 iFormat ) : FImageResource( iWidth, iHeight, lpData, iFormat, FString( "jpg" ) ), iQuality( 50 )
{
}


FJPEGResource::~FJPEGResource()
{
}

void FJPEGResource::SaveResource( void ** lpData, UI32 & iImgSize )
{
	jpeg_compress_struct jInfo;
	jpeg_error_mgr jErr;
	jInfo.err = jpeg_std_error( &jErr );
	jpeg_create_compress( &jInfo );
	UI8 * lpBuffer = NULL;
	ULONG iDataSize = 0;
	jpeg_mem_dest( &jInfo, &lpBuffer, &iDataSize );

	jInfo.image_width = iWidth;
	jInfo.image_height = iHeight;
	switch( iFormat )
	{
	case IMAGE_A:
		jInfo.input_components = 1;
		jInfo.in_color_space = JCS_GRAYSCALE;
		break;
	case IMAGE_RGB:
		jInfo.input_components = 3;
		jInfo.in_color_space = JCS_RGB;
		break;
	}

	jpeg_set_defaults( &jInfo );
	jpeg_set_quality( &jInfo, iQuality, TRUE );

	jpeg_start_compress( &jInfo, TRUE );
	UI32 iRowStride = jInfo.input_components*iWidth;
	JSAMPROW lpSample;

	while( jInfo.next_scanline < jInfo.image_height )
	{
		lpSample = ((UI8 *)this->lpData) + jInfo.next_scanline*iRowStride;
		jpeg_write_scanlines( &jInfo, &lpSample, 1 );
	}

	jpeg_finish_compress( &jInfo );
	jpeg_destroy_compress( &jInfo );

	*lpData = (void *)lpBuffer;
	iImgSize = iDataSize;
}

void FJPEGResource::Decode( void * lpData0, UI32 iDataLen )
{
	jpeg_decompress_struct jInfo;
	MyErrorMgr jError;

	jInfo.err = jpeg_std_error( &jError.jPub );
	jError.jPub.error_exit = MyErrorExit;

	if( setjmp( jError.jJmpBuf ) )
	{
		jpeg_destroy_decompress( &jInfo );
		return;
	}

	jpeg_create_decompress( &jInfo );

	jpeg_source_mgr jSrc;
	jSrc.bytes_in_buffer = iDataLen;
	jSrc.next_input_byte = (JOCTET *)lpData0;
	jSrc.init_source = InitSource;
	jSrc.fill_input_buffer = FillInputBuffer;
	jSrc.skip_input_data = SkipInputData;
	jSrc.resync_to_restart = jpeg_resync_to_restart;
	jSrc.term_source = TermSource;

	jInfo.src = &jSrc;

	jpeg_read_header( &jInfo, true );
	if( jInfo.jpeg_color_space != JCS_GRAYSCALE )
	{
		jInfo.out_color_space = JCS_RGB;
		iFormat = IMAGE_RGB;
	}
	else
		iFormat = IMAGE_A;


	jpeg_calc_output_dimensions( &jInfo );
	jpeg_start_decompress( &jInfo );

	iWidth = jInfo.output_width;
	iHeight = jInfo.output_height;

	RGB * lpPalette = (RGB *)FStack::GetInstanceStack()->PushBlock( sizeof( RGB )*256 );
	if( jInfo.quantize_colors )
	{
		I32 iShift = 8 - jInfo.data_precision;
		if( jInfo.jpeg_color_space != JCS_GRAYSCALE )
		{
			for ( I32 i = 0; i < jInfo.actual_number_of_colors; i++ )
			{
				lpPalette[i].r = jInfo.colormap [0][i];
				lpPalette[i].g = jInfo.colormap [1][i];
				lpPalette[i].b = jInfo.colormap [2][i];
			}
		}
		else
		{
			for ( I32 i = 0; i < jInfo.actual_number_of_colors; i++ )
			{
				lpPalette[i].r = jInfo.colormap [0][i];
				lpPalette[i].g = jInfo.colormap [0][i];
				lpPalette[i].b = jInfo.colormap [0][i];
			}
		}
	}

	JSAMPARRAY lpBuffer = (*jInfo.mem->alloc_sarray)( (j_common_ptr)&jInfo, JPOOL_IMAGE, jInfo.image_width*jInfo.num_components, 1 );
	if( iFormat == IMAGE_A )
		iBpp = 1;
	else
		iBpp = sizeof( RGB );
	lpData = FMalloc( iWidth*iHeight*iBpp );

	while( jInfo.output_scanline < jInfo.output_height )
	{
		jpeg_read_scanlines( &jInfo, lpBuffer, 1 );
		I32 y = jInfo.output_scanline - 1;
		if( jInfo.output_components == 1 )
		{
			if( jInfo.quantize_colors )
			{
				RGB * lpDst = ((RGB *)lpData) + y*iWidth;
				for( I32 i = 0;i < iWidth;i++, lpDst++ )
				{
					lpDst->r = lpPalette[lpBuffer[0][i]].r;
					lpDst->g = lpPalette[lpBuffer[0][i]].g;
					lpDst->b = lpPalette[lpBuffer[0][i]].b;
				}
			}
			else
			{
				UI8 * lpDst = ((UI8 *)lpData) + y*iWidth;
				for( I32 i = 0;i < iWidth;i++, lpDst++ )
					*lpDst = lpPalette[lpBuffer[0][i]].r;
			}
		}
		else
		{
			UI8 * lpSrc = lpBuffer[0];
			RGB * lpDst = ((RGB *)lpData) + y*iWidth;
			for( I32 i = 0;i < iWidth;i++, lpSrc += 3, lpDst++ )
			{
				lpDst->r = lpSrc[0];
				lpDst->g = lpSrc[1];
				lpDst->b = lpSrc[2];
			}
		}
	}

	jpeg_finish_decompress( &jInfo );
	jpeg_destroy_decompress( &jInfo );

	FStack::GetInstanceStack()->PopBlock();
}

FResource * FJPEGResource::Make( void * lpData, UI32 iDataLen )
{
	return new FJPEGResource( lpData, iDataLen );
}

void FJPEGResource::SetQuality( I32 iQuality0 )
{
	iQuality = iQuality0;
}