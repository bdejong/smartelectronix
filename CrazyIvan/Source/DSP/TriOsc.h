// TriOsc.h: interface for the CTriOsc class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CTriOsc  
{
public:
    void SetPhase(float phase);
    void SetParams(float min=-1,float max=1);
    float GetVal(float f);
    void suspend() { sample = 0.f; }
    CTriOsc();
    virtual ~CTriOsc();

    float SampleRateF;

private:
    float sample, slope, min, max;
};
