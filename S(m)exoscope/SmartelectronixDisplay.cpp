#include "SmartelectronixDisplay.hpp"
#include "SmartelectronixDisplayEditor.h"

#include "math.h"

//-----------------------------------------------------------------------------
CSmartelectronixDisplay::CSmartelectronixDisplay(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, kNumPrograms, kNumParams)
{
	setNumInputs(kNumInputChannels);
	setNumOutputs(kNumOutputChannels);

#if !SIMPLE_VERSION
	setUniqueID('osX1');
#else
	setUniqueID('osX2');
#endif

	
	canMono();
	canProcessReplacing();

	long j;
	for(j=0;j<OSC_WIDTH*2;j+=2)
	{
		peaks[j].h = j/2;
		peaks[j].v = OSC_HEIGHT/2-1;
		peaks[j+1].h = j/2;
		peaks[j+1].v = OSC_HEIGHT/2-1;
		copy[j].h = j/2;
		copy[j].v = OSC_HEIGHT/2-1;
		copy[j+1].h = j/2;
		copy[j+1].v = OSC_HEIGHT/2-1;
	}

	setParameter(kTriggerSpeed,0.5f);
	setParameter(kTriggerType,0.f);
	setParameter(kTriggerLevel,0.5f);
	setParameter(kTriggerLimit,0.5f);
	setParameter(kTimeWindow,0.75f);
	setParameter(kAmpWindow,0.5f);
	setParameter(kSyncDraw,0.f);
	setParameter(kChannel,0.f);
	setParameter(kFreeze,0.f);
	setParameter(kDCKill,0.f);
	
	suspend();

	editor = new CSmartelectronixDisplayEditor(this);
}

