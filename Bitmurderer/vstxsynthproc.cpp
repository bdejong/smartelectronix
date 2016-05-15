#ifndef __VstXSynth__
#include "vstxsynth.h"
#endif

#include "math.h"
#include "stdlib.h"

short createmask(float *bits)
{
	unsigned short tmp = 0;
	for(char i=0;i<15;i++)
		tmp = (tmp << 1) + (bits[i] > 0.5f);
	tmp |= 0x8000;
	return *(short *)&tmp;
}

float clip(float x)
{
	if(x > 1.f)
		return 1.f;
	return x;
}

float process1(float in, short mask, bool xor, bool and, bool power)
{
	bool sign = in < 0.f;

	in = clip(fabsf(in));

	if(power)
		in = powf(in,1/3.f);

	short x = (short)(in * 32767.f);

	if(xor)
		x = x ^ mask;

	if(and)
		x = x & mask;

	x &= 0x7fff; //reset the sign bit

	float out = (float) x / 32767.f;

	if(power)
		out = powf(out,3.f);

	return sign ? -out : out;
}

void VstXSynth::setSampleRate (float sampleRate)
{
	AudioEffectX::setSampleRate (sampleRate);
}

void VstXSynth::process(float **inputs, float **outputs, long sampleFrames)
{
	float *out1 = outputs[0];
	float *out2 = outputs[1];
	float *in1 = inputs[0];
	float *in2 = inputs[1];

	short mask = createmask(SAVE);
	bool or = SAVE[kOr]>0.5f;
	bool and = SAVE[kAnd]>0.5f;
	bool sig = SAVE[kSig]>0.5f;

	for(long i=0;i<sampleFrames;i++)
	{
		out1[i] += process1(in1[i],mask,or,and,sig);
		out2[i] += process1(in2[i],mask,or,and,sig);
	}
}

void VstXSynth::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
	float *out1 = outputs[0];
	float *out2 = outputs[1];
	float *in1 = inputs[0];
	float *in2 = inputs[1];

	short mask = createmask(SAVE);
	bool or = SAVE[kOr]>0.5f;
	bool and = SAVE[kAnd]>0.5f;
	bool sig = SAVE[kSig]>0.5f;

	for(long i=0;i<sampleFrames;i++)
	{
		out1[i] = process1(in1[i],mask,or,and,sig);
		out2[i] = process1(in2[i],mask,or,and,sig);
	}
}
