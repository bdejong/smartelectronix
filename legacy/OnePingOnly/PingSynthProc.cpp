#include "PingSynth.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "math.h"
#include <sstream>

#include "defs.h"

PingSynthProgram::PingSynthProgram()
{
	static bool FirstProgram = true;

	if(FirstProgram)
	{
		FirstProgram=false;
		srand((unsigned)time(0));
	}


	for(int i=0;i<nPing;i++)
	{
		this->fAmp[i] = 1.f;
		this->fDuration[i] = 0.25f*rand()/32768.f;
		this->fFreq[i] = rand()/32768.f;
		this->fBal[i] = rand()/32768.f;
		this->fDist[i] = 0.5f*rand()/32768.f;
		this->fNoise[i] = 0.f;
	}

	this->fDelay = 0.25f;
	this->fFeed = 0.6f;
	this->fMaster = 1.f;

	strcpy (name, "Init");
}

PingSynthProgram::~PingSynthProgram()
{
}

void PingSynth::setProgram (VstInt32 program)
{
	PingSynthProgram *ap = &programs[program];
	curProgram = program;

	for(int i=0;i<nPing;i++)
	{
		setParameter(i*nPar+0,ap->fFreq[i]);
		setParameter(i*nPar+1,ap->fDuration[i]);
		setParameter(i*nPar+2,ap->fAmp[i]);
		setParameter(i*nPar+3,ap->fBal[i]);
		setParameter(i*nPar+4,ap->fNoise[i]);
		setParameter(i*nPar+5,ap->fDist[i]);
	}

	setParameter (kDelay,ap->fDelay);
	setParameter (kFeed,ap->fFeed);
	setParameter (kMaster,ap->fMaster);
}

void PingSynth::setParameter (VstInt32 index, float value)
{
	PingSynthProgram *ap = &programs[curProgram];

	if(index<nPar*nPing)
	{
		int ParamIndex = index % nPar;
		int PingIndex  = index / nPar;

		switch(ParamIndex)
		{
			case kFreq:		fFreq[PingIndex]		= ap->fFreq[PingIndex]		= value; SetFreq(PingIndex,fFreq[PingIndex]); break;
			case kAmp:		fAmp [PingIndex]		= ap->fAmp [PingIndex]		= value; SetAmp (PingIndex,fAmp [PingIndex]); break;
			case kDuration:	fDuration[PingIndex]	= ap->fDuration[PingIndex]	= value; SetDuration(PingIndex,fDuration[PingIndex]); break;
			case kBal :		fBal[PingIndex]			= ap->fBal[PingIndex]		= value; SetBalance(PingIndex,fBal[PingIndex]); break;
			case kDist :	fDist[PingIndex]		= ap->fDist[PingIndex]		= value; SetDistortion(PingIndex,fDist[PingIndex]);break;
			case kNoise :	fNoise[PingIndex]		= ap->fNoise[PingIndex]		= value; SetNoise(PingIndex,fNoise[PingIndex]); break;
		}
	}
	else
	{
		switch(index)
		{
			case kFeed : fFeed = ap->fFeed = value; SetFeed(value); break;
			case kDelay : fDelay = ap->fDelay = value; SetDelay(value); break;
			case kMaster : fMaster = ap->fMaster = value; break;
		}
	}
}


float PingSynth::getParameter (VstInt32 index)
{
	float v = 0;

	if(index<nPar*nPing)
	{
		int ParamIndex = index % nPar;
		int PingIndex  = index / nPar;


		switch(ParamIndex)
		{
			case kFreq:		v=fFreq[PingIndex];		break;
			case kAmp:		v=fAmp[PingIndex];		break;
			case kDuration:	v=fDuration[PingIndex];	break;
			case kBal :		v=fBal[PingIndex];		break;
			case kDist :	v=fDist[PingIndex];		break;
			case kNoise :	v=fNoise[PingIndex];	break;
			case kDelay :	v=fNoise[PingIndex];	break;
		}
	}
	else
	{
		switch(index)
		{
			case kDelay: v = fDelay; break;
			case kFeed:  v = fFeed; break;
			case kMaster:  v = fMaster; break;
		}
	}

	return v;
}

void CreateString(char *label, const char *str, int index)
{
	std::stringstream ss;
	ss << str;
	ss << (index+1);
	strcpy(label,ss.str().c_str());
}

void PingSynth::getParameterName (VstInt32 index, char *label)
{
	if(index<nPar*nPing)
	{
		int ParamIndex = index % nPar;
		int PingIndex  = index / nPar;

		switch(ParamIndex)
		{
			case kFreq :		CreateString(label,"Freq",PingIndex);break;
			case kAmp :			CreateString(label,"Amp",PingIndex);break;
			case kDuration :	CreateString(label,"Dur",PingIndex);break;
			case kBal :			CreateString(label,"Bal",PingIndex);break;
			case kDist :		CreateString(label,"Dist",PingIndex);break;
			case kNoise :		CreateString(label,"Noise",PingIndex);break;
		}
	}
	else
	{
		switch(index)
		{
			case kDelay: strcpy(label,"Delay"); break;
			case kFeed:  strcpy(label,"Feed"); break;
			case kMaster:  strcpy(label,"MasterVolume"); break;
		}
	}
}

void long2string(const long number, char* str)
{
	std::stringstream ss;
	ss << number;
	strcpy(str, ss.str().c_str());
}

void PingSynth::getParameterDisplay (VstInt32 index, char *text)
{
	if(index<nPar*nPing)
	{
		int ParamIndex = index % nPar;
		int PingIndex  = index / nPar;

		switch(ParamIndex)
		{
			case kFreq :		float2string(2000.f*fFreq[PingIndex],text, 25);break;
			case kAmp :			float2string(fAmp[PingIndex],text, 25);break;
			case kDuration :	float2string(3.f*fDuration[PingIndex],text, 25);break;
			case kBal :			float2string(fBal[PingIndex],text, 25);break;
			case kDist :		float2string(fDist[PingIndex],text, 25);break;
			case kNoise :		float2string(fNoise[PingIndex],text, 25);break;
		}
	}
	else
	{
		switch(index)
		{
			case kDelay: float2string(fDelay*3.f,text, 25); break;
			case kFeed:  long2string((long)(fFeed*100.f),text); break;
			case kMaster:  dB2string(fMaster,text,25); break;
		}
	}
}

void PingSynth::getParameterLabel (VstInt32 index, char *label)
{
	if(index<nPar*nPing)
	{
		int ParamIndex = index % nPar;

		switch(ParamIndex)
		{
			case kFreq:		strcpy(label,"Hz");		break;
			case kAmp:		strcpy(label,"Amount"); break;
			case kDuration:	strcpy(label,"Sec");	break;
			case kBal :		strcpy(label,"Left");	break;
			case kDist :	strcpy(label,"Amount"); break;
			case kNoise :	strcpy(label,"Amount"); break;
		}
	}
	else
	{
		switch(index)
		{
			case kDelay: strcpy(label,"sec"); break;
			case kFeed:  strcpy(label,"%"); break;
			case kMaster:  strcpy(label,"dB"); break;
		}
	}
}
