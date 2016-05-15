#ifndef __DELAYEXAMPLE_H
#define __DELAYEXAMPLE_H

#include "audioeffectx.h"

//maximum delay in seconds
#define delayMaxSeconds 1

class CDelayExample : public AudioEffectX
{
public:

	enum
	{
		kSize,
		// TODO: Add more parameters here!
		kNumParams,
		kNumPrograms = 0,			// this template does not use programs (yet)
		kNumOutputChannels = 2,		// stereo in
		kNumInputChannels = 2		// stereo out
	};

	CDelayExample(audioMasterCallback audioMaster);
	~CDelayExample();

	virtual void	process(float **inputs, float **outputs, long sampleFrames);
	virtual void	processReplacing(float **inputs, float **outputs, long sampleFrames);
	
	virtual void	setProgramName(char *name);
	virtual void	setProgram(long index);
	virtual void	getProgramName(char *name);
	virtual void	setParameter(long index, float value);
	virtual float	getParameter(long index);
	virtual void	getParameterLabel(long index, char *label);
	virtual void	getParameterDisplay(long index, char *text);
	virtual void	getParameterName(long index, char *text);

	virtual long	canDo(char* text);
	virtual bool	getEffectName (char* name);
	virtual bool	getVendorString (char* text);
	virtual bool	getProductString (char* text);

	virtual void	suspend();
	virtual void	resume();
	virtual void	setSampleRate(float sampleRate);
	virtual void	setBlockSize (long blockSize);

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

	void getDisplay(long index, char *text)
	{
		getParameterDisplay(index,text);
		trim(text);
		getParameterLabel(index,&text[strlen(text)]);
		strupr(text);
	}

protected:
	
	float			savedParameters[kNumParams];

};

#endif
