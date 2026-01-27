#if !defined(AFX_BUTTERXOVER_H__7DD91B20_7446_11D4_9312_F0705373023F__INCLUDED_)
#define AFX_BUTTERXOVER_H__7DD91B20_7446_11D4_9312_F0705373023F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define nPre 1024

struct XOverCoeffs
{
	float aA1;
	float aA2;
	float bA2;
};

class CButterXOver  
{
public:
	float getFreq(long index);
	void processchange(float *input, float *output, long filter, long nSamp);
	void process(float *input, float *output, long nSamp);
	
	void precompute();
	void denormalise();
	void suspend();
	void setrate(float sr);
	void settype(float t);
	CButterXOver();
	virtual ~CButterXOver();
private:
	float outA1,in_1,out_2_A2,out_1_A2,m1,m2,in_2;

	float samplerate;
	long freq;

	static XOverCoeffs Pre[nPre];
	static float PreFreq[nPre];
};

#endif

