// WavetableOsc.cpp: implementation of the CWavetableOsc class.
//
//////////////////////////////////////////////////////////////////////

#include "WavetableFPOsc.h"
#include "math.h"

#ifdef _DEBUG
	#include "stdio.h"
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWavetableFPOsc::CWavetableFPOsc()
{
	fractable = buffer = dbuffer = 0;
	
	buffer = new float [4096];
	dbuffer = new float [4096];
	fractable = new float [4096];

	fmagic = (4096.f/44100.f)*0x1000;

	phase = 0;

	CreateFracTable();
}

CWavetableFPOsc::~CWavetableFPOsc()
{
	if(fractable)
		delete fractable;
	if(buffer)
		delete buffer;
	if(dbuffer)
		delete dbuffer;
}

void CWavetableFPOsc::reset()
{
	this->phase = 0;
}

void CWavetableFPOsc::setrate(float samplerate)
{
	fmagic = (4096.f/samplerate)*0x1000;
}

void CWavetableFPOsc::CreateFracTable()
{
	for(int i=0;i<4096;i++)
	{
		fractable[i] = (float)((double)i/4096.0);
		buffer[i] = (float) ((1.0 + sin((double)i/4096.0*2.0*3.1415926535))*0.5);
	}

	for(i=0;i<4096-1;i++)
		dbuffer[i] = buffer[i+1] - buffer[i];

	dbuffer[4095] = buffer[0] - buffer[4095];
}

void CWavetableFPOsc::setfreq(float freq)
{
	tmp = (unsigned int)(freq*fmagic);
}

void CWavetableFPOsc::setstereo(unsigned int otherphase, float stereo)
{
	phase = otherphase + (unsigned int)(4096.f*0x1000*stereo*0.5f);
}
