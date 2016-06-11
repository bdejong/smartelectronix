#pragma once

#include "public.sdk/source/vst2.x/audioeffectx.h"

class FFXTimeInfo
{
public:
	FFXTimeInfo() {};
	~FFXTimeInfo() {};

	// false if the VstTimeInfo pointer returned is null
	// i.e. the host doesn't support VstTimeInfo
	bool	isValid;

	// always valid
	double	samplePos;			// current location in samples
	double	sampleRate;

	bool	tempoIsValid;		// kVstTempoValid
	double	tempo;				// in beats/minute
	double	tempoBPS;			// in beats/second
	double	numSamplesInBeat;	// number of samples in 1 beat

	bool	ppqPosIsValid;		// kVstPpqPosValid
	double	ppqPos;				// 1 ppq = 1 MIDI beat (primary note division value)

	bool	barsIsValid;		// kVstBarsValid
	double	barStartPos;		// last bar start position, in ppq, relative to ppqPos

	bool	timeSigIsValid;		// kVstTimeSigValid
	long	timeSigNumerator;	// time signature
	long	timeSigDenominator;

	bool	samplesToNextBarIsValid;
	double	numSamplesToNextBar;

	bool	cyclePosIsValid;	// kVstCyclePosValid
	double	cycleStartPos;		// in terms of ppq
	double	cycleEndPos;		// in terms of ppq

	bool	playbackChanged;	// kVstTransportChanged
	bool	playbackIsOccuring;	// kVstTransportPlaying
	bool	cycleIsActive;		// kVstTransportCycleActive

/*	void writeDebugFile(const char *filename)
	{
		FILE *pf = fopen(filename,"wt");

		fprintf(pf,"bool isValid = %d\n",(long)isValid);

		fprintf(pf,"double samplePos = %f\n",samplePos);
		fprintf(pf,"double sampleRate = %f\n",sampleRate);
		fprintf(pf,"bool tempoIsValid = %d\n",(long)tempoIsValid);
		fprintf(pf,"double tempo = %f\n",tempo);
		fprintf(pf,"double tempoBPS = %f\n",tempoBPS);
		fprintf(pf,"double numSamplesInBeat = %f\n",numSamplesInBeat);
		fprintf(pf,"bool ppqPosIsValid = %d\n",(long)ppqPosIsValid);
		fprintf(pf,"double ppqPos = %f\n",ppqPos);
		fprintf(pf,"bool barsIsValid = %\dn",(long)barsIsValid);
		fprintf(pf,"double barStartPos = %f\n",barStartPos);
		fprintf(pf,"bool timeSigIsValid = %d\n",(long)timeSigIsValid);
		fprintf(pf,"long timeSigNumerator = %d\n",timeSigNumerator);
		fprintf(pf,"long timeSigDenominator = %d\n",timeSigDenominator);
		fprintf(pf,"bool samplesToNextBarIsValid = %b\n",(long)samplesToNextBarIsValid);
		fprintf(pf,"double numSamplesToNextBar = %f\n",numSamplesToNextBar);
		fprintf(pf,"bool cyclePosIsValid = %d\n",(long)cyclePosIsValid);
		fprintf(pf,"double cycleStartPos = %f\n",cycleStartPos);
		fprintf(pf,"double cycleEndPos = %f\n",cycleEndPos);
		fprintf(pf,"bool playbackChanged = %d\n",(long)playbackChanged);
		fprintf(pf,"bool playbackIsOccuring = %d\n",(long)playbackIsOccuring);
		fprintf(pf,"bool cycleIsActive = %d\n",(long)cycleIsActive);

		fclose(pf);
	}*/
};


#define NUMBERIO (2)
#define MAXSIZE (2000000)
#define BITSLIDES (7)
#define MAXSLIDES (1 << BITSLIDES)
#define FADETIME (150)

inline float hermite(float* wavetable, unsigned long nearest_sample, float x)
{
	float y0 = (nearest_sample == 0) ? 0.f : wavetable[nearest_sample-1];
	float y1=wavetable[nearest_sample];
	float y2=wavetable[nearest_sample+1];
	float y3=wavetable[nearest_sample+2];

    // 4-point, 3rd-order Hermite (x-form)
    float c0 = y1;
    float c1 = 0.5f * (y2 - y0);
    float c2 = y0 - 2.5f * y1 + 2.f * y2 - 0.5f * y3;
    float c3 = 1.5f * (y1 - y2) + 0.5f * (y3 - y0);

    return ((c3 * x + c2) * x + c1) * x + c0;
};

inline float hermiteInverse(float* wavetable, unsigned long nearest_sample, float x)
{
	float y3 = (nearest_sample == 0) ? 0.f : wavetable[nearest_sample-1];
	float y2=wavetable[nearest_sample];
	float y1=wavetable[nearest_sample+1];
	float y0=wavetable[nearest_sample+2];

	x = 1.f - x;

    // 4-point, 3rd-order Hermite (x-form)
    float c0 = y1;
    float c1 = 0.5f * (y2 - y0);
    float c2 = y0 - 2.5f * y1 + 2.f * y2 - 0.5f * y3;
    float c3 = 1.5f * (y1 - y2) + 0.5f * (y3 - y0);

    return ((c3 * x + c2) * x + c1) * x + c0;
};

struct GlitchParams
{
	unsigned long offset;
	bool reverse;
	bool stop;
	bool silence;
};

class SupaTrigger : public AudioEffectX
{
public:

	enum
	{
		kGranularity,
		kProbRearrange,
		kProbReverse,
		kProbSpeed,
		kSpeed,
		kProbRepeat,
		kProbSilence,
		kInstantReverse,
		kInstantSpeed,
		kInstantRepeat,
		kNumParams,
		kNumPrograms = 0
	};

	SupaTrigger(audioMasterCallback audioMaster);
	~SupaTrigger();

	virtual void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) override;

	virtual void setProgramName(char *name) override;
	virtual void getProgramName(char *name) override;
	virtual void setParameter(VstInt32 index, float value) override;
	virtual float getParameter(VstInt32 index) override;
	virtual void getParameterLabel(VstInt32 index, char *label) override;
	virtual void getParameterDisplay(VstInt32 index, char *text) override;
	virtual void getParameterName(VstInt32 index, char *text) override;
	virtual VstInt32 canDo(char* text) override;

	void randomize();

	virtual bool getEffectName(char* name) override;
	virtual bool getVendorString(char* text) override;
	virtual bool getProductString(char* text) override;

private:
	void convertVstTimeInfo(FFXTimeInfo *ffxtime);

	FFXTimeInfo timeInfo;

	unsigned long positionInMeasure;
	unsigned long previousSliceIndex;

	float *leftBuffer;
	float *rightBuffer;

	GlitchParams sequencer[MAXSLIDES];

	unsigned long granularityMask;
	unsigned long granularity;

	float SAVE[kNumParams];

	float gain;
	float speed, position;
	bool first;

	bool instantReverse;
	bool instantSlow;
	bool instantRepeat;
};
