#include "CyanideEffect.h"
#include "CyanideEditor.h"

#include "Defines.h"
#include "Presets.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

extern bool oome;

CyanideEffect::CyanideEffect(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, kNumPrograms, kNumParams)
{
	//borg stuff
	setUniqueID ('Cya2');
	setNumInputs(2);
	setNumOutputs(2);
	canMono();
	canProcessReplacing(true);
	programsAreChunks();
	
	//filters and processing
	F1L.setrate(getSampleRate());
	F1R.setrate(getSampleRate());
	F2L.setrate(getSampleRate());
	F2R.setrate(getSampleRate());
	
	shaper.updatedata();
	
	//load some defaults
	setParameter(kPreGain,0.76f);
	setParameter(kPreFilter,1.f);
	setParameter(kPostGain,0.76f);
	setParameter(kPostFilter,1.f);
	setParameter(kPreType,0.f);
	setParameter(kPostType,0.f);
	setParameter(kDryWet,1.f);
	setParameter(kOverSample,0.f);
	
	//initialise the noise
	for (long i=0;i<FRAME_SIZE;i++)
		noise[i] = (float)(((double)rand())/(RAND_MAX*1048576.0)); //anti-denormalising noise

	//load the presets
	setChunk((void *)preset,17152,false);
	
	previous_oversample = false;

	//the editor...
	editor = new CyanideEditor (this,&shaper);
	
	if (!editor)
		oome = true;

	resume();
}

CyanideEffect::~CyanideEffect()
{
}

void CyanideEffect::setProgram(long program)
{
	if(program < kNumPrograms)
	{
		//save what's in memory to the programs
		saveProgram(curProgram);
		curProgram = program;
		loadProgram(curProgram);
	}
}

void CyanideEffect::setProgramName(char *name)
{
	if(curProgram < kNumPrograms)
		strcpy (programs[curProgram].name, name);
}

void CyanideEffect::getProgramName(char *name)
{
	if(curProgram < kNumPrograms)
		strcpy (name, programs[curProgram].name);
	else
		strcpy (name, "");
}

bool CyanideEffect::getProgramNameIndexed(long category, long index, char* text)
{
	//the borg seem to have fucked up the way of callin' this in Wavelab!
	//category and index seem to be swapped in Wavelab ?!?
	//as category gets called 0..15 and index stays zero
	
	if(index >= 0 && index < kNumPrograms)
	{
		strcpy(text,programs[index].name);
		return true;
	}
	
	return false;
}


void CyanideEffect::setParameter (long index, float value)
{
	if(index < kNumParams)
	{
		switch(index)
		{
			case kPreFilter:	fPreFilter = value;		break;
			case kPreGain:		fPreGain = value;		break;
			case kPostFilter:	fPostFilter = value;	break;
			case kPostGain:		fPostGain = value;		break;
			case kPreType:		fPreType = value;		break;
			case kPostType:		fPostType = value;		break;
			case kDryWet:		fDryWet = value;		break;
			case kOverSample:	fOverSample	= value;	break;
			case kShaper:		shaper.updatedata();	break;
		}

		if (editor)
			((AEffGUIEditor *)editor)->setParameter(index, value);
	}
}

float CyanideEffect::getParameter (long index)
{
	switch(index)
	{
		case kPreFilter  : return fPreFilter; break;
		case kPreGain    : return fPreGain;   break;
		case kPostFilter : return fPostFilter; break;
		case kPostGain   : return fPostGain;  break;
		case kPostType	 : return fPostType; break;
		case kPreType	 : return fPreType; break;
		case kShaper     : return 0.f; break;
		case kOverSample : return fOverSample; break;
		case kDryWet     : return fDryWet; break;
		default			 : return 0.f; break;
	}
}

