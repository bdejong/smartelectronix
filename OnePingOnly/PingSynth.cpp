#include "PingSynth.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <limits>

#include "Delay.h"

inline bool IS_DENORMAL(const float flt)
{
	return flt != 0 && std::fabsf( flt ) < std::numeric_limits<float>::min();
}

PingSynth::PingSynth(audioMasterCallback audioMaster)
	:AudioEffectX(audioMaster,kNumPrograms,kNumParams)
{
	programs = new PingSynthProgram[kNumPrograms];

	DelayL = new CDelay((int)(getSampleRate()*3));
	DelayR = new CDelay((int)(getSampleRate()*3));

	for(int index=0;index<nPing;index++)
	{
		beta[index] = 0.99f;
		gamma[index] = -0.99f;
		alpha[index] = 1.9701f;
		d1[index]=d2[index]=0.f;
	}

	if (programs)
		setProgram (0);

	if (audioMaster)
	{
		setNumInputs(0);				// 1 input. so this inst. can be seen in Logic...
		setNumOutputs(kNumOutputs);	// 2 outputs
		canProcessReplacing(true);
		isSynth();
		setUniqueID('Ping');
	}

	for(int k=0;k<200;k++)
		Notes[k].Note = -1; //no notes yet!
	nNotes=0;

	resume();
}

PingSynth::~PingSynth()
{
	if(programs)
		delete [] programs;

	delete DelayL;
	delete DelayR;
}

VstInt32 PingSynth::processEvents (VstEvents* ev)
{
	nNotes=0; //no notes yet

	NoteData *pNotes = Notes;
	long velocity, note;

	for (long i = 0; i < ev->numEvents; i++)
	{
		if ((ev->events[i])->type != kVstMidiType)
			continue;

		VstMidiEvent* event = (VstMidiEvent*)ev->events[i];
		char* midiData = event->midiData;
		long status = midiData[0] & 0xf0;		// ignoring channel

		if(status == 0x90)	// note on only
		{
			velocity = midiData[2] & 0x7f;	//velocity
			note = midiData[1] & 0x7f;	//note number
			note -= lownote;

			if(velocity && (note>=0) && (note<nPing))
			{
				pNotes->Note = note; //this is the osc number
				pNotes->Velocity = velocity/128.f;
				pNotes++->Time = event->deltaFrames;
				nNotes++;  //one more note!
			}
		}
	}
	return 1;
}

