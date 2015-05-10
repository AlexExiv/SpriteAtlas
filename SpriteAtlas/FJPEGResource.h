#ifndef __FJPEG_RESOURCE_H__
#define __FJPEG_RESOURCE_H__

#include "FImageResource.h"

class FJPEGResource : public FImageResource
{
	friend class FResourceManager;

	void Decode( void * lpData, UI32 iDataLen );
	virtual FResource * Make( void * lpData, UI32 iDataLen );
	void SaveResource( void ** lpData, UI32 & iImgSize );
	FJPEGResource();

	I32 iQuality;
public:
	FJPEGResource( void * lpData, UI32 iDataLen );
	FJPEGResource( UI32 iWidth, UI32 iHeight, void * lpData, UI32 iFormat );
	~FJPEGResource();

	void SetQuality( I32 iQuality );
};

#endif