#if !defined(AFX_POLYPHASE_H__114FEB20_A9D7_11D4_B567_0000E8C8DA75__INCLUDED_)
#define AFX_POLYPHASE_H__114FEB20_A9D7_11D4_B567_0000E8C8DA75__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPolyphase  
{
public:
	void Downsample(float *input, float *output, long nSamples);
	void Upsample(float *input, float *output, long nSamples);
	void suspend();
	void denormalise();
	CPolyphase();
	virtual ~CPolyphase();
private:
	float uout[3];
	float uin_1[3];

	float lout[3];
	float lin_1[3];
};

#endif