void CyanideEffect::getParameterName (long index, char *label)
{
	switch(index)
	{
		case kPreFilter  : strcpy(label,"PreFilter"); break;
		case kPreGain    : strcpy(label,"PreGain"); break;
		case kPostFilter : strcpy(label,"PostFilter"); break;
		case kPostGain   : strcpy(label,"PostGain"); break;
		case kPostType	 : strcpy(label,"PostType"); break;
		case kPreType    : strcpy(label,"PreType"); break;
		case kDryWet     : strcpy(label,"DryWet"); break;
		case kOverSample : strcpy(label,"OverSample"); break;
		case kShaper     : strcpy(label,"Shaper"); break;
		default			 : strcpy(label,""); break;
	}
}

void CyanideEffect::getParameterDisplay (long index, char *text)
{
	switch(index)
	{
		case kPreFilter  :
			{
				float2string(F1L.getFreq((long)(fPreFilter*(nPre-1.f))),text);
				break;
			}
		case kPreGain    : dB2string(gainMapScaled(fPreGain),text); break;
		case kPostFilter : 
			{
				float2string(F1L.getFreq((long)(fPostFilter*(nPre-1.f))),text);
				break;
			}
		case kDryWet	 : long2string((long)(fDryWet*100),text); break;
		case kPostGain   : dB2string(gainMapScaled(fPostGain),text); break;
		case kOverSample :
			{
				if(fOverSample > 0.5f)
					strcpy(text,"16 times oversampling");
				else
					strcpy(text,"No oversampling");
				break;
			}
		case kPreType    : 
			{
				long LP_mix = 100 - (long)(fPreType*100.f);
				long HP_mix = (long)(fPreType*100.f);
				sprintf(text,"LP %d%% | HP %d%%",LP_mix,HP_mix);
				break;
			}
		case kPostType   :
			{
				long LP_mix = 100 - (long)(fPostType*100.f);
				long HP_mix = (long)(fPostType*100.f);
				sprintf(text,"LP %d%% | HP %d%%",LP_mix,HP_mix);
				break;
			}
		case kShaper     : strcpy(text,""); break;
		default			 : strcpy(text,""); break;
	}
}

void CyanideEffect::getParameterLabel (long index, char *label)
{
	switch(index)
	{
		case kPreFilter  : strcpy(label,"Hz"); break;
		case kPreGain    : strcpy(label,"dB"); break;
		case kPostFilter : strcpy(label,"Hz"); break;
		case kPostGain   : strcpy(label,"dB"); break;
		case kDryWet	 : strcpy(label,"% wet"); break;
		case kPreType    : 
		case kPostType   : 
		case kShaper     : strcpy(label,""); break;
		case kOverSample : strcpy(label,""); break;
		default			 : strcpy(label,""); break;
	}
}

void CyanideEffect::resume()
{
	wantEvents();
}

void CyanideEffect::suspend()
{
	F1L.suspend();
	F1R.suspend();
	F2L.suspend();
	F2R.suspend();
	for(long i=0;i<7;i++)
	{
		P1[i].suspend();
		P2[i].suspend();
	}
}

void CyanideEffect::setSampleRate(float sampleRate)
{
	AudioEffect::setSampleRate(sampleRate);

	F1L.setrate(sampleRate);
	F1R.setrate(sampleRate);
	F2L.setrate(sampleRate);
	F2R.setrate(sampleRate);
	
	for(long i=0;i<7;i++)
	{
		P1[i].suspend();
		P2[i].suspend();
	}
}

void save_data(unsigned char *x, long &pointer, unsigned char *data, long size)
{
	memcpy(x+pointer,data,size);
	pointer += size;
}

//saves the given program into the array...
void CyanideEffect::saveProgram(long index)
{
	long  pointer = 0;
	long i;
	
	for(i=1;i<kNumParams;i++)
	{
		float tmp = reverse(this->getParameter(i));
		save_data(programs[index].save,pointer,(unsigned char *)&tmp,sizeof(float));
	}
		
	long n = shaper.GetNPoints();
	long n_rev = reverse(n); //SAVE the reversed form, but don't mess up this one
	save_data(programs[index].save,pointer,(unsigned char *)&n_rev,sizeof(long));
	
	SplinePoint points[maxn];
	shaper.GetData(points);
	
	for(i=0;i<n;i++)
	{
		float x = reverse(points[i].x);
		save_data(programs[index].save,pointer,(unsigned char *)&x,sizeof(float));
		
		float y = reverse(points[i].y);
		save_data(programs[index].save,pointer,(unsigned char *)&y,sizeof(float));
	}
}