void PingSynth::process(float **inputs, float **outputs, VstInt32 sampleFrames)
{
	NoteData *pNotes = Notes;

	float *out1 = outputs[0];
    float *out2 = outputs[1];

	float out,o,oL,oR,noise;

	long n=0;
	int index;
	bool MoreThanOne = false;

	if(!nNotes) //there might be NO notes in the buffer!
		goto NoMoreNotes;

	while(n<sampleFrames)
	{
		noise = rand()/16384.f - 1.f; //we need noise in BOTH cases...

		if(n == pNotes->Time)
		{
			//the note...
			index = pNotes->Note;
			in[index] = g[index]*pNotes++->Velocity;
			nNotes--;

			while(pNotes->Time == n && nNotes) //checking for other notes on this timeframe
			{
				index = pNotes->Note;
				in[index] = g[index]*pNotes++->Velocity;
				nNotes--;
				MoreThanOne = true; //more than one note at this timeframe!
			}

			oL = oR = o = 0;

			//the main loop
			//I tried doing an "optimised" '*' loop but it actualy slowed down the thing!

			for(int i=0;i<nPing;i++)
			{
				out = beta[i]*(in[i] - out_2[i]) + alpha[i]*(in_1[i]-out_1[i]) + in_2[i];
				out_2[i] = out_1[i];
				out_1[i] = out;
				in_2[i] = in_1[i];
				in_1[i] = in[i];

				o = (out-in[i])*(1 + NoiseAmount[i]*noise);
				o *= d2[i]/(1+d1[i]*o*o);
				oL += bal[i]*o;
				oR += baltmp[i]*o;
			}

			*out1++ += DelayL->getVal(oL)*fMaster;
			*out2++ += DelayR->getVal(oR)*fMaster;

			if(MoreThanOne)
			{
				for(int i=0;i<nPing;i++)
					in[i] = 0.f; //kill input to all oscs, 'cos we don't know which ones sounded!
				MoreThanOne = false;
			}
			else
				in[index] = 0.f;

			n++; //we did one more sample

			//check to see if there are any more notes
			if(!nNotes)
				goto NoMoreNotes;
		}
		else
		{

			oL=oR=o=0;

			for(int i=0;i<nPing;i++)
			{
				out = -beta[i]*out_2[i] + alpha[i]*(in_1[i]-out_1[i]) + in_2[i];
				out_2[i]=out_1[i];
				out_1[i]=out;
				in_2[i]=in_1[i];
				in_1[i]=0;

				o = out*(1 + NoiseAmount[i]*noise);
				o *= d2[i]/(1+d1[i]*o*o);
				oL += bal[i]*o;
				oR += baltmp[i]*o;
			}

			*out1++ += DelayL->getVal(oL)*fMaster;
			*out2++ += DelayR->getVal(oR)*fMaster;

			n++; //we did one more sample
		}
	}

	goto end;

//this is the important/time-critical bit...

NoMoreNotes:

	sampleFrames -= n; //we've done n samples

	if(sampleFrames>2) //more than two samples to go...
	{
		noise = rand()/16384.f - 1.f;

		oL=oR=o=0;

		//in=0
		for(int i=0;i<nPing;i++)
		{
			out = -beta[i]*out_2[i] + alpha[i]*(in_1[i]-out_1[i]) + in_2[i];
			out_2[i] = out_1[i];
			out_1[i] = out;
			in_2[i] = in_1[i];
			in_1[i] = 0; //we HAVE to clear these out!, they'll be used in the next round!

			o = out*(1 + NoiseAmount[i]*noise);
			o *= d2[i]/(1+d1[i]*o*o);
			oL += bal[i]*o;
			oR += baltmp[i]*o;
		}

		*out1++ += DelayL->getVal(oL)*fMaster;
		*out2++ += DelayR->getVal(oR)*fMaster;

		noise = rand()/16384.f - 1.f;

		oL=oR=o=0;

		//in=in_1=0;
		for(int i=0;i<nPing;i++)
		{
			out = -beta[i]*out_2[i] - alpha[i]*out_1[i] + in_2[i];
			out_2[i] = out_1[i];
			out_1[i] = out;
			in_2[i] = 0; //we HAVE to clear these out!, they'll be used in the next round!

			o = out*(1 + NoiseAmount[i]*noise);
			o *= d2[i]/(1+d1[i]*o*o);
			oL += bal[i]*o;
			oR += baltmp[i]*o;
		}

		*out1++ += DelayL->getVal(oL)*fMaster;
		*out2++ += DelayR->getVal(oR)*fMaster;

		//we've done two more samples
		sampleFrames -= 2;

		//in=in_1=in_2=0;
		while(sampleFrames--) //this loop will be called the most (I *think*)
		{
			noise = rand()/16384.f - 1.f;

			oL=oR=o=0;

			for(int i=0;i<nPing;i++)
			{
				out = -beta[i]*out_2[i] - alpha[i]*out_1[i];
				out_2[i] = out_1[i];
				out_1[i] = out;

				o = out*(1 + NoiseAmount[i]*noise);
				o *= d2[i]/(1+d1[i]*o*o);
				oL += bal[i]*o;
				oR += (1- bal[i])*o;
			}

			*out1++ += DelayL->getVal(oL)*fMaster;
			*out2++ += DelayR->getVal(oR)*fMaster;
		}
	}
	else
	{
		while(sampleFrames--)
		{
			noise = rand()/16384.f - 1.f;

			oL=oR=o=0;

			for(int i=0;i<nPing;i++)
			{
				out = -beta[i]*out_2[i] + alpha[i]*(in_1[i]-out_1[i]) + in_2[i];
				out_2[i] = out_1[i];
				out_1[i] = out;
				in_2[i] = in_1[i];
				in_1[i] = 0;

				o = out*(1 + NoiseAmount[i]*noise);
				o *= d2[i]/(1+d1[i]*o*o);
				oL += bal[i]*o;
				oR += baltmp[i]*o;
			}

			*out1++ += DelayL->getVal(oL)*fMaster;
			*out2++ += DelayR->getVal(oR)*fMaster;
		}
	}

end:

	Notes->Note = -1;

	for(int i=0;i<nPing;i++) //avoid the "de-normalisation trap"
	{
		if(IS_DENORMAL(out_1[i]))
			out_1[i] = 0.f;

		if(IS_DENORMAL(out_2[i]))
			out_2[i] = 0.f;
	}
}

