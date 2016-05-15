// OnePoleLP.h: interface for the COnePoleLP class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ONEPOLELP_H__A59F05A2_7124_11D3_9312_E0861F7E7E38__INCLUDED_)
#define AFX_ONEPOLELP_H__A59F05A2_7124_11D3_9312_E0861F7E7E38__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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



#endif // !defined(AFX_ONEPOLELP_H__A59F05A2_7124_11D3_9312_E0861F7E7E38__INCLUDED_)
