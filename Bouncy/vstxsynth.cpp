#ifndef __VstXSynth__
#include "vstxsynth.h"
#endif

#if WIN32
	#include "Registry.h"
	#include <string>
#endif

VstXSynth::VstXSynth (audioMasterCallback audioMaster) : AudioEffectX (audioMaster, 0, kNumParams)
{
	setNumInputs (2);
	setNumOutputs (2);
	canProcessReplacing();
	//canMono();
	//hasVu(false);
	//hasClip(false);
	setUniqueID ('BNCY');

	delayL = new Bouncy(getSampleRate());
	delayR = new Bouncy(getSampleRate());

	save[kMaxDelay] = 0.58f;
	save[kDelayShape] = 0.42f;
	save[kAmpShape] = 0.08f;
	save[kRandAmp] = 0.f;

#if WIN32
	pf = 0;

	Registry reg("Smartelectronix\\Bouncy");

	if(reg.getLong("Debug") == 1)
	{
		pf = fopen("c:\\bouncy.log","wb");

		fprintf(pf,"Application-path : %s\n",__argv[0]);

		if( Multitap::SSEDetect() )
			fprintf(pf,"This processor supports SSE\n");
		else
			fprintf(pf,"This processor does not support SSE\n");

		if(reg.getLong("ForceSSE") == 1)
		{
			fprintf(pf,"Forcing SSE processing\n");
			delayL->forceSSE(true);
			delayR->forceSSE(true);
		}

		if(reg.getLong("ForceFPU") == 1)
		{
			fprintf(pf,"Forcing FPU processing\n");
			delayL->forceSSE(false);
			delayR->forceSSE(false);
		}
	}
#endif

	setParam();

	resume();
}

VstXSynth::~VstXSynth ()
{
	delete delayL;
	delete delayR;

#if WIN32
	if(pf != 0)
		fclose(pf);
#endif
}

void VstXSynth::getParameterLabel (long index, char *label)
{
	switch(index)
	{
		case kMaxDelay : strcpy(label," beats"); break;
		default : strcpy(label,""); break;
	}
}

void VstXSynth::getParameterDisplay (long index, char *text)
{
	switch(index)
	{
		case kMaxDelay : float2string(save[index]*NUM_BEATS,text, 10); break;
		case kDelayShape : float2string(save[index]*2.f - 1.f,text, 10); break;
		case kAmpShape : float2string(save[index]*2.f - 1.f,text, 10); break;
		case kRenewRand :
			if(save[kRenewRand] > 0.5f)
				strcpy(text,"on");
			else
				strcpy(text,"off");
			break;
		default : float2string(save[index],text, 10); break;
	}
}

void VstXSynth::getParameterName (long index, char *label)
{
	switch(index)
	{
		case kMaxDelay : strcpy(label,"max delay"); break;
		case kDelayShape : strcpy(label,"delay shape"); break;
		case kAmpShape : strcpy(label,"amp shape"); break;
		case kRandAmp : strcpy(label,"rand amp"); break;
		case kRenewRand : strcpy(label,"renew rand"); break;
	}
}

void VstXSynth::setParameter (long index, float value)
{
	if(index < kNumParams)
	{
		save[index] = value;

		if(index == kMaxDelay)
		{
			if ((save[kMaxDelay] * 60.f * NUM_BEATS) / (BPM * 5.f) > 1.f)
				save[kMaxDelay] = (BPM*5.f)/(60.f*NUM_BEATS);

			float beatzf = save[kMaxDelay] * NUM_BEATS;
			long  beatzi = (long)beatzf;
			float diff = fabsf(beatzf - (float)beatzi);

			if(diff < 0.1f)
				save[kMaxDelay] = (float) beatzi / (NUM_BEATS);
		}

		if(index == kRenewRand && save[kRenewRand] > 0.5f)
		{
			delayL->fillRand();
			delayR->fillRand();
		}

		//if (editor)
		//	((AEffGUIEditor *)editor)->setParameter(index, value);

		setParam();
	}
}

float VstXSynth::getParameter (long index)
{
	if(index < kNumParams)
		return save[index];
	else
		return 0.f;
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
	strcpy (name, "Bouncy");
	return true;
}

bool VstXSynth::getVendorString (char* text)
{
	strcpy (text, "Bram @ Smartelectronix");
	return true;
}

bool VstXSynth::getProductString (char* text)
{
	strcpy (text, "Bouncing ball delay");
	return true;
}

VstInt32 VstXSynth::canDo (char* text)
{
	if (!strcmp(text, "receiveVstTimeInfo"))  return 1;
	if (!strcmp(text, "receiveVstMidiEvent")) return 1;
	if (!strcmp(text, "plugAsChannelInsert")) return 1;
	if (!strcmp(text, "plugAsSend")) return 1;
	if (!strcmp(text, "1in2out")) return 1;
	if (!strcmp(text, "2in2out")) return 1;

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

VstInt32 VstXSynth::processEvents (VstEvents* ev)
{
	for (long i = 0; i < ev->numEvents; i++)
	{
		if ((ev->events[i])->type != kVstMidiType)
			continue;

		VstMidiEvent* event = (VstMidiEvent*)ev->events[i];
		unsigned char* midiData = (unsigned char *)event->midiData;

		unsigned char status = midiData[0] & 0xF0;
		unsigned char cc = midiData[1] & 0x7F;	// CC number

		if(status == 0xB0) //CC but not all-notes-off
		{
			float value = (float)(midiData[2] & 0x7F) / 127.0f;

			switch(cc)
			{
				case 73 : setParameter(kMaxDelay, value); break;
				case 74 : setParameter(kDelayShape, value); break;
				case 75 : setParameter(kAmpShape, value); break;
				case 76 : setParameter(kRandAmp, value); break;
				case 77 : setParameter(kRenewRand, value); break;
			}
		}
	}
	return 1;
}
