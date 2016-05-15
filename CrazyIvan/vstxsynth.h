#ifndef __VstXSynth__
#define __VstXSynth__

#include <string.h>

#ifndef __AudioEffectX__
#include "audioeffectx.h"
#endif

#include "OnePoleLP.h"
#include "MovingDelay.h"
#include "TriOsc.h"
#include "Flanger.h"
#include "EnvFollower.h"

enum
{
	kmurder,
		kbits,
		kand,
		kxor,
		kpower,
	kfreq1,
		kdfreq1,
		kvibfreq,
		kdvibfreq,
		kfeed1,
	kfreq2,
		kmindelay,
		kmaxdelay,
		kdist,
		kdamp,
		kfeed2,
	klimiter,
		kattack,
		krelease,
	kamp,
	kdrywet,
	krandomise, //allways keep this 1 last
	kNumParams
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

	virtual bool getOutputProperties (long index, VstPinProperties* properties);
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual long getVendorVersion () {return 1;}
	virtual long canDo (char* text);

	void randomise();

	short mask;
	float ivan_output;

	COnePoleLP LP;
	CMovingDelay Delay;
	CTriOsc S,S2;
	CFlanger F;
	EnvFollower E;

	bool isDirty;

	float SAVE[kNumParams];
	float SCALED[kNumParams];
};

#endif
