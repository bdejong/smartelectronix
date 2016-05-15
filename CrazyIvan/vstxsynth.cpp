#include <stdio.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#ifndef __VstXSynth__
#include "vstxsynth.h"
#endif

VstXSynth::VstXSynth (audioMasterCallback audioMaster) : AudioEffectX (audioMaster, 0, kNumParams)
{
	setNumInputs (2);
	setNumOutputs (2);
	canProcessReplacing();
	hasVu(false);
	hasClip(false);
	setUniqueID ('CRZY');
	canMono();

	srand( (unsigned)time( NULL ) );

	resume();

	for(long i=0;i<kNumParams-1;i++)
		setParameter(i,0.f);

	randomise();

	S.SampleRateF = getSampleRate();
	S2.SampleRateF = getSampleRate();
	F.T.SampleRateF = getSampleRate();
}

VstXSynth::~VstXSynth ()
{
}

#define XXX(x,y) case x :	strcpy(label,y); break;

void VstXSynth::getParameterLabel (long index, char *label)
{
	switch(index)
	{
		XXX(kmurder,"");
		XXX(kbits,"");
		XXX(kand,"");
		XXX(kxor,"");
		XXX(kpower,"");

		XXX(kfreq1,"Hz");
		XXX(kdfreq1,"Hz");
		XXX(kvibfreq,"Hz");
		XXX(kdvibfreq,"Hz");
		XXX(kfeed1,"");

		XXX(kfreq2,"Hz");
		XXX(kmindelay,"samples");
		XXX(kmaxdelay,"samples");
		XXX(kdist,"");
		XXX(kfeed2,"");
		XXX(kdamp,"");

		XXX(klimiter,"amount");
		XXX(kattack,"ms");
		XXX(krelease,"ms");

		XXX(kamp,"dB");
		XXX(kdrywet,"");
		XXX(krandomise,"");
	}
}

void VstXSynth::getParameterDisplay (long index, char *text)
{
	switch(index)
	{
		case kamp		: dB2string(SCALED[index],text); break;
		case kmurder	:
		case kand		:
		case kxor		:
		case kpower		:
		case krandomise :
			if(SCALED[index] > 0.5f)
				strcpy(text,"ON");
			else
				strcpy(text,"OFF");
			break;
		default : float2string(SCALED[index],text); break;
	}
}

void VstXSynth::getParameterName (long index, char *label)
{
	switch(index)
	{
		XXX(kmurder,"bitmurderer");
		XXX(kbits,"  bitmask");
		XXX(kand,"  AND");
		XXX(kxor,"  XOR");
		XXX(kpower,"  power");

		XXX(kfreq1,"frequency 1");
		XXX(kdfreq1,"  diff frequency 1");
		XXX(kvibfreq,"  vib frequency");
		XXX(kdvibfreq,"  diff vib frequency");
		XXX(kfeed1,"  feedback 1");

		XXX(kfreq2,"frequency 2");
		XXX(kmindelay,"  min delay");
		XXX(kmaxdelay,"  max delay");
		XXX(kdist,"  distortion");
		XXX(kfeed2,"  feedback 2");
		XXX(kdamp,"  damping");
		XXX(kamp,"amplify");

		XXX(klimiter,"limiter");
		XXX(kattack,"  attack");
		XXX(krelease,"  release");

		XXX(kdrywet,"dry-wet");
		XXX(krandomise,"randomise");
	}
}

float sq(float x) { return x*x*x; };

void VstXSynth::setParameter (long index, float value)
{
	SAVE[index] = value;

	switch(index)
	{
	case kfreq1		:
	case kdfreq1	: SCALED[kfreq1] = sq(SAVE[kfreq1]) * 300.f;
					  SCALED[kdfreq1] = SCALED[kfreq1] * SAVE[kdfreq1];
					  break;
	case kvibfreq	: SCALED[index] = sq(SAVE[index]) * 10.f; break;
	case kdvibfreq	: SCALED[index] = sq(SAVE[index]) * 10.f; break;
	case kfeed1		: SCALED[index] = SAVE[index] * 0.99f; break;
	case kfeed2		: SCALED[index] = SAVE[index] * 0.99f; break;
	case kmindelay	: SCALED[index] = SAVE[index] * 10.f; break;
	case kmaxdelay	: SCALED[index] = SAVE[index] * 15000.f; break;
	case kdist		: SCALED[index] = SAVE[index] * 30.f; break;
	case kbits		: SCALED[index] = SAVE[index] * 32768.f * 0.49f; break;
	case kattack	:
	case krelease	: SCALED[index] = SAVE[index] * 200.f; break;
	case klimiter	: SCALED[index] = SAVE[index]; break;
	case kdamp		:
	case kfreq2		:
	case kmurder	:
	case kand		:
	case kxor		:
	case kpower		:
	case kdrywet	: SCALED[index] = SAVE[index]; break;
	case kamp		: SCALED[index] = (SAVE[index]*SAVE[index])*2.f; break;
	case krandomise	: if(SAVE[index] > 0.5f)
					  {
						  randomise();
						  updateDisplay();
					  };
					  break;
	}

	isDirty = true;
}

float VstXSynth::getParameter (long index)
{
	return SAVE[index];
}

bool VstXSynth::getOutputProperties (long index, VstPinProperties* properties)
{
	if(index < 2)
	{
		properties->flags = kVstPinIsActive;
		return true;
	}
	return false;
}

bool VstXSynth::getEffectName (char* name)
{
	strcpy (name, "Crazy Ivan");
	return true;
}

bool VstXSynth::getVendorString (char* text)
{
	strcpy (text, "Bram @ Smartelectronix");
	return true;
}

bool VstXSynth::getProductString (char* text)
{
	strcpy (text, "Crazy Ivan");
	return true;
}

long VstXSynth::canDo (char* text)
{
	if (!strcmp (text, "receiveVstEvents"))
		return -1;
	if (!strcmp (text, "receiveVstMidiEvent"))
		return -1;
	return -1;	// explicitly can't do; 0 => don't know
}

void VstXSynth::setProgram (long program)
{
}

void VstXSynth::setProgramName (char *name)
{
}

void VstXSynth::getProgramName (char *name)
{
	strcpy (name,"");
}
