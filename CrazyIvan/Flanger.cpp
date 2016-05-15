// Flanger.cpp: implementation of the CFlanger class.
//
//////////////////////////////////////////////////////////////////////

#include "Flanger.h"
#include "math.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFlanger::CFlanger()
{
	SetParams();
	suspend();
}

CFlanger::~CFlanger()
{}

float CFlanger::GetVal(float in)
{
	float a = in*tmp2/(1+dist*in*in);

	float tmp = a + f*(out-a);
	out = tmp - dry*(D.GetVal(tmp,T.GetVal(freq)) + tmp);

	if(fabs(out) < 1e-10)
		out = 0.f;

	if(fabs(out) > 1e10)
		out = 0.f;
		
	return out;
}

void CFlanger::SetParams(float freq, float feed, float dry, float dist, float mindelay, float maxdelay)
{
	T.SetParams(mindelay,maxdelay);
	
	this->freq = freq;
	this->f = feed;
	this->freq = freq;
	this->dry = dry;
	this->dist = dist;
	this->maxdelay = maxdelay;
	this->mindelay = mindelay;
	
	if(dist<=1)
		tmp2 = 1.f + dist;
	else
		tmp2 = 2*(float)sqrt(dist);
}
