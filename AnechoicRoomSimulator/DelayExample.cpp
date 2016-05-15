#include "DelayExample.hpp"
#include "ASupaEditor.h"
#include "math.h"


//-----------------------------------------------------------------------------
CDelayExample::CDelayExample(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, kNumPrograms, kNumParams)
{
	setNumInputs(kNumInputChannels);		// stereo in
	setNumOutputs(kNumOutputChannels);		// stereo out
	setUniqueID('AneC');					// TODO: Change for plugin identification
	canMono();								// makes sense to feed both inputs with the same signal
	canProcessReplacing();					// supports both accumulating and replacing output

	//clear buffers!
	suspend();

	setParameter(kSize,0.31415f);

	editor = new ASupaEditor(this);
}

//-----------------------------------------------------------------------------------------
CDelayExample::~CDelayExample()
{
	// nothing to do here
	
	if(editor)
		delete editor;
	editor = 0;
}



///////////////////////////////////////////////////////////////////////////////////////////
// the parameters of the plugin
///////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------
void CDelayExample::setParameter(long index, float value)
{
	// save the parameter for the host...
	savedParameters[index] = value;

	// TODO: do something usefull with your parameter
	// an example might be: gain = savedParameters[index] * 2.f;
	// where gain is a class-varaible you use in processing
	switch(index)
	{
		//outputPointer and inputPointer are UNSIGNED and will 'wrap around'
		case kSize		: break;
		default			: break;
	}

	if (editor)
		((ASupaEditor *)editor)->setParameter(index, value);
}

//-----------------------------------------------------------------------------------------
float CDelayExample::getParameter(long index)
{
	// you should only really change this in special occasions
	// for example, when you use programs!!
	if(index >= 0 && index < kNumParams)
		return savedParameters[index]; // TODO: if you changed the parametername in DelayExample.hpp it needs to be changed here aswell
	else
		return 0.f;
}

//-----------------------------------------------------------------------------------------
void CDelayExample::getParameterName(long index, char *label)
{
	// TODO: give the parameters names, make sure you don't use more than 24 characters!
	switch(index)
	{
		case kSize		: strcpy(label, "Room Size");		break;
		default			: strcpy(label, "");			break;
	}
}

//-----------------------------------------------------------------------------------------
void CDelayExample::getParameterDisplay(long index, char *text)
{
	// TODO: give the parameters displays
	// you could use float2string(), dB2string(), long2string() or roll your own
	switch(index)
	{
		case kSize		: long2string((long)(savedParameters[index]*10000.f), text);	break;
		default			: strcpy(text, "");	break;
	}
}

//-----------------------------------------------------------------------------------------
void CDelayExample::getParameterLabel(long index, char *label)
{
	// TODO: give the parameters labels
	switch(index)
	{
		case kSize		: strcpy(label, " m");		break;
		default			: strcpy(label, "");		break;
	}
}



///////////////////////////////////////////////////////////////////////////////////////////
// programs (i.e. presets)
///////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------
void CDelayExample::setProgramName(char *name)
{
	// this template does not use programs yet
}

//-----------------------------------------------------------------------------------------
void CDelayExample::getProgramName(char *name)
{
	// this template does not use programs yet
	strcpy(name, "");
}

//-----------------------------------------------------------------------------------------
void CDelayExample::setProgram(long index)
{
		// this template does not use programs yet
};



///////////////////////////////////////////////////////////////////////////////////////////
// the actual processing
///////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------
void CDelayExample::process(float **inputs, float **outputs, long sampleFrames)
{
	// reminder : VST allows for sampleFrames to be different every block

	// this will only work if you have two in's and two out's!!
	float *in1  =  inputs[0];
	float *in2  =  inputs[1];
	float *out1 = outputs[0];
	float *out2 = outputs[1];

	// accumilate into output
	// watch out: in and out might actualy be the SAME array!!!
	for(long i=0;i<sampleFrames;i++)
	{
		out1[i] += in1[i];
		out2[i] += in2[i];
	}
}

//-----------------------------------------------------------------------------------------
void CDelayExample::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
	// reminder : VST allows for sampleFrames to be different every block

	// this will only work if you have two in's and two out's!!
	float *in1  =  inputs[0];
	float *in2  =  inputs[1];
	float *out1 = outputs[0];
	float *out2 = outputs[1];

	// replace output
	// watch out: in and out might actualy be the SAME array!!!
	for(long i=0;i<sampleFrames;i++)
	{
		out1[i] = in1[i];
		out2[i] = in2[i];
	}
}



///////////////////////////////////////////////////////////////////////////////////////////
// some more advanced VST features you should really use!
///////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------
bool CDelayExample::getEffectName (char* name)
{
	// TODO: return the real name of your plugin
	strcpy (name, "Anechoic Room Simulator");
	return true;
}

//-----------------------------------------------------------------------------------------
bool CDelayExample::getVendorString (char* text)
{
	// TODO: return the real name of your company
	strcpy (text, "Bram @ Smartelectronix");
	return true;
}

//-----------------------------------------------------------------------------------------
bool CDelayExample::getProductString (char* text)
{
	// TODO: return the real name of your product
	strcpy (text, "Anechoic Room Simulator");
	return true;
}

//-----------------------------------------------------------------------------------------
long CDelayExample::canDo(char* text)
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



///////////////////////////////////////////////////////////////////////////////////////////
// some features you might find usefull
///////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------
void CDelayExample::suspend()
{
	// this will get called before changes in -for example- the samplerate
	// or when the plugin is switched OFF
}

//-----------------------------------------------------------------------------------------
void CDelayExample::resume()
{
	// this will get called after changes in -for example- the samplerate
	// or when the plugin is switched ON
}

//-----------------------------------------------------------------------------------------
void CDelayExample::setSampleRate(float sampleRate)
{
	// allways call this
	AudioEffect::setSampleRate(sampleRate);

}

void CDelayExample::setBlockSize (long blockSize)
{
	// allways call this
	AudioEffect::setBlockSize(blockSize);

	// TODO: the MAXIMUM block size has changed...
}
