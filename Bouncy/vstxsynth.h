#ifndef __VstXSynth__
#define __VstXSynth__

#include <string.h>

#ifndef __AudioEffectX__
#include "audioeffectx.h"
#endif

#include "Bouncy.h"

#include <stdio.h>
#include <stdlib.h>

#define NUM_BEATS 16.f

enum
{
	kMaxDelay = 0, //big head
	kDelayShape, //small knob left
	kAmpShape, //small knob right
	kRandAmp, //slider
	kRenewRand, //onoff
	kNumParams,
	kNumOutputs = 2
};


class VstXSynth : public AudioEffectX
{
public:

	VstXSynth(audioMasterCallback audioMaster);

	~VstXSynth();

	virtual void process(float **inputs, float **outputs, long sampleframes);
	virtual void processReplacing(float **inputs, float **outputs, long sampleframes);

	virtual void setProgram(long program);
	virtual void setProgramName(char *name);
	virtual void getProgramName(char *name);
	virtual void setParameter(long index, float value);
	virtual float getParameter(long index);
	virtual void getParameterLabel(long index, char *label);
	virtual void getParameterDisplay(long index, char *text);
	virtual void getParameterName(long index, char *text);
	virtual void setSampleRate(float sampleRate);
	virtual void resume();
	virtual long processEvents (VstEvents* ev);

	virtual bool getOutputProperties (long index, VstPinProperties* properties);
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual long getVendorVersion () {return 1;}
	virtual long canDo (char* text);

private:

	void setParam()
	{
		isDirty = true;
	}

	Bouncy *delayL;
	Bouncy *delayR;

	bool isDirty;

	float save[kNumParams];

	float BPM;

#if WIN32
	FILE *pf;
#endif
};

#endif
