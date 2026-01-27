// WavetableOsc.h: interface for the CWavetableOsc class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

//#include "fastcast.h"

class CWavetableFPOsc
{
public:
	void setrate(float samplerate);
	void setfreq(float freq);
	void setstereo(unsigned int otherphase, float stereo);

	unsigned int getphase()
	{
		return phase;
	};

	void reset();

	inline float GetVal();

	CWavetableFPOsc();

	virtual ~CWavetableFPOsc();

private:
	void CreateFracTable();

	float *buffer;
	float *dbuffer;
	float *fractable;

	float fmagic;

	unsigned int phase;
	unsigned int tmp;
};

//no FM, no SO
inline float CWavetableFPOsc::GetVal()
{
	unsigned int index = (phase>>12) & 0xfff;

	float osc = buffer[index] + fractable[phase & 0xfff]*dbuffer[index];

	phase += tmp;

	return osc;
}
