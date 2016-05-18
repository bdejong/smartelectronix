#include "Multitap.h"

#if WIN32

#include <windows.h>
#include <assert.h>

Multitap::Multitap(unsigned long initialSize)
{
	sse = SSEDetect();

	amp = (__m128 *) _aligned_malloc(sizeof(__m128)*32,32);
	delay = (unsigned long *) _aligned_malloc(sizeof(unsigned long)*32,32);
	delayfpu = (unsigned long *) _aligned_malloc(sizeof(unsigned long)*32,32);

	buffer = 0;

	buffersize = 0;
	mask = 0;
	indexfpu = 0;
	maskfpu = 0;

	for(long i=0;i<32;i++)
	{
		delayfpu[i] = 0;
		delay[i] = 0;
		set4(amp[i],0.f);
	}

	setDelay(initialSize >= 1024 ? initialSize : 1024);
}

Multitap::~Multitap()
{
	if(buffer)
		delete buffer;

	_aligned_free(amp);
	_aligned_free(delay);
	_aligned_free(delayfpu);
}

//hosts that have power-of-2 blocksizes
//ann aligned memory will allways end up doing
//sse processing...
void Multitap::process(float *inputs, float *outputs, unsigned long nSamples, bool replace)
{
	//no use in using SSE for less samples then 16!
	//host calling VST plugins with less than 16-sample buffers should be shot, tortured and shot again
	if(nSamples <= 16 || !sse)
	{
		processFPU(inputs,outputs,nSamples,replace);
		return;
	}

	//let's see if the current index is a multiple of 4
	//if it isn't, we need to process untill it *IS*
	unsigned long startSize = (4 - (indexfpu & 3)) & 3;
	unsigned long blockSize = 0;

	//stupid, but who cares ;-)
	while(startSize + blockSize + 4 <= nSamples)
		blockSize += 4;

	//we'll have to process a maximum of 4 samples at the end...
	unsigned long endSize = nSamples - (startSize + blockSize);

	if(startSize)
		processFPU(inputs,outputs,startSize,replace);

	inputs += startSize;
	outputs += startSize;

	if(blockSize)
	{
		nSamples = blockSize;

		unsigned long index = indexfpu >> 2;

		_mm_empty();

		_mm_prefetch(((char *)&delay[0]),0);
		_mm_prefetch(((char *)&amp[0]),0);

		//are the buffers 16-byte aligned??
		if ((((int)inputs & 15) == 0) && (((int)outputs & 15) == 0))
		{
			nSamples >>= 2;
			while(nSamples--)
			{
				float *x = inputs + 4;
				float *y = outputs + 4;

				_mm_prefetch((char *) x,0);
				_mm_prefetch((char *) y,0);

				buffer[index] = _mm_load_ps(inputs);

				__m128 out_sse = _mm_setzero_ps();

				for(long z=0;z<32;z+=4)
				{
					long tmp1 = (index - delay[z+0]) & mask;
					long tmp2 = (index - delay[z+1]) & mask;
					long tmp3 = (index - delay[z+2]) & mask;
					long tmp4 = (index - delay[z+3]) & mask;

					//out += amp[z] * buffer[tmp1]
					out_sse = _mm_add_ps(_mm_mul_ps(amp[z],buffer[tmp1]),out_sse);
					_mm_prefetch(((char *)&buffer[tmp1]) + 16,0);

					out_sse = _mm_add_ps(_mm_mul_ps(amp[z+1],buffer[tmp2]),out_sse);
					_mm_prefetch(((char *)&buffer[tmp2]) + 16,0);

					out_sse = _mm_add_ps(_mm_mul_ps(amp[z+2],buffer[tmp3]),out_sse);
					_mm_prefetch(((char *)&buffer[tmp3]) + 16,0);

					out_sse = _mm_add_ps(_mm_mul_ps(amp[z+3],buffer[tmp4]),out_sse);
					_mm_prefetch(((char *)&buffer[tmp4]) + 16,0);
				}

				if(replace)
					_mm_store_ps(outputs,out_sse);
				else
					_mm_store_ps(outputs,_mm_add_ps(out_sse,_mm_load_ps(outputs)));

				index++;
				index &= mask;

				inputs = x;
				outputs = y;
			}
		}
		else //non-aligned buffers!
		{
			nSamples >>= 2;
			while(nSamples--)
			{
				float *x = inputs + 4;
				float *y = outputs + 4;

				_mm_prefetch((char *) x,0);
				_mm_prefetch((char *) y,0);

				buffer[index] = _mm_loadu_ps(inputs);

				__m128 out_sse = _mm_setzero_ps();

				for(long z=0;z<32;z+=4)
				{
					long tmp1 = (index - delay[z+0]) & mask;
					long tmp2 = (index - delay[z+1]) & mask;
					long tmp3 = (index - delay[z+2]) & mask;
					long tmp4 = (index - delay[z+3]) & mask;

					out_sse = _mm_add_ps(_mm_mul_ps(amp[z],buffer[tmp1]),out_sse);
					_mm_prefetch(((char *)&buffer[tmp1]) + 16,0);

					out_sse = _mm_add_ps(_mm_mul_ps(amp[z+1],buffer[tmp2]),out_sse);
					_mm_prefetch(((char *)&buffer[tmp2]) + 16,0);

					out_sse = _mm_add_ps(_mm_mul_ps(amp[z+2],buffer[tmp3]),out_sse);
					_mm_prefetch(((char *)&buffer[tmp3]) + 16,0);

					out_sse = _mm_add_ps(_mm_mul_ps(amp[z+3],buffer[tmp4]),out_sse);
					_mm_prefetch(((char *)&buffer[tmp4]) + 16,0);
				}

				if(replace)
					_mm_storeu_ps(outputs,out_sse);
				else
					_mm_storeu_ps(outputs,_mm_add_ps(out_sse,_mm_loadu_ps(outputs)));

				index++;
				index &= mask;

				inputs = x;
				outputs = y;
			}
		}

		_mm_empty();
	}

	indexfpu += blockSize;
	indexfpu &= maskfpu;

	//if there were some samples left we'll need to process them!!!
	if(endSize)
		processFPU(inputs,outputs,endSize,replace);
}

