// H2Oeffect.h: interface for the H2Oeffect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_H2OEFFECT_H__B37D5BC4_409C_11D4_9312_A8A1CBFC3939__INCLUDED_)
#define AFX_H2OEFFECT_H__B37D5BC4_409C_11D4_9312_A8A1CBFC3939__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string.h>
#include "audioeffectx.h"
#include "compressor.h"

class H2Oeffect;

class H2Oprogram
{
	H2Oprogram();
	~H2Oprogram();
	friend class H2Oeffect;
private:
	char name[24];

	float fPreAmp,fAttack,fRelease,fAmount,fPostAmp,fSaturate;
};

enum
{	
	kNumPrograms = 2,
	kNumOutputs = 2,
	kNumInputs = 2,
	
	kPreAmp = 0,
	kAttack,
	kRelease,
	kAmount,
	kPostAmp,
	kSaturate,
	
	kNumParams
};

class H2Oeffect : public AudioEffectX
{
	friend class H2Oprogram;
public:
	H2Oeffect(audioMasterCallback audioMaster);
	virtual ~H2Oeffect();
	
	virtual void process(float **inputs, float **outputs, long sampleFrames);
	virtual void processReplacing(float **inputs, float **outputs, long sampleFrames);
	
	virtual void resume();
	virtual void suspend();

	virtual void setProgramName(char *name);
	virtual void getProgramName(char *name);
	virtual void setProgram (long program);
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
	virtual long canDo (char* text);

private:
	H2Oprogram *programs;
	float fPreAmp,fAttack,fRelease,fAmount,fPostAmp,fSaturate;

	CCompressor *comp;
};

#endif // !defined(AFX_H2OEFFECT_H__B37D5BC4_409C_11D4_9312_A8A1CBFC3939__INCLUDED_)
