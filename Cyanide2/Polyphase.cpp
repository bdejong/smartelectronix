#include "Polyphase.h"

#include <math.h>

#define IS_DENORMAL(x) (fabs(x) < 0.000001f)

CPolyphase::CPolyphase()
{
	suspend();
}

CPolyphase::~CPolyphase()
{
}

void CPolyphase::suspend()
{
	lout[0] = lout[1] = lout[2] = 0.f;
	uout[0] = uout[1] = uout[2] = 0.f;

	uin_1[0] = uin_1[1] = uin_1[2] = 0.f;
	lin_1[0] = lin_1[1] = lin_1[2] = 0.f;
}

//remember: output has to be twice the size of input!!!
void CPolyphase::Upsample(float *input, float *output, long nSamples)
{
	long index = 0;
	for(long i=0;i<nSamples;i++)
	{
		lout[0] = (input[i] - lout[0])*0.039151597734460045f + lin_1[0];
		uout[0] = (input[i] - uout[0])*0.147377113601046600f + uin_1[0];

		uin_1[0] = lin_1[0] = input[i];

		uout[1] = (uout[0] - uout[1])*0.48246854276970014f + uin_1[1];
		lout[1] = (lout[0] - lout[1])*0.30264684832849340f + lin_1[1];
		
		lin_1[1] = lout[0];
		uin_1[1] = uout[0];

		output[index]   = lout[2] = (lout[1] - lout[2])*0.6746159185469639f + lin_1[2];
		output[index+1] = uout[2] = (uout[1] - uout[2])*0.8830050257693731f + uin_1[2];
		
		uin_1[2] = uout[1];
		lin_1[2] = lout[1];

		index += 2;
	}
}

//remember: input has to be twice the size of output!!!
void CPolyphase::Downsample(float *input, float *output, long nSamples)
{
	long index = 0;
	for(long i=0;i<nSamples;i++,index+=2)
	{
		uout[0] = (input[index] - uout[0])*0.1473771136010466f + uin_1[0];
		lout[0] = (input[index+1] - lout[0])*0.039151597734460045f + lin_1[0];
		
		lin_1[0] = input[index+1];
		uin_1[0] = input[index];

		uout[1] = (uout[0] - uout[1])*0.48246854276970014f + uin_1[1];
		lout[1] = (lout[0] - lout[1])*0.3026468483284934f + lin_1[1];
		
		lin_1[1] = lout[0];
		uin_1[1] = uout[0];

		uout[2] = (uout[1] - uout[2])*0.8830050257693731f + uin_1[2];
		lout[2] = (lout[1] - lout[2])*0.6746159185469639f + lin_1[2];
		
		lin_1[2] = lout[1];
		uin_1[2] = uout[1];

		output[i] = (lout[2] + uout[2])*0.5f;
	}
}

void CPolyphase::denormalise()
{
	if(IS_DENORMAL(lout[0]))
		lout[0] = 0.f;
	if(IS_DENORMAL(lout[1]))
		lout[1] = 0.f;
	if(IS_DENORMAL(lout[2]))
		lout[2] = 0.f;

	if(IS_DENORMAL(uout[0]))
		uout[0] = 0.f;
	if(IS_DENORMAL(uout[1]))
		uout[1] = 0.f;
	if(IS_DENORMAL(uout[2]))
		uout[2] = 0.f;
}