void PingSynth::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames)
{
	NoteData *pNotes = Notes;

	float *out1 = outputs[0];
    float *out2 = outputs[1];

	float out,o,oL,oR,noise;

	long n=0;
	int index;
	bool MoreThanOne = false;

	if(!nNotes) //there might be NO notes in the buffer!
		goto NoMoreNotes;

	while(n<sampleFrames)
	{
		noise = rand()/16384.f - 1.f; //we need noise in BOTH cases...

		if(n == pNotes->Time)
		{
			//the note...
			index = pNotes->Note;
			in[index] = g[index]*pNotes++->Velocity;
			nNotes--;

			while(pNotes->Time == n && nNotes) //checking for other notes on this timeframe
			{
				index = pNotes->Note;
				in[index] = g[index]*pNotes++->Velocity;
				nNotes--;
				MoreThanOne = true; //more than one note at this timeframe!
			}

			oL = oR = o = 0;

			//the main loop
			//I tried doing an "optimised" '*' loop but it actualy slowed down the thing!

			for(int i=0;i<nPing;i++)
			{
				out = beta[i]*(in[i] - out_2[i]) + alpha[i]*(in_1[i]-out_1[i]) + in_2[i];
				out_2[i] = out_1[i];
				out_1[i] = out;
				in_2[i] = in_1[i];
				in_1[i] = in[i];

				o = (out-in[i])*(1 + NoiseAmount[i]*noise);
				o *= d2[i]/(1+d1[i]*o*o);
				oL += bal[i]*o;
				oR += baltmp[i]*o;
			}

			*out1++ = DelayL->getVal(oL)*fMaster;
			*out2++ = DelayR->getVal(oR)*fMaster;

			if(MoreThanOne)
			{
				for(int i=0;i<nPing;i++)
					in[i] = 0.f; //kill input to all oscs, 'cos we don't know which ones sounded!
				MoreThanOne = false;
			}
			else
				in[index] = 0.f;

			n++; //we did one more sample

			//check to see if there are any more notes
			if(!nNotes)
				goto NoMoreNotes;
		}
		else
		{

			oL=oR=o=0;

			for(int i=0;i<nPing;i++)
			{
				out = -beta[i]*out_2[i] + alpha[i]*(in_1[i]-out_1[i]) + in_2[i];
				out_2[i]=out_1[i];
				out_1[i]=out;
				in_2[i]=in_1[i];
				in_1[i]=0;

				o = out*(1 + NoiseAmount[i]*noise);
				o *= d2[i]/(1+d1[i]*o*o);
				oL += bal[i]*o;
				oR += baltmp[i]*o;
			}

			*out1++ = DelayL->getVal(oL)*fMaster;
			*out2++ = DelayR->getVal(oR)*fMaster;

			n++; //we did one more sample
		}
	}

	goto end;

//this is the important/time-critical bit...

NoMoreNotes:

	sampleFrames -= n; //we've done n samples

	if(sampleFrames>2) //more than two samples to go...
	{
		noise = rand()/16384.f - 1.f;

		oL=oR=o=0;

		//in=0
		for(int i=0;i<nPing;i++)
		{
			out = -beta[i]*out_2[i] + alpha[i]*(in_1[i]-out_1[i]) + in_2[i];
			out_2[i] = out_1[i];
			out_1[i] = out;
			in_2[i] = in_1[i];
			in_1[i] = 0; //we HAVE to clear these out!, they'll be used in the next round!

			o = out*(1 + NoiseAmount[i]*noise);
			o *= d2[i]/(1+d1[i]*o*o);
			oL += bal[i]*o;
			oR += baltmp[i]*o;
		}

		*out1++ = DelayL->getVal(oL)*fMaster;
		*out2++ = DelayR->getVal(oR)*fMaster;

		noise = rand()/16384.f - 1.f;

		oL=oR=o=0;

		//in=in_1=0;
		for(int i=0;i<nPing;i++)
		{
			out = -beta[i]*out_2[i] - alpha[i]*out_1[i] + in_2[i];
			out_2[i] = out_1[i];
			out_1[i] = out;
			in_2[i] = 0; //we HAVE to clear these out!, they'll be used in the next round!

			o = out*(1 + NoiseAmount[i]*noise);
			o *= d2[i]/(1+d1[i]*o*o);
			oL += bal[i]*o;
			oR += baltmp[i]*o;
		}

		*out1++ = DelayL->getVal(oL)*fMaster;
		*out2++ = DelayR->getVal(oR)*fMaster;

		//we've done two more samples
		sampleFrames -= 2;

		//in=in_1=in_2=0;
		while(sampleFrames--) //this loop will be called the most (I *think*)
		{
			noise = rand()/16384.f - 1.f;

			oL=oR=o=0;

			for(int i=0;i<nPing;i++)
			{
				out = -beta[i]*out_2[i] - alpha[i]*out_1[i];
				out_2[i] = out_1[i];
				out_1[i] = out;

				o = out*(1 + NoiseAmount[i]*noise);
				o *= d2[i]/(1+d1[i]*o*o);
				oL += bal[i]*o;
				oR += (1- bal[i])*o;
			}

			*out1++ = DelayL->getVal(oL)*fMaster;
			*out2++ = DelayR->getVal(oR)*fMaster;
		}
	}
	else
	{
		while(sampleFrames--)
		{
			noise = rand()/16384.f - 1.f;

			oL=oR=o=0;

			for(int i=0;i<nPing;i++)
			{
				out = -beta[i]*out_2[i] + alpha[i]*(in_1[i]-out_1[i]) + in_2[i];
				out_2[i] = out_1[i];
				out_1[i] = out;
				in_2[i] = in_1[i];
				in_1[i] = 0;

				o = out*(1 + NoiseAmount[i]*noise);
				o *= d2[i]/(1+d1[i]*o*o);
				oL += bal[i]*o;
				oR += baltmp[i]*o;
			}

			*out1++ = DelayL->getVal(oL)*fMaster;
			*out2++ = DelayR->getVal(oR)*fMaster;
		}
	}

end:

	Notes->Note = -1;

	for(int i=0;i<nPing;i++) //avoid the "de-normalisation trap"
	{
		if(IS_DENORMAL(out_1[i]))
			out_1[i] = 0.f;

		if(IS_DENORMAL(out_2[i]))
			out_2[i] = 0.f;
	}
}


