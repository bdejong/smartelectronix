// Bouncy.h: interface for the Bouncy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BOUNCY_H__844BCE47_92B7_4935_A8F1_A00C030941F4__INCLUDED_)
#define AFX_BOUNCY_H__844BCE47_92B7_4935_A8F1_A00C030941F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_DELAY 20.f

#include "Multitap.h"
#include "math.h"
#include "stdlib.h"

class Bouncy : public Multitap  
{
public:
	Bouncy(float samplerate = 44100.f) : Multitap((unsigned long)(samplerate*MAX_DELAY)), //set initial delay size
								samplerate(samplerate)
								
	{
		fillRand();
	};

	void setSamplerate(float samplerate)
	{
		this->samplerate = samplerate;
	}

	virtual ~Bouncy() {}

	void setParameters(float max, float d, float a, float r)
	{
		float delay[32]; //delay in SAMPLES
		float amp[32];

		float delay_shape = powf(2.f,fabsf(d*2.f - 1.f)*6.f-3.f);
		float amp_shape   = powf(2.f,fabsf(a*2.f - 1.f)*6.f-3.f);
		
		float tmp = 0.f;
		long j;

		for(j=0;j<32;j++)
		{
			float x = (float)j/31.f;
			delay[j] = tmp;
			tmp += powf( (d < 0.5f) ? 1.f - x : x,delay_shape);
			amp[j] = powf( (a < 0.5f) ? 1.f - x : x,amp_shape) * ((1.f-r) + r*random[j]);
		}

		tmp = 1.f/tmp*max*samplerate*MAX_DELAY;

		for(j=0;j<32;j++)
			delay[j] = delay[j]*tmp;

		Multitap::setParameters(amp,delay); //set multi-tap delay :-)
	}

	void fillRand()
	{
		static const float randMaxInverse = 1.0f / (float)RAND_MAX;
		for(long i=0;i<32;i++)
			random[i] = (float)rand() * randMaxInverse;
	}

private:
	float samplerate;

	float random[32];
};

#endif // !defined(AFX_BOUNCY_H__844BCE47_92B7_4935_A8F1_A00C030941F4__INCLUDED_)
