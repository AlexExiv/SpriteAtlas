#ifndef __FFRAME_H__
#define __FFRAME_H__


#include "FString.h"

class FAtlasConfig;

class FFrame
{
	UI32 iWidth, iHeight;
	UI32 x, y;
	F32 u0, v0, u1, v1;
	RGBA * lpData;
	CHAR_ * lpFrameName;
	UI32 iIndex;
	bool bWrited;
	FAtlasConfig * lpAtlasCfg;

	void CalcAlpha( RGBA * lpData, UI32 iWidth, UI32 iHeight );
	void Scale( RGBA * lpSrc, UI32 iSrcW, UI32 iSrcH, RGBA * lpDst, UI32 iDstW, UI32 iDstH );
	I32 SumR( I32 v, I32 u, I32 iStepX, I32 iStepY, I32 iSrcW, I32 iSrcH, RGBA * lpSrc );
	I32 SumG( I32 v, I32 u, I32 iStepX, I32 iStepY, I32 iSrcW, I32 iSrcH, RGBA * lpSrc );
	I32 SumB( I32 v, I32 u, I32 iStepX, I32 iStepY, I32 iSrcW, I32 iSrcH, RGBA * lpSrc );
	I32 SumA( I32 v, I32 u, I32 iStepX, I32 iStepY, I32 iSrcW, I32 iSrcH, RGBA * lpSrc );

public:
	FFrame( const FString & sName, UI32 x, UI32 y, UI32 iSrcWidth, UI32 iSrcHeight, UI32 iDstWidth, UI32 iDstHeight, UI32 iIndex, FAtlasConfig * lpAtlasCfg );
	virtual ~FFrame();

	void WriteToDataRGB( RGB * lpDstData, UI32 x, UI32 y, UI32 iWidth, UI32 iHeight );
	void WriteToDataRGBA( RGBA * lpDstData, UI32 x, UI32 y, UI32 iWidth, UI32 iHeight );
	void WriteToDataAlpha( PIXCOMP * lpDstData, UI32 x, UI32 y, UI32 iWidth, UI32 iHeight );

	UI32 GetWidth()const;
	UI32 GetHeight()const;
	const CHAR_ * GetFrameName()const;
	UI32 GetIndex()const;
	void GetParams( UI32 & x, UI32 & y, F32 & u0, F32 & v0, F32 & u1, F32 & v1 );
	bool IsWrited()const;

	void SetWrited( bool bWrited );
};

#endif