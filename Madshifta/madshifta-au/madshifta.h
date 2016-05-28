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
class MadShiftaDSP : public DfxPluginCore
{
public:
	MadShiftaDSP(DfxPlugin * inDfxPlugin);
	virtual ~MadShiftaDSP();
	virtual void process(const float * inAudio, float * outAudio, unsigned long inNumFrames, bool inReplacing=true);
	virtual void reset();

private:
	float DoProcess(float inAudio);
	float DoFilter(float i, float cutoff, float res);
	void HandleMusicalNotes();

	// the current parameter set in memory:
	float fDryWet, fDelayLen, fDelayFB, fFine, fCutoff, fResonance, fOutVol;
	long courseTune, rootKey, filterType, midiMode;

	// the buffers:
	float * fade;
	float * delay, // the delay buffer
	* buffer; // the shift buffer

	// some variables for the algorithm:
	float last, old1, old2; // filter variables
	long oldtune; // last valid MIDI tuning
	unsigned long p1, p2, // output pointers
	delay_in, delay_out, // delay buffer pointers
	inp, // input pointer for the shift-buffer
	dp, // pointer increment
	nBuffer, // determine size of fade buffer
	nDelay;

	long notecount; // number of notes currently held down

};


//----------------------------------------------------------------------------- 
class MadShifta : public DfxPlugin
{
public:
	MadShifta(TARGET_API_BASE_INSTANCE_TYPE inInstance);
};



//----------------------------------------------------------------------------- 
// this function gives you the smallest power of 2
// that is larger than/equal to "number"
inline unsigned int n_larger(unsigned int number)
{
	unsigned int numb = 1;
	while (numb < number)
		numb = numb << 1;
	return numb;
}

//----------------------------------------------------------------------------- 
// simple approximation
inline double powerof2(double a)
{
	return exp(a * 0.69314718056);
}


#endif
