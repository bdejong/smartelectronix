// H2Oeffect.cpp: implementation of the H2Oeffect class.
//
//////////////////////////////////////////////////////////////////////

#include "H2Oeffect.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math.h"

#include "AEffEditor.hpp"

#include "asciitable.h"
#include "compressor.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
H2Oprogram::H2Oprogram()
{
	strcpy (name, "Init");

	fPreAmp = 0.5f;
	fAttack = 0.3f;
	fRelease = 0.1f;
	fAmount = 0.6f;
	fPostAmp = 0.6f;
	fSaturate = 1.f;
}

H2Oprogram::~H2Oprogram()
{
}

H2Oeffect::H2Oeffect(audioMasterCallback audioMaster)
	:AudioEffectX(audioMaster,kNumPrograms,kNumParams)
{
	programs = new H2Oprogram[kNumPrograms];

	comp = new CCompressor(this->getSampleRate());

	if (programs)
		setProgram (0);

	if (audioMaster)
	{
		setNumInputs (2);
		setNumOutputs (kNumOutputs);
		canProcessReplacing(true);
		canMono(true);
		setUniqueID('H2OF');
	}

	resume();
}

H2Oeffect::~H2Oeffect()
{
	if(programs)
		delete [] programs;

	delete comp;
}

void H2Oeffect::process(float **inputs, float **outputs, VstInt32 sampleFrames)
{
	comp->process(inputs,outputs,sampleFrames);
}

void H2Oeffect::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames)
{
	comp->processReplacing(inputs,outputs,sampleFrames);
}

void H2Oeffect::setProgram (VstInt32 program)
{
	H2Oprogram *ap = &programs[program];
	curProgram = program;

	setParameter(kAttack,ap->fAttack);
	setParameter(kRelease,ap->fRelease);
	setParameter(kAmount,ap->fAmount);
	setParameter(kPostAmp,ap->fPostAmp);
	setParameter(kPreAmp,ap->fPreAmp);
	setParameter(kSaturate,ap->fSaturate);
}

void H2Oeffect::setParameter (VstInt32 index, float value)
{
	H2Oprogram *ap = &programs[curProgram];

	switch(index)
	{
		case kAttack : fAttack = ap->fAttack = value; comp->setAttack(value); break;
		case kRelease : fRelease = ap->fRelease = value; comp->setRelease(value); break;
		case kAmount : fAmount = ap->fAmount = value; comp->setAmount(value); break;
		case kPostAmp : fPostAmp = ap->fPostAmp = value; comp->setPostamp(value); break;
		case kPreAmp : fPreAmp = ap->fPreAmp = value; comp->setPreamp(value); break;
		case kSaturate : fSaturate = ap->fSaturate = value; comp->setSaturate(value); break;
	}

	if(editor)
		editor->postUpdate();
}


float H2Oeffect::getParameter (VstInt32 index)
{
	float v = 0;

	switch(index)
	{
		case kAttack : v = fAttack; break;
		case kRelease : v = fRelease; break;
		case kAmount : v = fAmount; break;
		case kPostAmp : v = fPostAmp; break;
		case kPreAmp : v = fPreAmp; break;
		case kSaturate : v = fSaturate; break;
	}

	return v;
}

void H2Oeffect::getParameterName (VstInt32 index, char *label)
{
	switch(index)
	{
		case kAttack : strcpy(label,"Attack"); break;
		case kRelease : strcpy(label,"Release"); break;
		case kAmount : strcpy(label,"Amount"); break;
		case kPostAmp : strcpy(label,"Output gain"); break;
		case kPreAmp : strcpy(label,"Input gain"); break;
		case kSaturate : strcpy(label,"Saturate"); break;
	}
}


