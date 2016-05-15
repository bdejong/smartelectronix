#ifndef __VstXSynth__
#define __VstXSynth__

#include <string.h>

#ifndef __AudioEffectX__
#include "audioeffectx.h"
#endif

enum
{
	kBits = 15,
	kAnd = 15,
	kOr,
	kSig,
	kNumParams,
	kNumOutputs = 1
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

	virtual bool getOutputProperties (long index, VstPinProperties* properties);
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual long getVendorVersion () {return 1;}
	virtual long canDo (char* text);

	float SAVE[kNumParams];
};

#endif
