// Flanger.h: interface for the CFlanger class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "TriOsc.h"
#include "MovingDelay.h"

class CFlanger  
{
public:
    void SetParams(float freq=0.5f, float feed=0.85f, float dry=0.5f, float dist=1.f, float mindelay=0, float maxdelay=100);
    float GetVal(float in);
    CFlanger();
    virtual ~CFlanger();

    void suspend()
    {
        D.suspend();
        out = 0.f;
    }

    CTriOsc T;

private:
    float f,freq,dry,dist,maxdelay,mindelay,out,tmp2;
    CMovingDelay D;
};
