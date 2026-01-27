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
const size_t kFadeSize = 100001;
const size_t kBufferSize = 65537;


//----------------------------------------------------------------------------- 
// these macros do boring entry point stuff for us
DFX_ENTRY(MadShifta)
DFX_CORE_ENTRY(MadShiftaDSP)

//----------------------------------------------------------------------------- 
MadShifta::MadShifta(TARGET_API_BASE_INSTANCE_TYPE inInstance)
	: DfxPlugin(inInstance, kNumParams)	// 11 parameters
{
	initparameter_i(kTune, "Tune", 0, 0, kTuneMin, kTuneMax, kDfxParamUnit_semitones);
	initparameter_f(kFine, "Fine Tune", 0.0, 0.0, -100.0, 100.0, kDfxParamUnit_cents, kDfxParamCurve_);
	initparameter_f(kDelayLen, "Delay Length", 0.0, 0.0, 0.0, 100.0, kDfxParamUnit_percent);
	initparameter_f(kDelayFB, "Delay Feedback", 0.0, 0.0, 0.0, 100.0, kDfxParamUnit_percent);
	initparameter_f(kCutoff, "Cut-off", 99.0, 99.0, 1.0, 99.0, kDfxParamUnit_percent);
	initparameter_f(kResonance, "Resonance", 0.0, 0.0, 0.0, 98.0, kDfxParamUnit_percent);
	initparameter_indexed(kFType, "Filter Type", kFilterType_LowPass, kFilterType_LowPass, kFilterType_NumTypes);
	initparameter_f(kOutVol, "Output Volume", 100.0, 100.0, 0.0, 100.0, kDfxParamUnit_percent);
	initparameter_f(kDryWet, "Dry/Wet Mix", 100.0, 100.0, 0.0, 100.0, kDfxParamUnit_percent);
	initparameter_i(kRoot, "Root", 53, 53, -kTuneMin, NUM_NOTES-kTuneMax, kDfxParamUnit_notes);
	initparameter_indexed(kMMode, "MIDI Mode", kMidiMode_Back, kMidiMode_Back, kMidiMode_NumModes);

	// set the value strings for the filter type parameter
	setparametervaluestring(kFType, kFilterType_LowPass, "low-pass");
	setparametervaluestring(kFType, kFilterType_HighPass, "high-pass");
	// set the value strings for the MIDI mode parameter
	setparametervaluestring(kMMode, kMidiMode_Back, "back");
	setparametervaluestring(kMMode, kMidiMode_Hold, "hold");

	setparameterenforcevaluelimits(kCutoff);
	setparameterenforcevaluelimits(kResonance);

	setpresetname(0, "Mad Mother");	// default preset name
}

//-----------------------------------------------------------------------------------------
MadShiftaDSP::MadShiftaDSP(DfxPlugin * inDfxPlugin)
	: DfxPluginCore(inDfxPlugin)
{
	// allocate memory for the buffers
	fade = (float*) malloc(sizeof(float) * kFadeSize);
	delay = (float**) malloc(sizeof(float*) * kNumChannels);
	buffer = (float**) malloc(sizeof(float*) * kNumChannels);
	for (long i=0; i < kNumChannels; i++)
	{
		delay[i] = (float*) malloc(sizeof(float) * kBufferSize);
		buffer[i] = (float*) malloc(sizeof(float) * kBufferSize);
	}

	notecount = 0;

	nSize = 10;
	nBuffer = (unsigned) (1 << nSize); // same as 2^10, but faster
	// nBuffer is the buffer size of the fade buffer

	inp = 0;
	dp = 1;
	dp = dp-(1 << 12);

	// initialize the crossfading buffer with a raised cosine
	for (unsigned long i=0; i < nBuffer; i++)
		fade[i] = 0.5f+0.5f*cosf((((float)i/(float)(nBuffer-1))-0.5f)*2.0f*3.141592f);
}

//-----------------------------------------------------------------------------------------
MadShiftaDSP::~MadShiftaDSP()
{
	// deallocate the buffers
	free(fade);
	for (int i=0; i < kNumChannels; i++)
	{
		free(delay[i]);
		free(buffer[i]);
	}
	free(delay);
	free(buffer);
}

//-----------------------------------------------------------------------------------------
void MadShiftaDSP::reset()
{
	// empty those buffers!
	for (int i=0; i < kNumChannels; i++)
	{
		for (size_t j=0; j < kBufferSize; j++)
		{
			buffer[i][j] = 0.0f;
			delay[i][j] = 0.0f;
		}
	}
}

//----------------------------------------------------------------------------- 
// this is the filter section, currently a simple LP/HP filter
// is included, but this could be extended
float MadShiftaDSP::DoFilter(float i, float cutoff, float res, unsigned char ch)
{
	float fb = res + res/(1.0f-cutoff); // calculate feedback for resonance setting
	old1[ch] += cutoff * (i - old1[ch] + fb*(old1[ch]-old2[ch]));
	old2[ch] += cutoff * (old1[ch] - old2[ch]);
	if (filterType == kFilterType_LowPass)
		return old2[ch];  // return lowpass filtered signal
	else
		return i-old2[ch]; // return highpass filtered signal
}