void PingSynth::suspend()
{
	for(int i=0;i<nPing;i++)
		in[i]=in_1[i]=in_2[i]=out_1[i]=out_2[i]=0;

	DelayL->KillBuffer();
	DelayR->KillBuffer();
}

void PingSynth::resume ()
{
	for(int i=0;i<nPing;i++)
		in[i]=in_1[i]=in_2[i]=out_1[i]=out_2[i]=0;
}

void PingSynth::SetFreq(int index, float freq)
{
	if(freq<0.001f)
		freq = 0.001f;

	gamma[index] = -(float)cos((2.0*3.1415926535*2000*freq)/getSampleRate());
	alpha[index] = gamma[index]*(1+beta[index]);
}

void PingSynth::SetAmp(int index, float amp)
{
	this->amp[index] = amp;
	g[index] = this->amp[index]*0.4f/(1-beta[index]); //epirical hack
}

void PingSynth::SetDuration(int index, float duration)
{
	duration *= 3.f*(getSampleRate()/44100.f);
	beta[index] = (float)((duration*8000.0 - 1.0)/(duration*8000.0 + 1.0));
	alpha[index] = gamma[index]*(1+beta[index]);
	g[index] = amp[index]*0.4f/(1-beta[index]);
}

void PingSynth::SetBalance(int index, float balance)
{
	bal[index] = balance;
	baltmp[index] = 1 - balance;
}

