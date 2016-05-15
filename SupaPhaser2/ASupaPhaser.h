// ASupaPhaser.h: interface for the ASupaPhaser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ASUPAPHASER_H__75AAFEE4_CF6A_11D3_9312_8E696182DB38__INCLUDED_)
#define AFX_ASUPAPHASER_H__75AAFEE4_CF6A_11D3_9312_8E696182DB38__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "audioeffectx.h"
#include "WavetableFPOsc.h"
#include "math.h"

#define DIST_FIX 0.8f

inline int Float2Int(double f, double min, double max)
{
	double retval = min + (max - min)*f;
	return (int)floor(retval + 0.5); 
};

//nice scaling function. Maps [0..1] to [0..max] in a bad way.
//bad, like in good :) It's SLOW but great, use wisely.
inline float ScaleFreq(float in, float param, float max)
{
	return (float)((exp(in*param)-1)/(exp(param)-1)*max);
}

const int MaxnStages = 23;
const float MaxnStagesF = 23.f;

class ASupaPhaser : public AudioEffectX
{
public:
	ASupaPhaser(audioMasterCallback audioMaster);
	~ASupaPhaser();
	
	virtual void  process(float **inputs, float **outputs, long sampleFrames);
	virtual void  processReplacing(float **inputs, float **outputs, long sampleFrames);
		
	virtual void  setParameter(long index, float value);
	virtual float getParameter(long index);

	virtual void  getParameterLabel(long index, char *label);
	virtual void  getParameterDisplay(long index, char *text);
	virtual void  getParameterName(long index, char *text);
	virtual void  setSampleRate(float sampleRate);
	
	virtual void  suspend();
	virtual void  setBlockSize(long blockSize);

	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual long canDo (char* text);

	virtual bool getInputProperties (long index, VstPinProperties* properties);
	virtual bool getOutputProperties (long index, VstPinProperties* properties);
	virtual bool getProgramNameIndexed (long category, long index, char* text);
	virtual bool copyProgram (long destination);
	virtual void setProgram (long program);
	virtual void setProgramName (char *name);
	virtual void getProgramName (char *name);

	enum
	{
		kAttack,
		kRelease,
		kMinEnv,
		kMaxEnv,
		kMixture,

		kFreq,
		kMinFreq,
		kMaxFreq,
			
		kExtend,
		kStereo,
		knStages,
		
		kDistort,
		kFeed,
		kDryWet,
		kGain,
		kInvert,
		
		kNumParams,
		
		kNumPrograms = 64
	};

	void getDisplay(long index, char *text);

private:
	
#pragma pack(1)
	float y1[MaxnStages];
	float in_11[MaxnStages];
	float y2[MaxnStages];
	float in_12[MaxnStages];
#pragma pack()

	void setPresets();

	float *p11;
	float *p12;
	float *Noise;

	//LFO vars
	
	float prevOutL, prevOutR, ENV1, ENV2;

	float freq, dist;

	CWavetableFPOsc Osc1, Osc2;

	float presets[kNumPrograms][kNumParams];
	char  presetName[kNumPrograms][32];
	float *SAVE;

	float gainMap(float mm) 
	{ 
		float db; 
		mm = 100.f - mm*100.f; 
		if (mm <= 0.f) { 
			db = 10.f; 
		} else if (mm < 48.f) { 
			db = 10.f - 5.f/12.f * mm; 
		} else if (mm < 84.f) { 
			db = -10.f - 10.f/12.f * (mm - 48.f); 
		} else if (mm < 96.f) { 
			db = -40.f - 20.f/12.f * (mm - 84.f); 
		} else if (mm < 100.f) { 
			db = -60.f - 35.f * (mm - 96.f); 
		} else db = -200.f; 

		return powf(10,(db/20.f));
	};
};

#endif // !defined(AFX_ASUPAPHASER_H__75AAFEE4_CF6A_11D3_9312_8E696182DB38__INCLUDED_)
