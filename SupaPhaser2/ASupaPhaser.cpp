// ASupaPhaser.cpp: implementation of the ASupaPhaser class.
//
//////////////////////////////////////////////////////////////////////

#include "ASupaPhaser.h"
#include "ASupaEditor.h"
#include "aeffectx.h"
#include <stdio.h>
#include <stdlib.h>
#include "math.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void InitNoise(float *Noise, int bufSize)
{
	for (int i = 0; i < bufSize; i++)
		Noise[i] = (float)(((double)rand())/(RAND_MAX*1048576.0));
}

ASupaPhaser::ASupaPhaser(audioMasterCallback audioMaster)
				:AudioEffectX(audioMaster, kNumPrograms, kNumParams)
{
	p11 =NULL;
	p12 =NULL;
	p11=y1;
	p12=y2;

	for(int i=0;i<MaxnStages;i++)
		y2[i]=in_12[i]=y1[i]=in_11[i]=0.f;

	setNumInputs(2);
	setNumOutputs(2);
	canMono(true);
	canProcessReplacing(true);

	prevOutR = prevOutL = 0.f;

	setUniqueID ('ASuP');
	
	Noise = NULL;
	Noise = new float[this->getBlockSize()];
	InitNoise(Noise,this->getBlockSize());

	Osc1.setrate(this->getSampleRate());
	Osc2.setrate(this->getSampleRate());

	setPresets();

	setProgram(0);

	suspend();

	editor = new ASupaEditor(this);
}

ASupaPhaser::~ASupaPhaser()
{
#ifdef _DEBUG
	{
		FILE *pf = fopen("c:\\presets.cpp","wt");

		fprintf(pf,"#include \"ASupaPhaser.h\"\n\n");
		
		fprintf(pf,"void ASupaPhaser::setPresets()\n{\n");

		for(long i=0;i<kNumPrograms;i++)
		{
			char name[] = ""; //add space after name
			long offset = 0;

			fprintf(pf,"\tstrcpy(presetName[%d],\"%s%s\");\n",i+offset,name,presetName[i]);
			
			for(long j=0;j<kNumParams;j++)
				fprintf(pf,"\tpresets[%d][%d] = %ff;\n",i+offset,j,presets[i][j]);

			fprintf(pf,"\n");
		}

		fprintf(pf,"}\n");

		fclose(pf);
	}
#endif

	if(Noise != NULL)
		delete Noise;
}

void ASupaPhaser::setParameter (long index, float value)
{
	if(index < kNumParams && value >= 0.0 && value <= 1.0 )
	{
		SAVE[index] = value;
	}
	else
	{
		return;
	}

	if(index == kExtend)
		setParameterAutomated(kFreq,SAVE[kFreq]);

	if (editor)
		((ASupaEditor *)editor)->setParameter(index, value);
}

float ASupaPhaser::getParameter (long index)
{
	if(index < kNumParams)
		return SAVE[index];
	else
		return 0.f;
}

void ASupaPhaser::getParameterName (long index, char *label)
{
	switch(index)
	{
		case kMixture:	strcpy (label, "env-lfo mixture");break;
		case kAttack:	strcpy (label, "attack");break;
		case kRelease:	strcpy (label, "release");break;
		case kMinEnv:	strcpy (label, "min env");break;
		case kMaxEnv:	strcpy (label, "max env");break;
		case kDistort:	strcpy (label, "distortion");break;
		case kFeed:		strcpy (label, "feedback"); break;
		case kFreq:		strcpy (label, "sweep freq."); break;
		case kStereo:	strcpy (label, "stereo"); break;
		case kMinFreq:	strcpy (label, "min. depth"); break;
		case kMaxFreq:	strcpy (label, "max. depth"); break;
		case kGain:		strcpy (label, "out gain"); break;
		case kDryWet:	strcpy (label, "dry-wet"); break;
		case kExtend:	strcpy (label, "freq. range"); break;
		case knStages:	strcpy (label, "nr. stages"); break;
		case kInvert:	strcpy (label, "invert"); break;
	}
}

