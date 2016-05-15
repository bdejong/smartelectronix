#ifndef __SMARTELECTRONIXDISPLAY_H
#define __SMARTELECTRONIXDISPLAY_H

#include "audioeffectx.h"

#include "vstgui.h"
#include "Defines.h"

class CSmartelectronixDisplay : public AudioEffectX
{
public:

	/*
		trigger type : free / rising / falling / internal / external   (this order)
		channel : left / right
		all else : on / off
	*/

	// VST parameters
	enum
	{
		kTriggerSpeed, //internal trigger speed, knob
		kTriggerType, //trigger type, selection
		kTriggerLevel,	//trigger level, slider
		kTriggerLimit, //retrigger threshold, knob
		kTimeWindow, //X-range, knob
		kAmpWindow, //Y-range, knob
		kSyncDraw, //sync redraw, on/off
		kChannel, // channel selection, left/right
		kFreeze, //freeze display, on/off
		kDCKill, // kill DC, on/off
		kNumParams
	};

	// VST elements
	enum
	{
		kNumPrograms = 0,
#if SIMPLE_VERSION
		kNumInputChannels = 2,
		kNumOutputChannels = 2,	//VST doesn't like 0 output channels ;-)
#else
		kNumInputChannels = 3,
		kNumOutputChannels = 0,
#endif
	};

	// trigger types
	enum
	{
		kTriggerFree = 0,
		kTriggerRising,
		kTriggerFalling,
		kTriggerInternal,
#if !SIMPLE_VERSION
		kTriggerExternal,
#endif
		kNumTriggerTypes
	};

	CSmartelectronixDisplay(audioMasterCallback audioMaster);
	~CSmartelectronixDisplay();

	virtual void	process(float **inputs, float **outputs, long sampleFrames);
	virtual void	processReplacing(float **inputs, float **outputs, long sampleFrames);
	
	virtual void	setProgramName(char *name);
	virtual void	setProgram(long index);
	virtual void	getProgramName(char *name);
	virtual void	setParameter(long index, float value);
	virtual float	getParameter(long index);
	virtual void	getParameterLabel(long index, char *label);
	virtual void	getParameterDisplay(long index, char *text);
	virtual void	getParameterName(long index, char *text);

	void getDisplay(long index, char *text);

	virtual long	canDo(char* text);
	virtual bool	getEffectName (char* name);
	virtual bool	getVendorString (char* text);
	virtual bool	getProductString (char* text);

	virtual void	suspend();
	virtual void	resume();
	virtual void	setSampleRate(float sampleRate);
	virtual void	setBlockSize (long blockSize);

	// these should be protected...
	CPoint peaks[OSC_WIDTH*2];
	CPoint copy[OSC_WIDTH*2];

protected:

	// the actual algo :-)
	void processSub(float **inputs, long sampleFrames);

	// index into the peak-array
	unsigned long index;
	
	// counter which is used to set the amount of samples / pixel
	double counter;
	
	// max/min peak in this block
	float max, min, maxR, minR;
	
	// the last peak we encountered was a maximum!
	bool lastIsMax;

	// the previous sample (for edge-triggers)
	float previousSample;

	// the internal trigger oscillator
	double triggerPhase;

	// stupid VST parameter save
	float SAVE[kNumParams];

	// trigger limiter!
	long triggerLimitPhase;

	// dc killer
	double dcKill, dcFilterTemp;
};

#endif
