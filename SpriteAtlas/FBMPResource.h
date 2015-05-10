#ifndef __FBMP_RESOURCE_H__
#define __FBMP_RESOURCE_H__

#include "FImageResource.h"

class FBMPResource : public FImageResource
{
	friend class FResourceManager;

	void Decode( void * lpData, UI32 iDataLen );
	virtual FResource * Make( void * lpData, UI32 iDataLen );
	void SaveResource( void ** lpData, UI32 & iImgSize );
	FBMPResource();

public:
	FBMPResource( void * lpData, UI32 iDataLen );
	FBMPResource( UI32 iWidth, UI32 iHeight, void * lpData, UI32 iFormat );
	~FBMPResource();

};

#endif