void ASupaPhaser::getParameterDisplay (long index, char *text)
{
	switch(index)
	{
		case kDistort:	long2string((long)(SAVE[index]*100.f),text);break;
		case kMinEnv:	long2string((long)(SAVE[index]*100.f),text);break;
		case kMaxEnv:	long2string((long)(SAVE[index]*100.f),text);break;
		case kRelease:
			{
				if(SAVE[index] < 0.0001f)
					long2string((long)(0.0001f*1000.f),text);
				else
					long2string((long)(SAVE[index]*1000.f),text);
					
				break;
			}
		case kAttack:
			{
				if(SAVE[index] < 0.0001f)
					long2string((long)(0.0001f*1000.f),text);
				else
					long2string((long)(SAVE[index]*0.4f*1000.f),text);
					
				break;
			}
		case kMixture:	long2string((long)(SAVE[index]*100.f),text);break;
		case kFeed:		long2string((long)(SAVE[index]*100.f),text);break;
		case kFreq:		
			{
				if(SAVE[kExtend] > 0.5f)
					float2string(ScaleFreq(SAVE[kFreq],4, 10.f),text);
				else
					float2string(ScaleFreq(SAVE[kFreq],4, 2.f),text);
				
				text[5] = 0;
				
				break;
			}
		case kStereo:	long2string((long)(SAVE[index]*360.f),text);break;
		case kMinFreq:	long2string((long)(SAVE[index]*100.f),text);break;
		case kMaxFreq:	long2string((long)(SAVE[index]*100.f),text);break;
		case kGain:		
			{
				dB2string(gainMap(SAVE[index]),text);
				text[4] = 0;
				break;
			}
		case kDryWet:	long2string(100 - (long)(SAVE[index]*100),text);break;
		case kExtend:
			{
				if(SAVE[kExtend] > 0.5f)
					strcpy(text, "large");
				else
					strcpy(text, "small");
				break;
			}
		case knStages:	
			{
				int n = Float2Int(SAVE[knStages],0,(double)MaxnStages);
				if(n < 3) n = 3;
				if(n > MaxnStages) n = MaxnStages;
				
				long2string(n,text);
				
				break;
			}
		case kInvert:
			{
				unsigned char x = (unsigned char) (SAVE[kInvert] * 3.0);
				bool invertWet = (x & 0x2) == 2;
				bool invertFeed = (x & 0x1) == 1;

				if(!invertWet && !invertFeed)
				{
					strcpy(text, "wet 0 / feed 0");
					break;
				}
				if(!invertWet && invertFeed)
				{
					strcpy(text, "wet 0 / feed 1");
					break;
				}
				if(invertWet && !invertFeed)
				{
					strcpy(text, "wet 1 / feed 0");
					break;
				}
				if(invertWet && invertFeed)
				{
					strcpy(text, "wet 1 / feed 1");
					break;
				}
				break;
			}
	}
}

void ASupaPhaser::getParameterLabel (long index, char *label)
{
	switch(index)
	{
		case kMixture:	strcpy (label, "%");break;
		case kAttack:	strcpy (label, "");break;
		case kRelease:	strcpy (label, "");break;
		case kMinEnv:	strcpy (label, "%");break;
		case kMaxEnv:	strcpy (label, "%");break;
		case kDistort:	strcpy (label, "");break;
		case kFeed:		strcpy (label, "%");break;
		case kFreq:		strcpy (label, "");break;
		case kStereo:	strcpy (label, "°");break;
		case kMinFreq:	strcpy (label, "%");break;
		case kMaxFreq:	strcpy (label, "%");break;
		case kGain:		strcpy (label, "");break;
		case kDryWet:	strcpy (label, "%");break;
		case kExtend:	strcpy (label, "");break;
		case knStages:	strcpy (label, "");break;
		case kInvert:	strcpy (label, "");break;
	}
}

