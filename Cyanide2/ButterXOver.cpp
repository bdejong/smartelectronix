#include "ButterXOver.h"
#include "Defines.h"

#include <math.h>

XOverCoeffs CButterXOver::Pre[1024];
float CButterXOver::PreFreq[1024];

CButterXOver::CButterXOver()
{
	in_2=outA1=in_1=out_2_A2=out_1_A2=0.f;

	freq = (nPre-1) << 2;

	samplerate = 0.f;
	setrate(44100.0);
	
	settype(0.f);
}

CButterXOver::~CButterXOver()
{
}

void CButterXOver::settype(float t)
{
	const float amt = 4.f;
	const float tmp = 0.5f*amt;
	t = 0.5f*tanhf(amt*t - tmp)/tanhf(tmp) + 0.5f; //some empirical ch00ning
	float a = (float)cos(t*3.141592*0.5)*0.7f;
	float b = (float)sin(t*3.141592*0.5)*0.7f;
	m2 = (a - b)*0.5f;
	m1 = (a + b)*0.5f;
}

void CButterXOver::setrate(float sr)
{
	if(fabs(this->samplerate - sr)>1.0)
	{
		samplerate = sr;
		precompute();
	}
}

void CButterXOver::suspend()
{
	in_2=outA1=in_1=out_2_A2=out_1_A2=0.f;
}

void CButterXOver::denormalise()
{
	if(IS_DENORMAL(outA1))
		outA1=0.f;
	if(IS_DENORMAL(out_2_A2))
		out_2_A2=0.f;
	if(IS_DENORMAL(out_1_A2))
		out_1_A2=0.f;
}

void CButterXOver::precompute()
{
	double freq = 100.0;
	double mfreq;
	
	if(samplerate <= 44101.f)
		mfreq = pow(((samplerate*0.5f - 10.f)/100.0),(1.00/(double)nPre));
	else
		mfreq = pow(((44100.f*0.5f - 10.f)/100.0),(1.00/(double)nPre));
	
	long double Wd,tmp,tmp2;
	long i;
	for(i=0;i<nPre-1;i++)
	{
		Wd = (2.0*3.141592653*freq)/samplerate;
		
		tmp = tan(Wd*0.5f);
		tmp2 = 1.0/(1.0 + tmp + tmp*tmp);

		Pre[i].aA1 = (float)((-1.0 + tmp)/(1.0 + tmp));
		Pre[i].aA2 = (float)((1 - tmp + tmp*tmp)*tmp2);
		Pre[i].bA2 = (float)((2.0*tmp*tmp - 2.0)*tmp2);

		PreFreq[i] = (float)freq;

		freq *= mfreq;
	}

	Wd = (2.0*3.141592653*(samplerate*0.5f-10.f))/samplerate;
		
	tmp = tan(Wd*0.5f);
	tmp2 = 1.0 + tmp + tmp*tmp;

	Pre[i].aA1 = (float)((-1.0 + tmp)/(1.0 + tmp));
	Pre[i].aA2 = (float)((1 - tmp + tmp*tmp)/tmp2);
	Pre[i].bA2 = (float)((2.0*tmp*tmp - 2.0)/tmp2);

	PreFreq[i] = (float)freq;
}

void CButterXOver::process(float *input, float *output, long nSamp)
{
	float outA2;

	long realfreq = freq >> 2;
	
	for(long i=0;i<nSamp;i++)
	{
		outA2 = Pre[realfreq].aA2*(input[i] - out_2_A2);
		
		outA1 = Pre[realfreq].aA1*(input[i] - outA1) + in_1;
		
		outA2 += Pre[realfreq].bA2*(in_1 - out_1_A2) + in_2;


		out_2_A2 = out_1_A2;
		out_1_A2 = outA2;
		in_2 = in_1;
		in_1 = input[i];

		output[i] = m2*outA2 + m1*outA1; //mix = [-1..1]
	}
}

void CButterXOver::processchange(float *input, float *output, long filter, long nSamp)
{
	long realfreq = freq >> 2;
	
	if(realfreq == filter)
	{
		process(input,output,nSamp);
		return;
	}
	
	float outA2;
	long add = filter > realfreq ? 1 : -1;
	
	for(long i=0;i<nSamp;i++)
	{
		outA2 = Pre[realfreq].aA2*(input[i] - out_2_A2);
		outA1 = Pre[realfreq].aA1*(input[i] - outA1) + in_1;
		outA2 += Pre[realfreq].bA2*(in_1 - out_1_A2) + in_2;

		out_2_A2 = out_1_A2;
		out_1_A2 = outA2;
		in_2 = in_1;
		in_1 = input[i];

		output[i] = m2*outA2 + m1*outA1; //mix = [-1..1]

		if(realfreq != filter)
		{
			freq += add;
			realfreq = freq >> 2;
		}
	}
}

float CButterXOver::getFreq(long index)
{
	return PreFreq[index];
}