void PingSynth::SetNoise(int index, float n)
{
	NoiseAmount[index] = n;
}

void PingSynth::SetDistortion(int index, float dist)
{
	dist *= 10.f;

	d1[index] = dist;

	if(dist>1)
		d2[index] = (1.f + dist)*(1 - 0.8f*(dist-1.f)/19.f); //empirical hack
	else
		d2[index] = 1 + dist;
}

void PingSynth::SetDelay(float delay)
{
	int d = (int)(delay*this->getSampleRate()*3.f);
	DelayL->setDelay(d);
	DelayR->setDelay(d);
}

void PingSynth::SetFeed(float feed)
{
	DelayL->setFeed(feed);
	DelayR->setFeed(feed);
}

void PingSynth::setProgramName (char *name)
{
	strcpy (programs[curProgram].name, name);
}

//------------------------------------------------------------------------
void PingSynth::getProgramName (char *name)
{
	if (!strcmp (programs[curProgram].name, "Init"))
		sprintf (name, "%s %d", programs[curProgram].name, curProgram + 1);
	else
		strcpy (name, programs[curProgram].name);
}

void PingSynth::setSampleRate(float sampleRate)
{
	AudioEffect::setSampleRate (sampleRate);

	DelayL->setMaxDelay((int)(sampleRate*3));
	DelayR->setMaxDelay((int)(sampleRate*3));
	DelayL->setDelay((int)(sampleRate*3*fDelay));
	DelayR->setDelay((int)(sampleRate*3*fDelay));

	for(int i=0;i<nPing;i++)
	{
		this->SetFreq(i,fFreq[i]);
		this->SetDuration(i,fDuration[i]);
		in[i]=in_1[i]=in_2[i]=out_1[i]=out_2[i]=0;
	}
}

bool PingSynth::getEffectName (char* name)
{
	strcpy (name, "One Ping Only");
	return true;
}

bool PingSynth::getVendorString (char* text)
{
	strcpy (text, "Bram @ Smartelectronix");
	return true;
}

bool PingSynth::getProductString (char* text)
{
	strcpy (text, "One Ping Only");
	return true;
}

VstInt32 PingSynth::canDo (char* text)
{
	if (!strcmp (text, "receiveVstEvents"))
		return 1;
	if (!strcmp (text, "receiveVstMidiEvent"))
		return 1;
	return -1;	// explicitly can't do; 0 => don't know
}

bool PingSynth::getInputProperties(VstInt32 index, VstPinProperties* properties)
{
	return false;
}

bool PingSynth::getOutputProperties(VstInt32 index, VstPinProperties* properties)
{
	if ( (index >= 0) && (index < 2) )
	{
		sprintf (properties->label, "OnePingOnly output %i", index+1);
		sprintf (properties->shortLabel, "Out %i", index+1);
		properties->flags = (kVstPinIsStereo | kVstPinIsActive);
		return true;
	}
	return false;
}