void phase2(float *y1, float *in_11, float *y2, float *in_12, int n, float inputL, float inputR, float aLeft, float aRight)
{
	float tmpL = y1[0];
	float tmpR = y2[0];

	y1[0] = aLeft*(inputL - y1[0]) + in_11[0];
	y2[0] = aRight*(inputR - y2[0]) + in_12[0];
	
	in_11[0] = inputL;
	in_12[0] = inputR;
	
	float tmp2L;
	float tmp2R;
	int ntimes = (n-1)/2;
	int i=1;
	
	while(ntimes--)
	{
		tmp2L = y1[i];
		tmp2R = y2[i];

		y1[i] = aLeft*(y1[i-1] - tmp2L) + tmpL;
		y2[i] = aRight*(y2[i-1] - tmp2R) + tmpR;
		
		tmpL = y1[i+1];
		tmpR = y2[i+1];

		y1[i+1] = aLeft*(y1[i] - tmpL) + tmp2L;
		y2[i+1] = aRight*(y2[i] - tmpR) + tmp2R;
		
		i+=2;
	}

	if((n-1)%2)
	{
		y1[i] = aLeft*(y1[i-1] - y1[i]) + tmpL;
		y2[i] = aRight*(y2[i-1] - y2[i]) + tmpR;
	}
}

void ASupaPhaser::process(float **inputs, float **outputs, long sampleFrames)
{
	float *in1  =  inputs[0];
	float *in2  =  inputs[1];
	float *out1 = outputs[0];
	float *out2 = outputs[1];

	//some usefull pointers
	float *pNoise = Noise;

	//input to phaser
	float inputL, inputR;

	//make it quicker...
	float aL,aR,inL,inR;

	//LFO vars
	float LFO1,LFO2;

	float feed = SAVE[kFeed]*0.99f;
	float outtmp = gainMap(SAVE[kGain])*(float)sqrt(1-feed*feed);
	float freqtmp1 = (1-SAVE[kMixture])*(SAVE[kMaxFreq]+SAVE[kMinFreq])*0.5f;
	float freqtmp2 = (SAVE[kMaxFreq]-SAVE[kMinFreq])*0.5f*(1-SAVE[kMixture]);
	float freq = ScaleFreq(SAVE[kFreq],4,SAVE[kExtend] > 0.5f ? 10.f : 2.f);
	
	Osc1.setfreq(freq);
	Osc2.setfreq(freq);
	Osc2.setstereo(Osc1.getphase(),SAVE[kStereo]);

	int nStages = Float2Int(SAVE[knStages],0,(double)MaxnStages);
	if(nStages < 3) nStages = 3;
	if(nStages > MaxnStages) nStages = MaxnStages;

	// envelope gen 1
	float ga, ga2;
	if(SAVE[kAttack] < 0.0001f)
		ga = (float) expf(-1.f/(getSampleRate()*0.0001f));
	else
		ga = (float) expf(-1.f/(getSampleRate()*SAVE[kAttack]*0.4f)); //400Ms Max

	ga2 = 1-ga;

	// envelope gen 2
	float gr, gr2;
	if(SAVE[kRelease] < 0.0001f)
		gr = (float) expf(-1.f/(getSampleRate()*0.0001f));
	else
		gr = (float) expf(-1.f/(getSampleRate()*SAVE[kRelease]));

	gr2 = 1-gr;

	float envtmp2 = SAVE[kMaxEnv]*2.5f - SAVE[kMinEnv]*1.5f;
	float envtmp1 = SAVE[kMinEnv]*1.5f;

	// distortion
	float distMap = sqrtf(SAVE[kDistort]) * 0.95f;
	float dist = distMap;
	float k = 2.f*0.99f*distMap/(1.f-0.99f*distMap);
	float k2 = (1.f + k)*(1.f - DIST_FIX*SAVE[kDistort]);

	unsigned char x = (unsigned char) (SAVE[kInvert] * 3.0);
	bool invertWet = (x & 0x2) == 2;
	bool invertFeed = (x & 0x1) == 1;

	while(sampleFrames--)
	{
		inL = (*in1++) + (*pNoise);
        inR = (*in2++) + (*pNoise);
		pNoise++;

		/////////////////////////////////////////////////////////////////////////////////////////
		//LEFT
		/////////////////////////////////////////////////////////////////////////////////////////
				
		//LFO
		LFO1 = Osc1.GetVal();
		
		//Envelope
		float env1in = fabsf(inL);
			
		if(ENV1 < env1in)
			ENV1 = ga*(ENV1-env1in) + env1in;
		else
			ENV1 = gr*(ENV1-env1in) + env1in;
			
		float envscale = envtmp1 + envtmp2*ENV1;
		if(envscale>1.f)
			envscale=1.f;

		//phaser parameter
		aL = -SAVE[kMixture]*envscale - freqtmp1 - (float)LFO1*freqtmp2;

		//Distortion
		inL *= k2/(1.f+k*env1in);
				
		inputL = inL + prevOutL*feed; //input to the phaser is a mix of x and the feedback signal

		/////////////////////////////////////////////////////////////////////////////////////////
		//RIGHT
		/////////////////////////////////////////////////////////////////////////////////////////
		
		//LFO
		LFO2 = Osc2.GetVal();
				
		//Envelope
		float env2in = fabsf(inR);
		
		if(ENV2 < env2in)
			ENV2 = ga*(ENV2-env2in) + env2in;
		else
			ENV2 = gr*(ENV2-env2in) + env2in;
			
		envscale = envtmp1 + envtmp2*ENV2;
		if(envscale>1.f)
			envscale=1.f;

		//phaser var
		aR = -SAVE[kMixture]*envscale - freqtmp1 - (float)LFO2*freqtmp2;

		//Distort
		inR *= k2/(1.f+k*env2in);
				
		inputR = inR + prevOutR*feed;

		
		//blah...
		phase2(y1,in_11,y2,in_12,nStages,inputL,inputR,aL,aR);

		prevOutL = y1[nStages-1];
		prevOutR = y2[nStages-1];

		if(invertWet)
		{
			*out1++ += (SAVE[kDryWet]*(inL + prevOutL) - prevOutL)*outtmp;
			*out2++ += (SAVE[kDryWet]*(inR + prevOutR) - prevOutR)*outtmp;
		}
		else
		{
			*out1++ += (SAVE[kDryWet]*(inL - prevOutL) + prevOutL)*outtmp;
			*out2++ += (SAVE[kDryWet]*(inR - prevOutR) + prevOutR)*outtmp;
		}

		if(invertFeed)
		{
			prevOutL = -prevOutL;	
			prevOutR = -prevOutR;
		}
	}
}

