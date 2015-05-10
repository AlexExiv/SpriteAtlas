#ifndef __FATLAS_BUILDER_H__
#define __FATLAS_BUILDER_H__


#include "FString.h"
#include "FList.h"
#include "FAtlasHeader.h"



class FAtlasConfig;
class FFrame;

class FAtlasBuilder
{
public:
	typedef FList<FFrame *> FFrameList;
	typedef FList<FFrame *>::Iterator FFrameIterator;

private:
	FString sFileName;
	FAtlasConfig * lpAtlasCfg;
	FFrameList lFrameList;
	RGBA * lpAtlasData;
	UI32 iHeight;

	void ReallocData( UI32 iWidth, UI32 iHeight );
	void Clear();

public:
	FAtlasBuilder();
	~FAtlasBuilder();

	UI32 BuildAtlas( const FString & sFileName );
	UI32 EditAlphaStart();
	UI32 EditAlphaDone();
	UI32 SaveAtlas();

	bool IsPacked()const;
	void GetHelpInfo( CHAR_ * lpBuffer );
};

#endif