// EnvFollower.h: interface for the EnvFollower class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "math.h"

class EnvFollower  
{
public:
    EnvFollower();
    virtual ~EnvFollower();

    void suspend()
    {
        env = 0.f;
    }

    //should be in [0..200]
    void SetParams(float attack, float release)
    {
        if(attack < 0.1f) attack = 0.1f;
        if(release < 0.1f) release = 0.1f;

        a = 1.f - powf(10,log10f(0.001f)/(attack * 0.001f * SampleRateF));
        r = 1.f - powf(10,log10f(0.001f)/(release * 0.001f * SampleRateF));
    }

    float SampleRateF;

    inline float GetVal(float in)
    {
        float tmp = fabsf(in);

        if(tmp > env)
            env += a*(tmp - env);
        else
            env += r*(tmp - env);

        return env;
    }

private:
    float a,r,env;

};
