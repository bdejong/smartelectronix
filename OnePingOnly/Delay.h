// Delay.h: interface for the CDelay class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DELAY_H__47BBC360_1AD9_11D4_B567_002018B8E8B7__INCLUDED_)
#define AFX_DELAY_H__47BBC360_1AD9_11D4_B567_002018B8E8B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDelay  
{
public:
	void setFeed(float feed);
	void KillBuffer();
	void setMaxDelay(int MaxDelay=44100*5);
	float getVal(float in);
	void setDelay(int d);
	CDelay(int MaxDelay);
	virtual ~CDelay();
private:
	float *buffer;
	float feed,feedtmp;
	int index;
	int pos;
	int delay;
	int MaxDelay;
};

#endif // !defined(AFX_DELAY_H__47BBC360_1AD9_11D4_B567_002018B8E8B7__INCLUDED_)