void Multitap::processFPU(float *inputs, float *outputs, unsigned long nSamples, bool replace)
{
	float *bufferfpu = (float *)&buffer[0];
	float *ampfpu = (float *)&amp[0];

	float out0,out1,out2,out3;

	for(unsigned long j=0;j<nSamples;j++)
	{
		bufferfpu[indexfpu] = inputs[j];

		out0 = out1 = out2 = out3 = 0.f;

		for(long i=0;i<32;i+=4)
		{
			out0  += bufferfpu[(indexfpu - delayfpu[i+0]) & maskfpu]*ampfpu[(i+0)*4];
			out1  += bufferfpu[(indexfpu - delayfpu[i+1]) & maskfpu]*ampfpu[(i+1)*4];
			out2  += bufferfpu[(indexfpu - delayfpu[i+2]) & maskfpu]*ampfpu[(i+2)*4];
			out3  += bufferfpu[(indexfpu - delayfpu[i+3]) & maskfpu]*ampfpu[(i+3)*4];
		}

		if(replace)
			outputs[j] = out0 + out1 + out2 + out3;
		else
			outputs[j] += out0 + out1 + out2 + out3;

		indexfpu++;
		indexfpu &= maskfpu;
	}
}

void Multitap::setParameters(const float _amp[32], const float _delay[32])
{
	float maxdelay = 0.f;
	long i;
	for(i=0;i<32;i++)
		maxdelay = _delay[i] > maxdelay ? _delay[i] : maxdelay;

	if((unsigned long) maxdelay > buffersize*4)
		setDelay((unsigned long)maxdelay);

	for(i=0;i<32;i++)
	{
		delayfpu[i] = ((unsigned long)_delay[i]) & 0xfffffffc;
		delay[i] = delayfpu[i] >> 2;
		set4(amp[i],_amp[i]);
	}
};

