// TriOsc.h: interface for the CTriOsc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRIOSC_H__A59F05A0_7124_11D3_9312_E0861F7E7E38__INCLUDED_)
#define AFX_TRIOSC_H__A59F05A0_7124_11D3_9312_E0861F7E7E38__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTriOsc  
{
public:
	void SetPhase(float phase);
	void SetParams(float min=-1,float max=1);
	float GetVal(float f);
	void suspend() { sample = 0.f;};
	CTriOsc();
	virtual ~CTriOsc();

	float SampleRateF;

private:
	float sample, slope, min, max;
};

#endif // !defined(AFX_TRIOSC_H__A59F05A0_7124_11D3_9312_E0861F7E7E38__INCLUDED_)
