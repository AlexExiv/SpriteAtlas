#ifndef __FIMAGE_RESOURCE_H__
#define __FIMAGE_RESOURCE_H__

#include "FResource.h"

class FTexture;

class FImageResource : public FResource
{
public:
	enum
	{
		IMAGE_UNKNOWN = 0,
		IMAGE_RGB,
		IMAGE_RGBX,
		IMAGE_RGBA,
		IMAGE_A,
		IMAGE_L,
		IMAGE_DXT1,
		IMAGE_DXT2,
		IMAGE_DXT3,
		IMAGE_DXT4,
		IMAGE_DXT5
	};

protected:
	UI32 iWidth, iHeight;
	UI32 iFormat, iBpp;

	FImageResource( void * lpData, const FString & sExtStr );
	FImageResource( const FString & sExtStr );
	FImageResource( UI32 iWidth, UI32 iHeight, void * lpData, UI32 iFormat, const FString & sExtStr );
	virtual void Decode( void * lpData, UI32 iDataLen ){}
	virtual FResource * Make( void * lpData, UI32 iDataLen );
	virtual void SaveResource( void ** lpBuffer, UI32 & iDataSize ){};

public:
	~FImageResource();

	UI32 GetWidth()const;
	UI32 GetHeight()const;
	UI32 GetFormat()const;
	UI32 GetBpp()const;
	virtual void * GetDataLevel( I32 iLevel )const;
	virtual UI32 GetLevelSize( I32 iLevel )const;
	bool IsCompressed()const;

	virtual void AddAlphaData( FImageResource * lpImage );
	virtual void AddAlphaData( void * lpAlphaData, I32 iFormat );
};

#endif