void Multitap::setDelay(unsigned long size)
{
	if(size <= buffersize*4)
		return;

	unsigned long n = 1;
	while(n <= size)
		n <<= 1;

	maskfpu = n - 1;
	buffersize = n >> 2;
	mask = buffersize - 1;
	indexfpu = 0;

	if(buffer)
	{
		_aligned_free(buffer);
		buffer = 0;
	}

	buffer = (__m128 *) _aligned_malloc(sizeof(__m128)*buffersize,32);

	resume();
};

void Multitap::resume()
{
	for(unsigned long j=0;j<buffersize;j++)
		set4(buffer[j],0.f);

	indexfpu = 0;
};

bool Multitap::SSEDetect()
{
	bool SSE = true;
	__try
	{
		__asm
		{
			orps xmm1, xmm2
		}

		_mm_empty(); //you knever know
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		SSE = false;
	}

	return SSE;
}

void Multitap::set4(__m128 &x, float y)
{
	float *px = (float *)(&x);
	px[0] = y;
	px[1] = y;
	px[2] = y;
	px[3] = y;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
#else //MAC!!!!!!!!!!!!!!!!!!!!!!!!!!!
////////////////////////////////////////////////////////////////////////////////////////////////////////////

Multitap::Multitap(unsigned long initialSize)
{
	buffer = 0;

	buffersize = 0;
	mask = 0;
	index = 0;

	for(long i=0;i<32;i++)
	{
		delay[i] = 0;
		amp[i] = 0.f;
	}

	setDelay(initialSize >= 1024 ? initialSize : 1024);
}

Multitap::~Multitap()
{
	if(buffer)
		delete buffer;
}

void Multitap::process(float *inputs, float *outputs, unsigned long nSamples, bool replace)
{
	float out0,out1,out2,out3;

	for(unsigned long j=0;j<nSamples;j++)
	{
		buffer[index] = inputs[j];

		out0 = out1 = out2 = out3 = 0.f;

		for(long i=0;i<32;i+=4)
		{
			out0  += buffer[(index - delay[i+0]) & mask]*amp[i+0];
			out1  += buffer[(index - delay[i+1]) & mask]*amp[i+1];
			out2  += buffer[(index - delay[i+2]) & mask]*amp[i+2];
			out3  += buffer[(index - delay[i+3]) & mask]*amp[i+3];
		}

		if(replace)
			outputs[j] = out0 + out1 + out2 + out3;
		else
			outputs[j] += out0 + out1 + out2 + out3;

		index++;
		index &= mask;
	}
}

void Multitap::setParameters(const float _amp[32], const float _delay[32])
{
	float maxdelay = 0.f;
	long i;
	for(i=0;i<32;i++)
		maxdelay = _delay[i] > maxdelay ? _delay[i] : maxdelay;

	if((unsigned long) maxdelay > buffersize)
		setDelay((unsigned long)maxdelay);

	for(i=0;i<32;i++)
	{
		delay[i] = (unsigned long)_delay[i];
		amp[i] = _amp[i];
	}
}

void Multitap::setDelay(unsigned long size)
{
	if(size <= buffersize)
		return;

	unsigned long n = 1;
	while(n <= size)
		n <<= 1;

	buffersize = n;
	mask = buffersize - 1;
	index = 0;

	if(buffer)
	{
		delete buffer;
		buffer = 0;
	}

	buffer = new float [buffersize];

	resume();
}

void Multitap::resume()
{
	for(unsigned long j=0;j<buffersize;j++)
		buffer[j] = 0.f;

	index = 0;
}

#endif
