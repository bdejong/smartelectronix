// TriOsc.cpp: implementation of the CTriOsc class.
//
//////////////////////////////////////////////////////////////////////

#include "TriOsc.h"
#include "math.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTriOsc::CTriOsc()
{
    SetParams();

    sample = 0.f;
    slope = 1.f;
}

CTriOsc::~CTriOsc()
{}

float CTriOsc::GetVal(float freq)
{
    sample += slope*fabsf(freq)/SampleRateF;

    if(sample > 1.f)
    {
        slope = -slope;
        sample = 2.f - sample;
    }

    if(sample < 0.f)
    {
        slope = -slope;
        sample = -sample;
    }

    if(fabs(sample) < 1e-10)
        sample = 0.f;
    if(fabs(sample) > 1e10)
        sample = 0.f;

    return min + sample*(max - min);

}

void CTriOsc::SetParams(float min, float max)
{
    if(max>min)
    {
        this->max = max;
        this->min = min;
    }
    else
    {
        this->max = min;
        this->min = max;
    }
}