//----------------------------------------------------------------------------- 
// This is the processing routine that is called for every sample
// from process() or processReplacing()
// In its current implementation, i[0] contains the left channel,
// i[1] the right channel, but this could be extended to more channels.
// NOTE: arrays are generally not the fastest way to access memory
// locations, but they show the structured approach better than direct
// pointer access. But feel free to change that... :-)
//
// The algorithm goes like this:
// out = filter(pitchshift(input + delay(out)*feedback))*volume
void MadShiftaDSP::DoProcess(float inAudio1, float inAudio2)
{
	float a, b;
	unsigned int ul1, ul2;
	unsigned char ch;
	float i[2];
	i[0] = inAudio1;
	i[1] = inAudio2;

	// increasing the delay buffer pointers, including
	// some more power-of-2 automatic wrap arounds
	delay_in = (delay_in+1) & (nDelay-1);
	delay_out = (delay_out+1) & (nDelay-1);

	ul1 = p1 >> 12; // the '12' is to keep the accuracy...
	ul2 = p2 >> 12;

	for (ch=0; ch < kNumChannels; ch++) // currently this is intended for 2 channels!
	{
		// write last value at delay_in in delay buffer
		delay[ch][delay_in] = last[ch];

		// store input value in buffer + feedback of delay buffer at delay_out
		buffer[ch][inp] = i[ch] + fDelayFB*delay[ch][delay_out];

		// apply the fading to smooth things
		a = buffer[ch][(inp-ul1) & (nBuffer-1)] * fade[ul1 & (nBuffer-1)];
		b = buffer[ch][(inp-ul2) & (nBuffer-1)] * fade[ul2 & (nBuffer-1)];

		// apply filter
		last[ch] = DoFilter(a+b, cut, reso, ch);

		a = fDryWet * last[ch];

		// do hard clipping, could be improved by saturation shaping:
		if (a > 1.0f)
			a = 1.0f;
		else if (a < -1.0f)
			a = -1.0f;

		// mix processed/unprocessed signal
		i[ch] = i[ch]*(1.0f-fDryWet) + a;
	}
	p1 = p1 - dp;
	p2 = p2 - dp;

	// increase/wrap input pointer
	inp = (inp+1) & (nBuffer-1);
}

/*
//----------------------------------------------------------------------------- 
void MadShifta::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
	for (long i=0; i < sampleFrames; i++)
	{
		DoProcess(inputs[0][i], inputs[1][i]);
		outputs[0][i] = inputs[0][i] * fOutVol;
		outputs[1][i] = inputs[1][i] * fOutVol;
	}
}
       
//----------------------------------------------------------------------------- 
// In this procedure, incoming MIDI events are handled
long MadShifta::processEvents(VstEvents *ev)
{
	long root, note;
	float s, nvol;

	for (long i=0; i < ev->numEvents; i++)
	{
		VstMidiEvent * event = (VstMidiEvent*) (ev->events[i]); // get current event
		char * midiData = event->midiData;
		long status = midiData[0] & 0xF0; // channel information is removed
		if ( (status == 0x90) && (midiData[2] > 0) ) // "note on" ?
		{
			note = (midiData[1] & 0x7F); // midi note
			root = rootKeyScaled(fRoot);
			notecount++;
			if (notecount == 1) oldtune = fTune;
			s = (float)(note-root) / (float)(kTuneMax-kTuneMin) + 0.5f;
			setParameter(kTune, s); // recalculate the tuning
		}
		else if ( ((status == 0x90) && (midiData[2] == 0)) || (status == 0x80) ) // "note off" ?
		{
			notecount--;
			if (notecount <= 0)
			{
				notecount = 0;
				if (fMMode < 0.5f) setParameter(kTune, oldtune);
			}
		}
	}
}

//----------------------------------------------------------------------------- 
void MadShifta::setParameter(long index, float value)
{
	double f;

	switch (index)
	{
		case kFine:
			fFine = value;
			// recalculate tune and finetune values for the algorithm:
			semitones = tuneScaled(fTune);
			f = powerof2(semitones/12.0);
			dp = (unsigned long) round(((0.5f-fFine)*(f*0.25f)+f)*(float)(1 << 12));
			dp = dp-(1 << 12);
			break;

		case kTune:
			// recalculate tune and finetune values for the algorithm:
			fTune = value;
			semitones = tuneScaled(fTune);
			f = powerof2(semitones/12.0);
			dp = (unsigned long) round(((0.5-fFine)*(f*0.25)+f)*(double)(1 << 12));
			dp = dp-(1 << 12);
			break;

		case kDelayLen:
			fDelayLen = value;
			// logarithmic scale for this control:
			value = round(-500.0f*log10((1.0f-value)+0.01f))/2000.0f;
			// lower boundary:
			if (value < 0.0001f)
				value = 0.0001f;
			displace = (unsigned long)round(44100.0f*value) + 1;
			nDelay = n_larger(displace);
			delay_in = 0;
			// nifty wrap-around:
			delay_out = (delay_in-displace) & (nDelay-1);
			inp = 0;
			p1 = 0;
			p2 = (nBuffer >> 1) << 12; // p2 starts at center of fade
			break;
	}
}
*/
