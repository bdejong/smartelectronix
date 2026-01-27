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

void CFlanger::SetParams(float newFreq, float feed, float newDry, float newDist, float newMinDelay, float newMaxDelay)
{
    T.SetParams(newMinDelay, newMaxDelay);

    freq = newFreq;
    f = feed;
    dry = newDry;
    dist = newDist;
    maxdelay = newMaxDelay;
    mindelay = newMinDelay;

    if(dist <= 1)
        tmp2 = 1.f + dist;
    else
        tmp2 = 2 * static_cast<float>(sqrt(dist));
}