//-----------------------------------------------------------------------------------------
CSmartelectronixDisplay::~CSmartelectronixDisplay()
{
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::processSub(float **inputs, long sampleFrames)
{
	if(SAVE[kFreeze] > 0.5)
	{
		suspend();
		return;
	}
	
	float *samples = SAVE[kChannel] > 0.5 ? inputs[1] : inputs[0];

#if !SIMPLE_VERSION
	float *triggerExternal = inputs[2];
#endif

	//some simple parameter mappings...
	float	gain			= powf(10.f,SAVE[kAmpWindow]*6.f - 3.f);
	float	triggerLevel	= (SAVE[kTriggerLevel]*2.f-1.f);
	long	triggerType		= (long)(SAVE[kTriggerType]*kNumTriggerTypes + 0.0001);
	long	triggerLimit	= (long)(pow(10.f,SAVE[kTriggerLimit]*4.f)); // [0=>1 1=>10000
	double	triggerSpeed	= pow(10.0,2.5*SAVE[kTriggerSpeed]-5.0);
	double	counterSpeed	= pow(10.f,-SAVE[kTimeWindow]*5.f + 1.5); // [0=>10 1=>0.001
	double	R				= 1.0 - 250.0/getSampleRate();
	bool	dcOn			= SAVE[kDCKill] > 0.5f;

	for(long i=0;i<sampleFrames;i++)
	{
		// DC filter...
		dcKill = samples[i] - dcFilterTemp + R * dcKill;
		
		dcFilterTemp = samples[i];
		
		if(fabs(dcKill) < 1e-10)
			dcKill = 0.f;

		// Gain
		float sample = dcOn ? (float)dcKill : samples[i];
		sample = clip(sample*gain,1.f);
		
		// triggers
		
		bool trigger = false;
	
		switch(triggerType)
		{
			case kTriggerInternal	:
				{
					// internal oscillator, nothing fancy
					triggerPhase += triggerSpeed;
					if(triggerPhase >= 1.0)
					{
						triggerPhase -= 1.0;
						trigger = true;
					}
					break;
				}
			case kTriggerRising		: 
				{
					// trigger on a rising edge
					if(sample >= triggerLevel && previousSample < triggerLevel)
						trigger = true;
					break;
				}
			case kTriggerFalling	:
				{
					// trigger on a falling edge
					if(sample <= triggerLevel && previousSample > triggerLevel)
						trigger = true;
					break;
				}
			case kTriggerFree		:
				{
					// trigger when we've run out of the screen area :-)
					if(index >= OSC_WIDTH)
						trigger = true;
					break;
				}
#if !SIMPLE_VERSION
			case kTriggerExternal	:
				{
					trigger = triggerExternal[i] >= 1.f;
					break;
				}
#endif
		}

		// if there's a retrigger, but too fast, kill it
		triggerLimitPhase++;
		if(trigger && triggerLimitPhase < triggerLimit && triggerType != kTriggerFree && triggerType != kTriggerInternal)
			trigger = false;

		// @ trigger
		if(trigger)
		{
			unsigned long j;
			
			// zero peaks after the last one
			for(j=index*2;j<OSC_WIDTH*2;j+=2)
				peaks[j].v = peaks[j+1].v = OSC_HEIGHT/2-1;

			// copy to a buffer for drawing!
			for(j=0;j<OSC_WIDTH*2;j++)
				copy[j] = peaks[j];

			// reset everything
			index = 0;
			counter = 1.0;
			max = -MAX_FLOAT;
			min = MAX_FLOAT;
			triggerLimitPhase = 0;
		}

		// @ sample
		if(sample > max)
		{
			max = sample;
			lastIsMax = true;
		}

		if(sample < min)
		{
			min = sample;
			lastIsMax = false;
		}

		counter += counterSpeed;

		// @ counter
		// the counter keeps track of how many samples/pixel we have
		if(counter >= 1.0)
		{
			if(index < OSC_WIDTH)
			{
				// scale here, better than in the graphics thread :-)
				long max_Y = (long)(OSC_HEIGHT*0.5f - max*0.5f*OSC_HEIGHT) - 1;
				long min_Y = (long)(OSC_HEIGHT*0.5f - min*0.5f*OSC_HEIGHT) - 1;

				// thanks to David @ Plogue for this interesting hint!
				peaks[(index<<1)].v = lastIsMax ? min_Y : max_Y;
				peaks[(index<<1)+1].v = lastIsMax ? max_Y : min_Y;

				index++;
			}

			max = -MAX_FLOAT;
			min = MAX_FLOAT;

			//counter = counter - (long)counter;
			counter -= 1.0;
		}
		
		// store for edge-triggers !
		previousSample = sample;
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------------------
bool CSmartelectronixDisplay::getEffectName (char* name)
{
	strcpy (name, PLUGIN_NAME);
	return true;
}

//-----------------------------------------------------------------------------------------
bool CSmartelectronixDisplay::getVendorString (char* text)
{
	strcpy (text, "Bram @ Smartelectronix");
	return true;
}

//-----------------------------------------------------------------------------------------
bool CSmartelectronixDisplay::getProductString (char* text)
{
	strcpy (text, PLUGIN_NAME);
	return true;
}

//-----------------------------------------------------------------------------------------
long CSmartelectronixDisplay::canDo(char* text)
{
	// set the capabilities of your plugin
	if (!strcmp (text, "receiveVstEvents"))    return 0;
	if (!strcmp (text, "receiveVstMidiEvent")) return 0;
	if (!strcmp (text, "receiveVstTimeInfo"))  return 0;
	if (!strcmp(text, "plugAsChannelInsert")) return 1;
	if (!strcmp(text, "plugAsSend")) return 1;
	if (!strcmp(text, "mixDryWet")) return 1;
	if (!strcmp(text, "1in2out")) return 1;
	if (!strcmp(text, "2in2out")) return 1;

	return -1;	// explicitly can't do; 0 => don't know
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::suspend()
{
	index = 0;
	counter = 1.0;
	max = -MAX_FLOAT;
	min = MAX_FLOAT;
	previousSample = 0.f;
	triggerPhase = 0.f;
	triggerLimitPhase = 0;
	dcKill = dcFilterTemp = 0.0;
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::resume()
{
	index = 0;
	counter = 1.0;
	max = -MAX_FLOAT;
	min = MAX_FLOAT;
	previousSample = 0.f;
	triggerPhase = 0.f;
	triggerLimitPhase = 0;
	dcKill = dcFilterTemp = 0.0;
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::setSampleRate(float sampleRate)
{
	// allways call this
	AudioEffect::setSampleRate(sampleRate);

	// TODO: the samplerate has changed...
}

void CSmartelectronixDisplay::setBlockSize (long blockSize)
{
	// allways call this
	AudioEffect::setBlockSize(blockSize);

	// TODO: the MAXIMUM block size has changed...
}

void CSmartelectronixDisplay::process(float **inputs, float **outputs, long sampleFrames)
{
#if SIMPLE_VERSION
	float *in1  =  inputs[0];
	float *in2  =  inputs[1];
	float *out1 = outputs[0];
	float *out2 = outputs[1];

	for(long i=0;i<sampleFrames;i++)
	{
		out1[i] += in1[i];
		out2[i] += in2[i];
	}
#endif

	processSub(inputs,sampleFrames);
}

void CSmartelectronixDisplay::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
#if SIMPLE_VERSION
	float *in1  =  inputs[0];
	float *in2  =  inputs[1];
	float *out1 = outputs[0];
	float *out2 = outputs[1];

	for(long i=0;i<sampleFrames;i++)
	{
		out1[i] = in1[i];
		out2[i] = in2[i];
	}
#endif

	processSub(inputs,sampleFrames);
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::setProgramName(char *name)
{
	// this template does not use programs yet
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::getProgramName(char *name)
{
	// this template does not use programs yet
	strcpy(name, "");
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::setProgram(long index)
{
		// this template does not use programs yet
};

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::setParameter(long index, float value)
{
	SAVE[index] = value;

	if (editor)
		((AEffGUIEditor *)editor)->setParameter(index, value);
}

//-----------------------------------------------------------------------------------------
float CSmartelectronixDisplay::getParameter(long index)
{
	return SAVE[index];
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::getParameterName(long index, char *label)
{
	switch(index)
	{
		case kTriggerSpeed	: strcpy(label, "Internal Trigger Speed"); break;
		case kTriggerType	: strcpy(label, "Trigger Type"); break;
		case kTriggerLevel	: strcpy(label, "Trigger Level"); break;
		case kTriggerLimit	: strcpy(label, "Trigger Limit"); break;
		case kTimeWindow	: strcpy(label, "Time"); break;
		case kAmpWindow		: strcpy(label, "Amp"); break;
		case kSyncDraw		: strcpy(label, "Sync Redraw"); break;
		case kChannel		: strcpy(label, "Channel"); break;
		case kFreeze		: strcpy(label, "Freeze"); break;
		case kDCKill		: strcpy(label, "DC Killer"); break;
		default				: strcpy(label, ""); break;
	}
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::getParameterDisplay(long index, char *text)
{
	switch(index)
	{
		case kTriggerType	:
			{
				long triggerType = (long)(SAVE[kTriggerType]*kNumTriggerTypes + 0.0001);
				long2string(triggerType,text);
				break;
			}
		case kTriggerLevel	:
			{
				float	triggerLevel	= (SAVE[kTriggerLevel]*2.f-1.f);
				float2string(triggerLevel,text);
				break;
			}
		case kTriggerLimit	:
			{
				long triggerLimit = (long)(pow(10.f,SAVE[kTriggerLimit]*4.f)); // [0=>1 1=>10000
				long2string(triggerLimit,text);
				break;
			}
		case kTimeWindow	:
			{
				double counterSpeed = pow(10.f,-SAVE[kTimeWindow]*5.f + 1.5); // [0=>10 1=>0.001
				float2string((float)counterSpeed,text);
				break;
			}
		case kTriggerSpeed	:
			{
				double triggerSpeed = pow(10.0,2.5*SAVE[kTriggerSpeed]-5.0) * getSampleRate();
				float2string((float)triggerSpeed,text);
				break;
			}
		case kAmpWindow		:
			{
				float gain = powf(10.f,SAVE[kAmpWindow]*6.f - 3.f);
				float2string(gain,text);
				break;
			}
		case kChannel		: 
			{
				if(SAVE[index] > 0.5f)
					strcpy(text, "right");
				else
					strcpy(text, "left");
				
				break;
			}
		case kSyncDraw		: 
		case kFreeze		: 
		case kDCKill		:
			{
				if(SAVE[index] > 0.5f)
					strcpy(text, "on");
				else
					strcpy(text, "off");
				
				break;
			}
		default: strcpy(text,""); break;
	}

//	float2string(SAVE[index],text);
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::getParameterLabel(long index, char *label)
{
	switch(index)
	{
		case kTriggerSpeed	: strcpy(label, ""); break;
		case kTriggerType	: strcpy(label, ""); break;
		case kTriggerLevel	: strcpy(label, ""); break;
		case kTriggerLimit	: strcpy(label, ""); break;
		case kTimeWindow	: strcpy(label, ""); break;
		case kAmpWindow		: strcpy(label, ""); break;
		case kSyncDraw		: strcpy(label, ""); break;
		case kChannel		: strcpy(label, ""); break;
		case kFreeze		: strcpy(label, ""); break;
		case kDCKill		: strcpy(label, ""); break;
		default				: strcpy(label, ""); break;
	}
}

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

void CSmartelectronixDisplay::getDisplay(long index, char *text)
{
	getParameterDisplay(index,text);
	trim(text);
	text[5] = 0; //hack !hack !hack !hack !hack !hack !hack !
	getParameterLabel(index,&text[strlen(text)]);
}
