// OnePoleLP.h: interface for the COnePoleLP class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "math.h"

class COnePoleLP  
{
public:
    void SetParams(float lambda);
    inline float GetVal(float in);
    COnePoleLP();
    virtual ~COnePoleLP();
    void suspend()
    {
        Value = 0.f;
    }
private:
    float Value,lambda;

};

inline float COnePoleLP::GetVal(float in)
{
    Value += lambda*(in - Value);
    
    if(fabs(Value) < 1e-10)
        Value = 0.f;
    
    if(fabs(Value) > 1e10)
        Value = 0.f;

    return Value;
}
