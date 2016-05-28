// This is the effect part of the MadShifta source code

// All administrative stuff like parameter management is handled here,
// but of course also the implementation of the actual algorithm.

// Written by Tobias Fleischer/Tobybear (tobybear.de), 
// original algorithm source code & additional ideas by Bram from
// Smartelectronix (smartelectronix.com), 
// Mac VST & Audio Unit ports by Sophia Poirier from Smartelectronix

// Even if it is source code, this is meant to be a learning project,
// so don't even try to change just the graphics and sell it. :-)


#include "madshifta.h"


//----------------------------------------------------------------------------- 
const long kTuneMin = -24;
const long kTuneMax = 24;

const size_t kFadeSize = 100001;
const size_t kBufferSize = 65537;


//----------------------------------------------------------------------------- 
// these macros do boring entry point stuff for us
DFX_EFFECT_ENTRY(MadShifta)
DFX_CORE_ENTRY(MadShiftaDSP)

//----------------------------------------------------------------------------- 
MadShifta::MadShifta(TARGET_API_BASE_INSTANCE_TYPE inInstance)
	: DfxPlugin(inInstance, kNumParams)	// 11 parameters
{
	initparameter_i(kTune, "Tune", 0, 0, kTuneMin, kTuneMax, kDfxParamUnit_semitones);
	initparameter_f(kFine, "Fine Tune", 0.0, 0.0, -100.0, 100.0, kDfxParamUnit_cents);
	initparameter_f(kDelayLen, "Delay Length", 0.0, 0.0, 0.0, 100.0, kDfxParamUnit_percent);
	initparameter_f(kDelayFB, "Delay Feedback", 0.0, 0.0, 0.0, 100.0, kDfxParamUnit_percent);
	initparameter_f(kCutoff, "Cut-off", 99.0, 99.0, 1.0, 99.0, kDfxParamUnit_percent);
	initparameter_f(kResonance, "Resonance", 0.0, 0.0, 0.0, 98.0, kDfxParamUnit_percent);
	initparameter_list(kFType, "Filter Type", kFilterType_LowPass, kFilterType_LowPass, kFilterType_NumTypes);
	initparameter_f(kOutVol, "Output Volume", 100.0, 100.0, 0.0, 100.0, kDfxParamUnit_percent, kDfxParamCurve_cubed);
	initparameter_f(kDryWet, "Dry/Wet Mix", 100.0, 100.0, 0.0, 100.0, kDfxParamUnit_percent);
	initparameter_i(kRoot, "Root", 60, 60, -kTuneMin, NUM_NOTES-kTuneMax, kDfxParamUnit_notes);
	initparameter_list(kMMode, "MIDI Mode", kMidiMode_Back, kMidiMode_Back, kMidiMode_NumModes);

	// set the value strings for the filter type parameter
	setparametervaluestring(kFType, kFilterType_LowPass, "low-pass");
	setparametervaluestring(kFType, kFilterType_HighPass, "high-pass");
	// set the value strings for the MIDI mode parameter
	setparametervaluestring(kMMode, kMidiMode_Back, "back");
	setparametervaluestring(kMMode, kMidiMode_Hold, "hold");

	setparameterenforcevaluelimits(kCutoff, true);
	setparameterenforcevaluelimits(kResonance, true);

	setpresetname(0, "Mad Mother");	// default preset name
}

//-----------------------------------------------------------------------------------------
MadShiftaDSP::MadShiftaDSP(DfxPlugin * inDfxPlugin)
	: DfxPluginCore(inDfxPlugin)
{
	// allocate memory for the buffers
	fade = (float*) malloc(sizeof(float) * kFadeSize);
	delay = (float*) malloc(sizeof(float) * kBufferSize);
	buffer = (float*) malloc(sizeof(float) * kBufferSize);

	notecount = 0;

	// nBuffer is the buffer size of the fade buffer
	nBuffer = (unsigned) (1 << 10); // same as 2^10, but faster

	inp = 0;
	dp = 1;
	dp = dp - (1 << 12);

	// XXX these initializations were all missing from Toby's code
	last = old1 = old2 = 0.0f;
	oldtune = 0;
	// XXX ...though these ones should happen upon first execution of process()
	p1 = p2 = 0;
	delay_in = delay_out = 0;
	nDelay = 2;

	// initialize the crossfading buffer with a raised cosine
	for (unsigned long i=0; i < nBuffer; i++)
		fade[i] = 0.5f+0.5f*cosf((((float)i/(float)(nBuffer-1))-0.5f)*2.0f*kDFX_PI_f);
}

//-----------------------------------------------------------------------------------------
MadShiftaDSP::~MadShiftaDSP()
{
	// deallocate the buffers
	if (fade != NULL)
		free(fade);
	fade = NULL;

	if (delay != NULL)
		free(delay);
	delay = NULL;

	if (buffer != NULL)
		free(buffer);
	buffer = NULL;
}

//-----------------------------------------------------------------------------------------
void MadShiftaDSP::reset()
{
	// empty those buffers!
	for (size_t i=0; i < kBufferSize; i++)
	{
		buffer[i] = 0.0f;
		delay[i] = 0.0f;
	}
}

//----------------------------------------------------------------------------- 
// this is the filter section
// currently a simple LP/HP filter is included, but this could be extended
float MadShiftaDSP::DoFilter(float i, float cutoff, float res)
{
	float fb = res + res/(1.0f-cutoff); // calculate feedback for resonance setting
	old1 += cutoff * (i - old1 + fb*(old1-old2));
	old2 += cutoff * (old1 - old2);
	if (filterType == kFilterType_LowPass)
		return old2;  // return lowpass filtered signal
	else
		return i-old2; // return highpass filtered signal
}