void CyanideEffect::loadProgram(long index)
{
	long  pointer = 0;
	unsigned char *cdata = (unsigned char *) programs[index].save;
	
	long i;
	for(i=1;i<kNumParams;i++)
	{
		float tmp = reverse(*(float *)(cdata + pointer));
		setParameter(i,tmp);
		pointer += sizeof(float);
	}
	
	long n = reverse(*(long *)(cdata + pointer));
	pointer += sizeof(long);
			
	SplinePoint points[maxn];	  
				
	for(i=0;i<n;i++)
	{
		points[i].x = reverse(*(float *)(cdata + pointer));
		pointer += sizeof(float);
		points[i].y = reverse(*(float *)(cdata + pointer));
		pointer += sizeof(float);
	}
	
	shaper.SetData(points,n);
	shaper.updatedata();

	setParameter(kShaper,0.f);
}

long CyanideEffect::getChunk(void** data, bool isPreset)
{
	//make sure the current program is up to date!
	saveProgram(curProgram);

	if(isPreset)
	{
		*data = (void *)&programs[curProgram];
		
		return sizeof(CyanideEffectProgram);
	}
	else //save a BANK
	{
		*data = (void *)&programs[0];
		
		return sizeof(CyanideEffectProgram) * kNumPrograms;
	}
}

long CyanideEffect::setChunk(void* data, long byteSize, bool isPreset)
{
	if(isPreset)
	{
		if( byteSize == sizeof(CyanideEffectProgram) )
		{
			CyanideEffectProgram *tmp = (CyanideEffectProgram *) data;
			
			if(strcmp(tmp->kookie,thisVersionKookie) == 0)
			{
				memcpy(&programs[curProgram],tmp,sizeof(CyanideEffectProgram));
				
				loadProgram(curProgram);
				
				return 1;
			}
			return 0;
		}
	}
	else //load a BANK
	{
		if( byteSize == sizeof(CyanideEffectProgram) * kNumPrograms )
		{
			for(long i=0;i<kNumPrograms;i++)
			{
				CyanideEffectProgram *tmp = &((CyanideEffectProgram *)data)[i];
				
				if(strcmp(tmp->kookie,thisVersionKookie) == 0) //IF equal
					memcpy(&programs[i],tmp,sizeof(CyanideEffectProgram));
			}

			loadProgram(curProgram);
			
			return 1;
		}

		return 0;
	}
	
	return 0;
}

bool CyanideEffect::getInputProperties (long index, VstPinProperties* properties)
{
    if (index < 2)
    {
        sprintf (properties->label, "Cyanide %1d In", index + 1);
        properties->flags = kVstPinIsStereo | kVstPinIsActive;
        return true;
    }
    return false;
}

bool CyanideEffect::getOutputProperties (long index, VstPinProperties* properties)
{
    if (index < 2)
    {
        sprintf (properties->label, "Cyanide %1d Out", index + 1);
        properties->flags = kVstPinIsStereo | kVstPinIsActive;
        return true;
    }
    return false;
}

bool CyanideEffect::getEffectName (char* name)
{
	strcpy (name, "Cyanide 2");
	return true;
}

bool CyanideEffect::getVendorString (char* text)
{
	strcpy (text, "Bram @ Smartelectronix");
	return true;
}

bool CyanideEffect::getProductString (char* text)
{
	strcpy (text, "Cyanide 2");
	return true;
}

long CyanideEffect::canDo(char* text)
{
	if (!strcmp (text, "receiveVstEvents"))
		return 1;
	if (!strcmp (text, "receiveVstMidiEvent"))
		return 1;
	if (!strcmp (text, "plugAsChannelInsert"))
		return 1;
	if (!strcmp (text, "plugAsSend"))
		return 1;
	return -1;	// explicitly can't do; 0 => don't know
}

