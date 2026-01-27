#if !defined(AFX_CYANIDEEFFECT_H__D87291E2_F4C8_11D3_A03D_00AA00419C92__INCLUDED_)
#define AFX_CYANIDEEFFECT_H__D87291E2_F4C8_11D3_A03D_00AA00419C92__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "audioeffectx.h"
#include "ButterXOver.h"
#include "Polyphase.h"
#include "Shaper.h"

enum
{
	kShaper,
	kPreGain,
	kPreFilter,
	kPostGain,
	kPostFilter,
	kPreType,
	kPostType,
	kDryWet,
	kOverSample,
	kNumParams,
	kNumPrograms = 16
};

#pragma pack(1)

struct CyanideEffectProgram
{
	char kookie[24];
	unsigned char save[1024];
	char name[24];
};

class CyanideEffect  : public AudioEffectX
{
public:
	CyanideEffect(audioMasterCallback audioMaster);
	virtual ~CyanideEffect();
	
	//all VST-specific stuff....
	void  setProgram (long program);
	void  setProgramName (char *name);
	void  getProgramName (char *name);
	bool  getProgramNameIndexed(long category, long index, char* text);
	void  setParameter (long index, float value);
	float getParameter (long index);
	void  getParameterName (long index, char *label);
	void  getParameterDisplay (long index, char *text);
	void  getParameterLabel (long index, char *label);
	void suspend();
	void resume();
	void setSampleRate(float sampleRate);
	long getChunk(void** data, bool isPreset);
	long setChunk(void* data, long byteSize, bool isPreset);
	void process (float **inputs, float **outputs, long sampleFrames);
	void processReplacing (float **inputs, float **outputs, long sampleFrames);
	long processEvents (VstEvents* ev);
	bool getEffectName (char* name);
	bool getVendorString (char* text);
	bool getProductString (char* text);
	long canDo (char* text);

	bool getInputProperties (long index, VstPinProperties* properties);
	bool getOutputProperties (long index, VstPinProperties* properties);

protected:

	CShaper shaper;

private:

	void InternProcess(float **inputs, float **outputs, long sampleFrames, bool replace);

	CButterXOver F1L, F2L, F1R, F2R;
	CPolyphase P1[8], P2[8];
	
	float fPreGain;
	float fPreFilter;
	float fPostGain;
	float fPostFilter;
	float fPreType;
	float fPostType;
	float fDryWet;
	float fOverSample;
	float fOffset;
	
	float buffer1[FRAME_SIZE*16];
	float buffer2[FRAME_SIZE*16];
	float noise[FRAME_SIZE];
	long previous_oversample;

	CyanideEffectProgram programs[kNumPrograms];
	void loadProgram(long index);
	void saveProgram(long index);
};

#pragma pack()

#endif

