#ifndef __MADSHIFTA_H
#define __MADSHIFTA_H


#include "dfxplugin.h"



//----------------------------------------------------------------------------- 
 // the set of constants defining the effect's parameter set:
enum {
	kTune,
	kFine,
	kDelayLen,
	kDelayFB,
	kCutoff,
	kResonance,
	kFType,
	kOutVol,
	kDryWet,
	kRoot,
	kMMode,

	kNumParams
};

enum {
	kFilterType_LowPass,
	kFilterType_HighPass,
	kFilterType_NumTypes
};

enum {
	kMidiMode_Back,
	kMidiMode_Hold,
	kMidiMode_NumModes
};


//----------------------------------------------------------------------------- 
const long kNumChannels = 2;

const long kTuneMin = -24;
const long kTuneMax = 24;


//----------------------------------------------------------------------------- 
// these are some macros for converting some of the parameters' 
// values from VST-style 0.0 to 1.0 value to their actual values
#define tuneScaled(value)   ( (int) round( ((value)*(float)(kTuneMax-kTuneMin)) + (float)kTuneMin ) )
#define fineTuneScaled(value)   ( ((value)*200.0f) - 100.0f )
#define rootKeyScaled(value)   ( (int)round( ((value)*(float)(NUM_NOTES-(kTuneMax-kTuneMin))) ) - kTuneMin )



//----------------------------------------------------------------------------- 
class MadShiftaDSP : public DfxPluginCore
{
public:
	MadShiftaDSP(DfxPlugin * inDfxPlugin);
	virtual ~MadShiftaDSP();
	virtual void process(const float * in, float * out, unsigned long inNumFrames, bool replacing=true);
	virtual void reset();

private:
	void DoProcess(float inAudio1, float inAudio2);
	float DoFilter(float i, float cutoff, float res, unsigned char ch);

	// the current parameter set in memory:
	long filterType;

	// the buffers:
	float * fade;
	float ** delay, // the delay buffer
	** buffer; // the shift buffer

	// some variables for the algorithm:
	float last[kNumChannels], old1[kNumChannels], old2[kNumChannels]; // filter variables
	int semitones; // semitone shift (can be negative)
	float oldtune, // last valid MIDI tuning
	cut, reso; // current cutoff and resonance values
	unsigned long p1, p2, // output pointers
	delay_in, delay_out, // delay buffer pointers
	inp, // input pointer for the shift-buffer
	dp, // pointer increment
	nBuffer, nSize, // determine size of fade buffer
	displace, nDelay;

	int notecount; // number of notes currently held down

};


//----------------------------------------------------------------------------- 
class MadShifta : public DfxPlugin
{
public:
	MadShifta(TARGET_API_BASE_INSTANCE_TYPE inInstance);
};



// this function gives you the smallest power of 2
// that is larger than/equal to "number"
inline unsigned int n_larger(unsigned int number)
{
	unsigned int numb = 1;
	while (numb < number) numb = numb << 1;
	return numb;
}

// simple approximation
inline double powerof2(double a)
{
	return exp(a * 0.69314718056);
}


#endif
