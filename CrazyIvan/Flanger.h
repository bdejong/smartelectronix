// Flanger.h: interface for the CFlanger class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FLANGER_H__6F599F03_7426_11D3_9312_BE4A2D16EB39__INCLUDED_)
#define AFX_FLANGER_H__6F599F03_7426_11D3_9312_BE4A2D16EB39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

#endif // !defined(AFX_FLANGER_H__6F599F03_7426_11D3_9312_BE4A2D16EB39__INCLUDED_)
