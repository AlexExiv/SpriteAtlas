#ifndef __FTGARESOURCE_H__
#define __FTGARESOURCE_H__

#include "FImageResource.h"

class FTGAResource : public FImageResource
{
	friend class FResourceManager;

	void Decode( void * lpData, UI32 iDataLen );
	virtual FResource * Make( void * lpData, UI32 iDataLen );

	FTGAResource();
public:
	FTGAResource( void * lpData, UI32 iDataLen );
	~FTGAResource();
};

#endif