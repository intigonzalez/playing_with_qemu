#ifndef VIDEO_H_
#define VIDEO_H_

#include "includes/global.h"
#include "kernel.h"


void InitVideoDriver(unsigned char * video);

extern "C"
{
	void printf(const char* format,...);

}

#endif /*VIDEO_H_*/