void float2string2(float value, char *text)
{
	VstInt32 c = 0, neg = 0;
	char string[32];
	char *s;
	double v, integ, i10, mantissa, m10, ten = 10.;

	v = (double)value;
	if(v < 0)
	{
		neg = 1;
		value = -value;
		v = -v;
		c++;
		if(v > 9999999.)
		{
			strcpy(string, " Huge!  ");
			return;
		}
	}
	else if(v > 99999999.)
	{
		strcpy(string, " Huge!  ");
		return;
	}

	s = string + 31;
	*s-- = 0;
	*s-- = '.';
	c++;

	integ = floor(v);
	i10 = fmod(integ, ten);
	*s-- = (VstInt32)i10 + '0';
	integ /= ten;
	c++;
	while(integ >= 1. && c < 8)
	{
		i10 = fmod(integ, ten);
		*s-- = (VstInt32)i10 + '0';
		integ /= ten;
		c++;
	}
	if(neg)
		*s-- = '-';
	strcpy(text, s + 1);
	if(c >= 8)
		return;

	s = string + 31;
	*s-- = 0;
	mantissa = fmod(v, 1.);
	mantissa *= pow(ten, (double)(8 - c));
	while(c < 5)
	{
		if(mantissa <= 0)
			*s-- = '0';
		else
		{
			m10 = fmod(mantissa, ten);
			*s-- = (VstInt32)m10 + '0';
			mantissa /= 10.;
		}
		c++;
	}
	strcat(text, s + 1);
}

void dB2string2(float value, char *text)
{
	if(value <= 0.f)
	{
/*#if MAC
		strcpy(text, "   -�   ");
#else
		strcpy(text, "  -oo   ");
#endif*/
		text[0] = '-';
		text[1] = (char) __INF;
		text[2] = 0;
	}
	else
		float2string2((float)(20. * log10(value)), text);
}

void H2Oeffect::getParameterDisplay (VstInt32 index, char *text)
{
	switch(index)
	{
		case kAttack : float2string2(comp->getAttack(),text); break;
		case kRelease : float2string2(comp->getRelease(),text); break;
		case kAmount : float2string2(comp->getAmount(),text); break;
		case kPostAmp : dB2string2(comp->getPostamp(),text); break;
		case kPreAmp : dB2string2(comp->getPreamp(),text); break;
		case kSaturate :
			if(comp->getSaturate())
				strcpy(text,"on");
			else
				strcpy(text,"off");
			break;
	}
}

void H2Oeffect::getParameterLabel (VstInt32 index, char *label)
{
	switch(index)
	{
		case kAttack : strcpy(label,"ms"); break;
		case kRelease : strcpy(label,"ms"); break;
		case kAmount : strcpy(label,""); break;
		case kPostAmp : strcpy(label,"dB"); break;
		case kPreAmp : strcpy(label,"dB"); break;
		case kSaturate : strcpy(label,""); break;
	}
}

void H2Oeffect::suspend()
{
	comp->Suspend();
}

void H2Oeffect::resume ()
{
}

bool H2Oeffect::getOutputProperties (VstInt32 index, VstPinProperties* properties)
{
	if (index < kNumOutputs)
	{
		if(index == 0)
		{
			sprintf (properties->label, "H2O Left");
		}
		else
		{
			if(index == 1)
				sprintf (properties->label, "H2O Right");
			else
				sprintf (properties->label, "H2O %d",index+1);
		}

		properties->flags = kVstPinIsActive;

		if(index < 2)
			properties->flags |= kVstPinIsStereo; // make channel 1+2 stereo

		return true;
	}
	return false;
};

void H2Oeffect::setProgramName (char *name)
{
	strcpy (programs[curProgram].name, name);
}

//------------------------------------------------------------------------
void H2Oeffect::getProgramName (char *name)
{
	if (!strcmp (programs[curProgram].name, "Init"))
		sprintf (name, "%s %d", programs[curProgram].name, curProgram + 1);
	else
		strcpy (name, programs[curProgram].name);
}

void H2Oeffect::setSampleRate(float sampleRate)
{
	AudioEffect::setSampleRate (sampleRate);

	comp->setSamplerate(sampleRate);
}


bool H2Oeffect::getEffectName (char* name)
{
	strcpy (name, "H2O");
	return true;
}

bool H2Oeffect::getVendorString (char* text)
{
	strcpy (text, "Bram @ Smartelectronix");
	return true;
}

bool H2Oeffect::getProductString (char* text)
{
	strcpy (text, "H2O Compressor");
	return true;
}

VstInt32 H2Oeffect::canDo (char* text)
{
	if (!strcmp(text, "receiveVstTimeInfo"))  return 0;
	if (!strcmp(text, "receiveVstMidiEvent")) return 0;
	if (!strcmp(text, "plugAsChannelInsert")) return 1;
	if (!strcmp(text, "plugAsSend")) return 1;
	if (!strcmp(text, "1in2out")) return 1;
	if (!strcmp(text, "2in2out")) return 1;

	return -1;	// explicitly can't do; 0 => don't know
}
