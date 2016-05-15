#ifndef __VstXSynth__
#include "vstxsynth.h"
#endif

#include "math.h"
#include "stdlib.h"

inline float clip(float x)
{
	if(x > 1.f)
		x = 1.f;
	else
	{
		if (x < -1.f)
			x = -1.f;
	}

	return x;
}

inline float TriToSine(float Value)
{
	float v2 = Value*Value;
	return Value*(1.569799213f + 0.5f*(-1.279196852f + 0.139598426f*v2)*v2);
};

float process1(float in, short mask, bool b_xor, bool b_and, bool power)
{
	bool sign = in < 0.f;

	in = clip(fabsf(in));

	if(power)
		in = powf(in,1.f/3.f);

	short x = (short)(in * 32767.f);

	if(b_xor)
		x = x ^ mask;

	if(b_and)
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

	S.SampleRateF = getSampleRate();
	S2.SampleRateF = getSampleRate();
	F.T.SampleRateF = getSampleRate();
	E.SampleRateF = getSampleRate();

	isDirty = true;
}

float randbool()
{
	if (rand() > (RAND_MAX/2))
		return 1.f;
	else
		return 0.f;
}

float randfloat(float min=0.f, float max=1.f)
{
	return min + (max - min) * (float) rand() / (float) RAND_MAX;
}

#define YYY

void VstXSynth::randomise()
{
	setParameter(kmurder,randbool());
	setParameter(kbits,randfloat());
	setParameter(kand,randbool());
	setParameter(kxor,randbool());
	setParameter(kpower,randbool());
	setParameter(kfreq1,randfloat());
	setParameter(kdfreq1,randfloat());
	setParameter(kvibfreq,randfloat());
	setParameter(kdvibfreq,randfloat());
	setParameter(kfeed1,randfloat());
	setParameter(kfreq2,randfloat());
	setParameter(kmindelay,randfloat());
	setParameter(kmaxdelay,randfloat());
	setParameter(kdist,randfloat());
	setParameter(kfeed2,randfloat());
	setParameter(kdamp,randfloat());
	setParameter(klimiter,0.3f);
	setParameter(kattack,randfloat());
	setParameter(krelease,randfloat());
	setParameter(kamp,0.5f);
	setParameter(kdrywet,1.f);
}

void VstXSynth::resume()
{
	ivan_output = 0;

	F.suspend();
	LP.suspend();
	S.suspend();
	E.suspend();
	Delay.suspend();
	S2.suspend();
}

void VstXSynth::process(float **inputs, float **outputs, long sampleFrames)
{
	if(isDirty)
	{
		LP.SetParams(1.f - SCALED[kdamp]);

		float x1 = fabsf(SCALED[kfreq1] - SCALED[kdfreq1]);
		float x2 = fabsf(SCALED[kfreq1] + SCALED[kdfreq1]);

		if(x1 < 0.01f)
			x1 = 0.01f;
		if(x2 < 0.01f)
			x2 = 0.01f;

		x1 = getSampleRate()/x1;
		if(x1 > 500000.f) x1 = 500000.f;
		x2 = getSampleRate()/x2;
		if(x2 > 500000.f) x2 = 500000.f;

		S.SetParams(x1,x2);
		F.SetParams(SCALED[kfreq2],SCALED[kfeed2],SCALED[kdrywet],SCALED[kdist],SCALED[kmindelay],SCALED[kmaxdelay]);

		E.SetParams(SCALED[kattack],SCALED[krelease]);

		isDirty = false;
	}

	float	*out1	= outputs[0];
	float	*out2	= outputs[1];
	float	*in1	= inputs[0];
	bool	murder	= SCALED[kmurder] > 0.5f;
	short	mask	= (short)SCALED[kbits];
	bool	b_and		= SCALED[kand] > 0.5f;
	bool	b_xor		= SCALED[kxor] > 0.5f;
	bool	power	= SCALED[kpower] > 0.5f;
	float	tmp1	= (1.f - SCALED[kdrywet]);
	float	tmp2	= SCALED[kdrywet]*SCALED[kamp];
	float	tmp4	= (1.f-SCALED[kfeed1]);
	float	tmp3	= SCALED[klimiter] * 0.5f;

	for(long i=0;i<sampleFrames;i++)
	{
		float bitout;

		if(murder)
			bitout = process1(in1[i],mask,b_xor,b_and,power);
		else
			bitout = in1[i];

		ivan_output = F.GetVal(Delay.GetVal(LP.GetVal(SCALED[kfeed1]*ivan_output + tmp4*bitout),S.GetVal(TriToSine(S2.GetVal(SCALED[kvibfreq]))*SCALED[kdvibfreq])));

		float env_out = E.GetVal(ivan_output);

		if(env_out < 1.f)
			ivan_output *= 1.f - tmp3 * env_out;
		else
			ivan_output *= (1.f - tmp3 * 2.f) + tmp3 / env_out;

		out1[i] = in1[i]*tmp1 + ivan_output*tmp2;
		out2[i] = in1[i]*tmp1 + ivan_output*tmp2;
	}

	if(fabs(ivan_output) < 1e-10 || fabs(ivan_output) > 1e10)
		ivan_output = 0.f;
}