///////////////////////////////////////////////////////////////////////////////////////////
// here comes the processing!!!

long CyanideEffect::processEvents (VstEvents* ev)
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
				case 73 : setParameter(kPreGain, value); break;
				case 74 : setParameter(kPreType, value); break;
				case 75 : setParameter(kPreFilter, value); break;
				
				case 76 : setParameter(kPostGain, value); break;
				case 77 : setParameter(kPostType, value); break;
				case 78 : setParameter(kPostFilter, value); break;

				case 79 : setParameter(kDryWet, value); break;
			}
		}
	}
	return 1;
}

//the processing call...
void CyanideEffect::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
	long frames;
	for(frames=0;frames < sampleFrames/FRAME_SIZE;frames++)
	{
		float * in[2]  = {&inputs[0][frames * FRAME_SIZE],&inputs[1][frames * FRAME_SIZE]};
		float * out[2] = {&outputs[0][frames * FRAME_SIZE],&outputs[1][frames * FRAME_SIZE]};
	
		InternProcess(in,out,FRAME_SIZE,true);
	}

	long todo = sampleFrames % FRAME_SIZE;

	if(todo)
	{
		float * in[2]  = {&inputs[0][frames * FRAME_SIZE],&inputs[1][frames * FRAME_SIZE]};
		float * out[2] = {&outputs[0][frames * FRAME_SIZE],&outputs[1][frames * FRAME_SIZE]};

		InternProcess(in,out,todo,true);
	}
}

//the processing call...
void CyanideEffect::process(float **inputs, float **outputs, long sampleFrames)
{
	long frames;
	for(frames=0;frames < sampleFrames/FRAME_SIZE;frames++)
	{
		float * in[2]  = {&inputs[0][frames * FRAME_SIZE],&inputs[1][frames * FRAME_SIZE]};
		float * out[2] = {&outputs[0][frames * FRAME_SIZE],&outputs[1][frames * FRAME_SIZE]};
	
		InternProcess(in,out,FRAME_SIZE,false);
	}

	long todo = sampleFrames % FRAME_SIZE;

	if(todo)
	{
		float * in[2]  = {&inputs[0][frames * FRAME_SIZE],&inputs[1][frames * FRAME_SIZE]};
		float * out[2] = {&outputs[0][frames * FRAME_SIZE],&outputs[1][frames * FRAME_SIZE]};

		InternProcess(in,out,todo,false);
	}
}

//out = in*amount + noise
void amp(float *input, float *output, float *noise, float amount, long nSamp)
{
	long i;
	for(i=0;i<nSamp/4;i++)
	{
		output[0] = input[0]*amount + noise[0];
		output[1] = input[1]*amount + noise[1];
		output[2] = input[2]*amount + noise[2];
		output[3] = input[3]*amount + noise[3];

		output += 4;
		input += 4;
		noise += 4;
	}

	for(i=0;i<nSamp % 4;i++)
	{
		output[i] = (input[i] + noise[i])*amount;
	}
}

//out = in1*a1 + in2*an
void mix(float *output, float *in1, float *in2, float a1, float a2, long nSamp)
{
	long i;
	for(i=0;i < nSamp/4;i++)
	{
		output[0] = in1[0]*a1 + in2[0]*a2;
		output[1] = in1[1]*a1 + in2[1]*a2;
		output[2] = in1[2]*a1 + in2[2]*a2;
		output[3] = in1[3]*a1 + in2[3]*a2;

		output += 4;
		in1 += 4;
		in2 += 4;
	}

	for(i=0;i < nSamp % 4;i++)
		output[i] = in1[i]*a1 + in2[i]*a2;
}

