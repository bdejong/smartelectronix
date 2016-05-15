#pragma once

#if WIN32

#include "xmmintrin.h"

class Multitap
{
public:
	
	void process(float *inputs, float *outputs, unsigned long nSamples, bool replace);
	void resume();

	static bool SSEDetect();

	void forceSSE(bool toForce)
	{
		sse = toForce;
	}

protected:
	
	Multitap(unsigned long initialSize);
	~Multitap();
	
	void setDelay(unsigned long size);
	void setParameters(const float _amp[32], const float _delay[32]);	

private:
	
	void processFPU(float *inputs, float *outputs, unsigned long nSamples, bool replace);
	void set4(__m128 &x, float y);
	
	__m128 *buffer;
	__m128 *amp;
	
	unsigned long buffersize;
	unsigned long mask;
	unsigned long *delay;
	unsigned long indexfpu;
	unsigned long maskfpu;
	unsigned long *delayfpu;

	bool sse;
};

#else

class Multitap
{
public:
	
	void process(float *inputs, float *outputs, unsigned long nSamples, bool replace);
	void resume();

protected:
	
	Multitap(unsigned long initialSize);
	~Multitap();
	
	void setDelay(unsigned long size);
	void setParameters(const float _amp[32], const float _delay[32]);	

private:
	
	float *buffer;
	
	float amp[32];
	unsigned long delay[32];

	unsigned long buffersize;
	unsigned long index;
	unsigned long mask;
};

#endif
