#ifndef __FSTACK_H__
#define __FSTACK_H__

#include "types.h"

#define MAX_STACK_ELEMENTS 1024
#define MAX_STACK_SIZE 26048576

class FStack
{
	UI8 * lpData;
	UI32 iElementSizes[MAX_STACK_ELEMENTS];
	UI32 iStackSize, iAllocatedSize, iLastElement;

public:
	FStack( UI32 iStackSize );
	~FStack();

	static void Init();
	static void Deinit();
	static FStack * GetInstanceStack();

	void * PushBlock( UI32 iSize );
	void PopBlock();
};

#endif