//out += in1*a1 + in2*an
void mixadd(float *output, float *in1, float *in2, float a1, float a2, long nSamp)
{
	long i;
	for(i=0;i < nSamp/4;i++)
	{
		output[0] += in1[0]*a1 + in2[0]*a2;
		output[1] += in1[1]*a1 + in2[1]*a2;
		output[2] += in1[2]*a1 + in2[2]*a2;
		output[3] += in1[3]*a1 + in2[3]*a2;

		output += 4;
		in1 += 4;
		in2 += 4;
	}

	for(i=0;i < nSamp % 4;i++)
		output[i] += in1[i]*a1 + in2[i]*a2;
}

void CyanideEffect::InternProcess(float **inputs, float **outputs, long sampleFrames, bool replace)
{
	float *in1  =  inputs[0];
	float *in2  =  inputs[1];
	float *out1 = outputs[0];
	float *out2 = outputs[1];

	long newf1 = (long)(fPreFilter*(nPre-1.f));
	long newf2 = (long)(fPostFilter*(nPre-1.f));
	float PreGain = gainMapScaled(fPreGain);
	float PostGain = gainMapScaled(fPostGain);
	float m1 = sinf(fDryWet*3.141592f*0.5f);
	float m2 = cosf(fDryWet*3.141592f*0.5f);

	bool oversample = fOverSample > 0.5f;
	
	if(previous_oversample != oversample)
	{
		for(long i=0;i<8;i++)
		{
			P1[i].suspend();
			P2[i].suspend();
		}

		previous_oversample = oversample;
	}

	F1L.settype(fPreType);
	F1R.settype(fPreType);
	F2L.settype(fPostType);
	F2R.settype(fPostType);
	
	//left

	amp(in1,buffer1,noise,PreGain,sampleFrames);
	F1L.processchange(buffer1,buffer1,newf1,sampleFrames);
		
	if(!oversample)
		shaper.process(buffer1,buffer1,sampleFrames);
	else
	{
		P1[0].Upsample(buffer1,buffer2,sampleFrames);
		P1[1].Upsample(buffer2,buffer1,sampleFrames*2);
		P1[2].Upsample(buffer1,buffer2,sampleFrames*4);
		P1[3].Upsample(buffer2,buffer1,sampleFrames*8);
	
		shaper.process(buffer1,buffer1,sampleFrames*16);

		P1[4].Downsample(buffer1,buffer2,sampleFrames*8);
		P1[5].Downsample(buffer2,buffer1,sampleFrames*4);
		P1[6].Downsample(buffer1,buffer2,sampleFrames*2);
		P1[7].Downsample(buffer2,buffer1,sampleFrames);
	}

	
	F2L.processchange(buffer1,buffer1,newf2,sampleFrames);

	if(replace)
		mix(out1,buffer1,in1,m1*PostGain,m2,sampleFrames);
	else
		mixadd(out1,buffer1,in1,m1*PostGain,m2,sampleFrames);

	/////////////////////////////////////////////////////////
	//right

	amp(in2,buffer1,noise,PreGain,sampleFrames);
	
	F1R.processchange(buffer1,buffer1,newf1,sampleFrames);

	if(!oversample)
		shaper.process(buffer1,buffer1,sampleFrames);
	else
	{
		P2[0].Upsample(buffer1,buffer2,sampleFrames);
		P2[1].Upsample(buffer2,buffer1,sampleFrames*2);
		P2[2].Upsample(buffer1,buffer2,sampleFrames*4);
		P2[3].Upsample(buffer2,buffer1,sampleFrames*8);
	
		shaper.process(buffer1,buffer1,sampleFrames*16);

		P2[4].Downsample(buffer1,buffer2,sampleFrames*8);
		P2[5].Downsample(buffer2,buffer1,sampleFrames*4);
		P2[6].Downsample(buffer1,buffer2,sampleFrames*2);
		P2[7].Downsample(buffer2,buffer1,sampleFrames);
	}
	
	F2R.processchange(buffer1,buffer1,newf2,sampleFrames);
	
	if(replace)
		mix(out2,buffer1,in2,m1*PostGain,m2,sampleFrames);
	else
		mixadd(out2,buffer1,in2,m1*PostGain,m2,sampleFrames);
}