void ASupaPhaser::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
	float *in1  =  inputs[0];
	float *in2  =  inputs[1];
	float *out1 = outputs[0];
	float *out2 = outputs[1];

	//some usefull pointers
	float *pNoise = Noise;

	//input to phaser
	float inputL, inputR;

	//make it quicker...
	float aL,aR,inL,inR;

	//LFO vars
	float LFO1,LFO2;

	float feed = SAVE[kFeed]*0.99f;
	float outtmp = gainMap(SAVE[kGain])*(float)sqrt(1-feed*feed);
	float freqtmp1 = (1-SAVE[kMixture])*(SAVE[kMaxFreq]+SAVE[kMinFreq])*0.5f;
	float freqtmp2 = (SAVE[kMaxFreq]-SAVE[kMinFreq])*0.5f*(1-SAVE[kMixture]);
	float freq = ScaleFreq(SAVE[kFreq],4,SAVE[kExtend] > 0.5f ? 10.f : 2.f);
	
	Osc1.setfreq(freq);
	Osc2.setfreq(freq);
	Osc2.setstereo(Osc1.getphase(),SAVE[kStereo]);

	int nStages = Float2Int(SAVE[knStages],0,(double)MaxnStages);
	if(nStages < 3) nStages = 3;
	if(nStages > MaxnStages) nStages = MaxnStages;

	// envelope gen 1
	float ga, ga2;
	if(SAVE[kAttack] < 0.0001f)
		ga = (float) expf(-1.f/(getSampleRate()*0.0001f));
	else
		ga = (float) expf(-1.f/(getSampleRate()*SAVE[kAttack]*0.4f)); //400Ms Max

	ga2 = 1-ga;

	// envelope gen 2
	float gr, gr2;
	if(SAVE[kRelease] < 0.0001f)
		gr = (float) expf(-1.f/(getSampleRate()*0.0001f));
	else
		gr = (float) expf(-1.f/(getSampleRate()*SAVE[kRelease]));

	gr2 = 1-gr;

	float envtmp2 = SAVE[kMaxEnv]*2.5f - SAVE[kMinEnv]*1.5f;
	float envtmp1 = SAVE[kMinEnv]*1.5f;

	// distortion
	float distMap = sqrtf(SAVE[kDistort]) * 0.95f;
	float dist = distMap;
	float k = 2.f*0.99f*distMap/(1.f-0.99f*distMap);
	float k2 = (1.f + k)*(1.f - DIST_FIX*SAVE[kDistort]);

	unsigned char x = (unsigned char) (SAVE[kInvert] * 3.0);
	bool invertWet = (x & 0x2) == 2;
	bool invertFeed = (x & 0x1) == 1;

	while(sampleFrames--)
	{
		inL = (*in1++) + (*pNoise);
        inR = (*in2++) + (*pNoise);
		pNoise++;

		/////////////////////////////////////////////////////////////////////////////////////////
		//LEFT
		/////////////////////////////////////////////////////////////////////////////////////////
				
		//LFO
		LFO1 = Osc1.GetVal();
		
		//Envelope
		float env1in = fabsf(inL);
			
		if(ENV1 < env1in)
			ENV1 = ga*(ENV1-env1in) + env1in;
		else
			ENV1 = gr*(ENV1-env1in) + env1in;
			
		float envscale = envtmp1 + envtmp2*ENV1;
		if(envscale>1.f)
			envscale=1.f;

		//phaser parameter
		aL = -SAVE[kMixture]*envscale - freqtmp1 - (float)LFO1*freqtmp2;

		//Distortion
		inL *= k2/(1.f+k*env1in);
				
		inputL = inL + prevOutL*feed; //input to the phaser is a mix of x and the feedback signal

		/////////////////////////////////////////////////////////////////////////////////////////
		//RIGHT
		/////////////////////////////////////////////////////////////////////////////////////////
		
		//LFO
		LFO2 = Osc2.GetVal();
				
		//Envelope
		float env2in = fabsf(inR);
			
		if(ENV2 < env2in)
			ENV2 = ga*(ENV2-env2in) + env2in;
		else
			ENV2 = gr*(ENV2-env2in) + env2in;
			
		envscale = envtmp1 + envtmp2*ENV2;
		if(envscale>1.f)
			envscale=1.f;

		//phaser var
		aR = -SAVE[kMixture]*envscale - freqtmp1 - (float)LFO2*freqtmp2;

		//Distort
		inR *= k2/(1.f+k*env2in);
				
		inputR = inR + prevOutR*feed;

		
		//blah...
		phase2(y1,in_11,y2,in_12,nStages,inputL,inputR,aL,aR);

		prevOutL = y1[nStages-1];
		prevOutR = y2[nStages-1];
		
		if(invertWet)
		{
			*out1++ = (SAVE[kDryWet]*(inL + prevOutL) - prevOutL)*outtmp;
			*out2++ = (SAVE[kDryWet]*(inR + prevOutR) - prevOutR)*outtmp;
		}
		else
		{
			*out1++ = (SAVE[kDryWet]*(inL - prevOutL) + prevOutL)*outtmp;
			*out2++ = (SAVE[kDryWet]*(inR - prevOutR) + prevOutR)*outtmp;
		}

		if(invertFeed)
		{
			prevOutL = -prevOutL;	
			prevOutR = -prevOutR;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// VST things
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ASupaPhaser::suspend()
{
	for(int i=0;i<MaxnStages;i++)
		y2[i]=in_12[i]=y1[i]=in_11[i]=0.f;
	
	prevOutR = prevOutL = 0.f;
	
	ENV1 = ENV2 = 0.f;

	Osc1.reset();
	Osc2.reset();
	Osc2.setstereo(Osc1.getphase(),SAVE[kStereo]);
}

void ASupaPhaser::setBlockSize(long blockSize)
{
	AudioEffect::setBlockSize(blockSize);
	if(Noise != NULL)
		delete Noise;
	Noise = new float[blockSize];
	InitNoise(Noise,blockSize);
}

void ASupaPhaser::setSampleRate(float sampleRate)
{
	AudioEffect::setSampleRate(sampleRate);
	Osc1.setrate(sampleRate);
	Osc2.setrate(sampleRate);
	Osc2.setstereo(Osc1.getphase(),SAVE[kStereo]);
}

bool ASupaPhaser::getEffectName (char* name)
{
	strcpy (name, "SupaPhaser");
	return true;
}

bool ASupaPhaser::getVendorString (char* text)
{
	strcpy (text, "Bram @ Smartelectronix");
	return true;
}

bool ASupaPhaser::getProductString (char* text)
{
	strcpy (text, "SupaPhaser");
	return true;
}

long ASupaPhaser::canDo (char* text)
{
	if (!strcmp(text, "receiveVstTimeInfo"))  return 0;
	if (!strcmp(text, "receiveVstMidiEvent")) return 0;
	if (!strcmp(text, "plugAsChannelInsert")) return 1;
	if (!strcmp(text, "plugAsSend")) return 1;
	if (!strcmp(text, "1in2out")) return 1;
	if (!strcmp(text, "2in2out")) return 1;

	return -1;	// explicitly can't do; 0 => don't know
}

bool ASupaPhaser::getInputProperties (long index, VstPinProperties* properties)
{
    if (index < 2)
    {
        sprintf (properties->label, "SupaPhaser %1d In", index + 1);
        properties->flags = kVstPinIsStereo | kVstPinIsActive;
        return true;
    }
    return false;
}

bool ASupaPhaser::getOutputProperties (long index, VstPinProperties* properties)
{
    if (index < 2)
    {
        sprintf (properties->label, "SupaPhaser %1d Out", index + 1);
        properties->flags = kVstPinIsStereo | kVstPinIsActive;
        return true;
    }
    return false;
}

bool ASupaPhaser::getProgramNameIndexed (long category, long index, char* text)
{
    if (index < kNumPrograms)
    {
        strcpy (text, presetName[index]);
        return true;
    }
    return false;
}

bool ASupaPhaser::copyProgram (long destination)
{
    if (destination < kNumPrograms)
    {
        memcpy(presets[destination],presets[curProgram],sizeof(float)*kNumParams);
		strcpy(presetName[destination],presetName[curProgram]);
        return true;
    }
    return false;
}

void ASupaPhaser::setProgram (long program)
{
	curProgram = program;
	
	SAVE = presets[curProgram];
	
	for(long i=0;i<kNumParams;i++)
		setParameter(i,SAVE[i]);
}

void ASupaPhaser::setProgramName (char *name)
{
	strcpy(presetName[curProgram], name);
}

//------------------------------------------------------------------------
void ASupaPhaser::getProgramName (char *name)
{
	strcpy (name, presetName[curProgram]);
}

// trim whitespace from char[]
void trim(char *text)
{
	long j=0,i=0;
	while(text[i])
	{
		if(text[i] == ' ')
			i++;
		else
			text[j++] = text[i++];
	}
	text[j] = 0;
}

// generate nice readable output!
void ASupaPhaser::getDisplay(long index, char *text)
{
	getParameterDisplay(index,text);
	trim(text);
	//text[5] = 0; //hack !hack !hack !hack !hack !hack !hack !
	getParameterLabel(index,&text[strlen(text)]);
	strupr(text);
}
