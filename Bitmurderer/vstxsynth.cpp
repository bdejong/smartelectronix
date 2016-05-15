#include <stdio.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#ifndef __VstXSynth__
#include "vstxsynth.h"
#endif

extern bool oome;

#include "MurderEditor.h"

VstXSynth::VstXSynth (audioMasterCallback audioMaster) : AudioEffectX (audioMaster, 0, kNumParams)
{
	setNumInputs (2);
	setNumOutputs (2);
	canProcessReplacing();
	hasVu(false);
	hasClip(false);
	setUniqueID ('BITM');
	canMono();

	resume();

	for(char i=0;i<kNumParams;i++)
		SAVE[i] = 0.f;

	editor = new MurderEditor(this);
}

VstXSynth::~VstXSynth ()
{
}

void VstXSynth::getParameterLabel (long index, char *label)
{
	strcpy(label,"");
}

void float2bool(float x, char *text)
{
	strcpy(text,x > 0.5f ? "ON" : "OFF");
}

void VstXSynth::getParameterDisplay (long index, char *text)
{
	float2bool(SAVE[index],text);

	/*
	switch(index)
	{
		case kDist : if(SAVE[kDist] > 0.5f) strcpy(text,"ON"); else  strcpy(text,"OFF"); break;
		case kCrazy : if(SAVE[kCrazy] > 0.5f) strcpy(text,"ON"); else  strcpy(text,"OFF"); break;
		case kRand : strcpy(text,"--------"); break;
		case kTame : dB2string(SAVE[kTame],text); break;
	}*/
}

void VstXSynth::getParameterName (long index, char *label)
{
	if(index < kBits)
		strcpy(label,"bit");
	else
		switch(index)
		{
			case kAnd	:	strcpy(label,"AND"); break;
			case kOr	:	strcpy(label,"OR"); break;
			case kSig	:	strcpy(label,"Sigmoid"); break;
		}
}

void VstXSynth::setParameter (long index, float value)
{
	SAVE[index] = value;

    if (editor)
        ((AEffGUIEditor*)editor)->setParameter(index, value);
}

float VstXSynth::getParameter (long index)
{
	return SAVE[index];
}

bool VstXSynth::getOutputProperties (long index, VstPinProperties* properties)
{
	if(index < kNumOutputs)
	{
		properties->flags = kVstPinIsActive;
		return true;
	}
	return false;
}

bool VstXSynth::getEffectName (char* name)
{
	strcpy (name, "Bit Murderer");
	return true;
}

bool VstXSynth::getVendorString (char* text)
{
	strcpy (text, "Bram @ Smartelectronix");
	return true;
}

bool VstXSynth::getProductString (char* text)
{
	strcpy (text, "Bit Murderer");
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