//----------------------------------------------------------------------------- 
// This is the processing routine that is called for every sample
// from process()
//
// The algorithm goes like this:
// out = filter(pitchshift(input + delay(out)*feedback))*volume
float MadShiftaDSP::DoProcess(float inAudio)
{
	// square root for equal power mix
	float inputGain = sqrtf(1.0f - fDryWet);
	float outputGain = sqrtf(fDryWet);

	// increasing the delay buffer pointers, including
	// some more power-of-2 automatic wrap-arounds
	delay_in = (delay_in+1) & (nDelay-1);
	delay_out = (delay_out+1) & (nDelay-1);

	unsigned int ul1 = p1 >> 12; // the '12' is to keep the accuracy...
	unsigned int ul2 = p2 >> 12;

	// write last value at delay_in in delay buffer
	delay[delay_in] = last;

	// store input value in buffer + feedback of delay buffer at delay_out
	buffer[inp] = inAudio + fDelayFB*delay[delay_out];

	// apply the fading to smooth things
	float a = buffer[(inp-ul1) & (nBuffer-1)] * fade[ul1 & (nBuffer-1)];
	float b = buffer[(inp-ul2) & (nBuffer-1)] * fade[ul2 & (nBuffer-1)];

	// apply filter
	last = DoFilter(a+b, fCutoff, fResonance);

	a = last * outputGain;

// XXX hard clipping sux yeh? and silly in a floating-point bus context
#if 0
	// do hard clipping, could be improved by saturation shaping:
	if (a > 1.0f)
		a = 1.0f;
	else if (a < -1.0f)
		a = -1.0f;
#endif

	// mix processed/unprocessed signal
	a += (inAudio * inputGain);

	p1 = p1 - dp;
	p2 = p2 - dp;

	// increase/wrap input pointer
	inp = (inp + 1) & (nBuffer - 1);

	return a;
}

//----------------------------------------------------------------------------- 
void MadShiftaDSP::process(const float * inAudio, float * outAudio, unsigned long inNumFrames, bool inReplacing)
{
	// process the MIDI notes before fetching parameter values since 
	// the MIDI notes can potentially alter parameter values
	if (GetChannelNum() == 0)
		HandleMusicalNotes();


	// fetch the current parameter values
	courseTune = getparameter_i(kTune);
	fFine = getparameter_gen(kFine);
	fDelayLen = getparameter_scalar(kDelayLen);
	fDelayFB = getparameter_scalar(kDelayFB);
	fCutoff = getparameter_scalar(kCutoff);
	fResonance = getparameter_scalar(kResonance);
	filterType = getparameter_i(kFType);
	fOutVol = getparameter_scalar(kOutVol);
	fDryWet = getparameter_scalar(kDryWet);
	rootKey = getparameter_i(kRoot);
	midiMode = getparameter_i(kMMode);

	// recalculate tune and finetune values for the algorithm:
	if ( getparameterchanged(kTune) || getparameterchanged(kFine) )
	{
		double f = powerof2((double)courseTune/12.0);
		dp = (unsigned long) round(((fFine-0.5)*(f*0.25)+f)*(double)(1 << 12));
		dp -= (1 << 12);
	}

	// logarithmic scale for delay length:
	if ( getparameterchanged(kDelayLen) )
	{
		double f = round(-500.0*log10((1.0-fDelayLen)+0.01))/2000.0;
		// lower boundary:
		if (f < 0.0001)
			f = 0.0001;
		unsigned long displace = (unsigned long)round(getsamplerate()*f) + 1;
		nDelay = n_larger(displace);
		delay_in = 0;
		// nifty wrap-around:
		delay_out = (delay_in-displace) & (nDelay-1);
		inp = 0;
		p1 = 0;
		p2 = (nBuffer >> 1) << 12; // p2 starts at center of fade
	}


	// render the audio output
	for (unsigned long i=0; i < inNumFrames; i++)
		outAudio[i] = DoProcess(inAudio[i]) * fOutVol;
}

//----------------------------------------------------------------------------- 
// In this procedure, incoming MIDI events are handled
void MadShiftaDSP::HandleMusicalNotes()
{
	DfxMidi * midiStream = dfxplugin->getmidistream();
	if (midiStream == NULL)
		return;

	bool tuneParamChanged = false;
	for (long i=0; i < midiStream->numBlockEvents; i++)
	{
		int currentNote = midiStream->blockEvents[i].byte1;

		switch (midiStream->blockEvents[i].status)
		{
			case kMidiNoteOn:
				notecount++;
				if (notecount == 1)
					oldtune = courseTune;
				dfxplugin->setparameter_i(kTune, currentNote); // recalculate the tuning
				tuneParamChanged = true;
				break;

			case kMidiNoteOff:
				notecount--;
				if (notecount <= 0)
				{
					notecount = 0;
					if (midiMode == kMidiMode_Back)
					{
						dfxplugin->setparameter_i(kTune, oldtune);
						tuneParamChanged = true;
					}
				}
				break;

			case kMidiCC:
				if (midiStream->blockEvents[i].byte1 == kMidiCC_AllNotesOff)
				{
					if (notecount > 0)
					{
						if (midiMode == kMidiMode_Back)
						{
							dfxplugin->setparameter_i(kTune, oldtune);
							tuneParamChanged = true;
						}
					}
					notecount = 0;
				}
				break;

			default:
				break;
		}
	}

	if (tuneParamChanged)
		dfxplugin->postupdate_parameter(kTune);
}
