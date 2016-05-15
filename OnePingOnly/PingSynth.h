// PingSynth.h: interface for the PingSynth class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PINGSYNTH_H__52F698A4_DD93_11D3_9312_A3F8F095C838__INCLUDED_)
#define AFX_PINGSYNTH_H__52F698A4_DD93_11D3_9312_A3F8F095C838__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string.h>
#include "audioeffectx.h"
#include "delay.h"

#define nPing 128
#define nPar  6
#define lownote 0

class PingSynth;

class PingSynthProgram
{
	PingSynthProgram();
	~PingSynthProgram();
	friend class PingSynth;
private:
	char name[24];
	float fFreq[nPing],fDuration[nPing],fAmp[nPing],fBal[nPing],fNoise[nPing],fDist[nPing];
	float fDelay, fFeed, fMaster;
};

enum
{	
	kNumPrograms = 12,
	kNumOutputs = 2,
	kFreq = 0, kDuration, kAmp, kBal, kNoise, kDist,
	//here's the other ones
	kFeed = nPing*nPar, kDelay, kMaster,
	kNumParams
};

struct NoteData
{
	int		Note;
	long	Time;
	float	Velocity;
};

class PingSynth : public AudioEffectX
{
	friend class PingSynthProgram;
public:
	PingSynth(audioMasterCallback audioMaster);
	virtual ~PingSynth();

	virtual void process(float **inputs, float **outputs, long sampleFrames);
	virtual void processReplacing(float **inputs, float **outputs, long sampleFrames);
	virtual long processEvents (VstEvents* ev);

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

	bool getEffectName (char* name);
	bool getVendorString (char* text);
	bool getProductString (char* text);
	long canDo (char* text);
	bool getOutputProperties(long index, VstPinProperties* properties);
	bool getInputProperties(long index, VstPinProperties* properties);

private:
	void SetDuration(int index, float duration);
	void SetAmp(int index, float amp);
	void SetFreq(int index, float freq);
	void SetNoise(int index, float n);
	void SetBalance(int index, float balance);
	void SetDistortion(int index, float dist);
	void SetDelay(float delay);
	void SetFeed(float feed);
		
	PingSynthProgram *programs;
	NoteData Notes[200];
	int nNotes;
	
	//stuff
	float amp[nPing];
	float bal[nPing];
	float baltmp[nPing];
	float NoiseAmount[nPing];
	float d1[nPing], d2[nPing];

	//filters
	float alpha[nPing];
	float beta[nPing];
	float gamma[nPing];
	float g[nPing];
	float in[nPing],in_1[nPing],in_2[nPing],out_1[nPing],out_2[nPing];
	
	//data
	float fFreq[nPing];
	float fDuration[nPing];
	float fAmp[nPing];
	float fBal[nPing];
	float fNoise[nPing];
	float fDist[nPing];

	float fDelay, fFeed;
	float fMaster;

	//the delays
	CDelay *DelayL;
	CDelay *DelayR;
};

#endif // !defined(AFX_PINGSYNTH_H__52F698A4_DD93_11D3_9312_A3F8F095C838__INCLUDED_)
