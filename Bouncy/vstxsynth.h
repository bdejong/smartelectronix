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

	virtual void process(float **inputs, float **outputs, VstInt32 sampleframes);
	virtual void processReplacing(float **inputs, float **outputs, VstInt32 sampleframes);

	virtual void setProgram(VstInt32 program);
	virtual void setProgramName(char *name);
	virtual void getProgramName(char *name);
	virtual void setParameter(VstInt32 index, float value);
	virtual float getParameter(VstInt32 index);
	virtual void getParameterLabel(VstInt32 index, char *label);
	virtual void getParameterDisplay(VstInt32 index, char *text);
	virtual void getParameterName(VstInt32 index, char *text);
	virtual void setSampleRate(float sampleRate);
	virtual void resume();
	virtual VstInt32 processEvents (VstEvents* ev);

	virtual bool getOutputProperties (VstInt32 index, VstPinProperties* properties);
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual VstInt32 getVendorVersion () {return 1;}
	virtual VstInt32 canDo (char* text);

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