void VstXSynth::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
	if(isDirty)
	{
		LP.SetParams(1.f - SCALED[kdamp]);

		float x1 = fabsf(SCALED[kfreq1] - SCALED[kdfreq1]);
		float x2 = fabsf(SCALED[kfreq1] + SCALED[kdfreq1]);

		if(x1 < 0.01f)
			x1 = 0.01f;
		if(x2 < 0.01f)
			x2 = 0.01f;

		x1 = getSampleRate()/x1;
		if(x1 > 500000.f) x1 = 500000.f;
		x2 = getSampleRate()/x2;
		if(x2 > 500000.f) x2 = 500000.f;

		S.SetParams(x1,x2);
		F.SetParams(SCALED[kfreq2],SCALED[kfeed2],SCALED[kdrywet],SCALED[kdist],SCALED[kmindelay],SCALED[kmaxdelay]);

		E.SetParams(SCALED[kattack],SCALED[krelease]);

		isDirty = false;
	}

	float	*out1	= outputs[0];
	float	*out2	= outputs[1];
	float	*in1	= inputs[0];
	bool	murder	= SCALED[kmurder] > 0.5f;
	short	mask	= (short)SCALED[kbits];
	bool	b_and		= SCALED[kand] > 0.5f;
	bool	b_xor		= SCALED[kxor] > 0.5f;
	bool	power	= SCALED[kpower] > 0.5f;
	float	tmp1	= (1.f - SCALED[kdrywet]);
	float	tmp2	= SCALED[kdrywet]*SCALED[kamp];
	float	tmp4	= (1.f-SCALED[kfeed1]);
	float	tmp3	= SCALED[klimiter] * 0.5f;

	for(long i=0;i<sampleFrames;i++)
	{
		float bitout;

		if(murder)
			bitout = process1(in1[i],mask,b_xor,b_and,power);
		else
			bitout = in1[i];

		ivan_output = F.GetVal(Delay.GetVal(LP.GetVal(SCALED[kfeed1]*ivan_output + tmp4*bitout),S.GetVal(TriToSine(S2.GetVal(SCALED[kvibfreq]))*SCALED[kdvibfreq])));

		float env_out = E.GetVal(ivan_output);

		if(env_out < 1.f)
			ivan_output *= 1.f - tmp3 * env_out;
		else
			ivan_output *= (1.f - tmp3 * 2.f) + tmp3 / env_out;

		out1[i] = in1[i]*tmp1 + ivan_output*tmp2;
		out2[i] = in1[i]*tmp1 + ivan_output*tmp2;
	}

	if(fabs(ivan_output) < 1e-10 || fabs(ivan_output) > 1e10)
		ivan_output = 0.f;
}
