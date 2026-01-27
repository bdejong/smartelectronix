#include <stdio.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "vstxsynth.h"

float sq(float x) { return x*x*x; };

inline float clip(float x)
{
    if (x > 1.f)
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

    if (power)
        in = powf(in, 1.f / 3.f);

    short x = (short)(in * 32767.f);

    if (b_xor)
        x = x ^ mask;

    if (b_and)
        x = x & mask;

    x &= 0x7fff; //reset the sign bit

    float out = (float)x / 32767.f;

    if (power)
        out = powf(out, 3.f);

    return sign ? -out : out;
}

float randbool()
{
    if (rand() > (RAND_MAX / 2))
        return 1.f;
    else
        return 0.f;
}

float randfloat(float min = 0.f, float max = 1.f)
{
    return min + (max - min) * (float)rand() / (float)RAND_MAX;
}

VstXSynth::VstXSynth(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, 0, kNumParams)
{
    setNumInputs (2);
    setNumOutputs (2);
    canProcessReplacing();
    setUniqueID ('CRZY');

    srand( (unsigned)time( NULL ) );

    resume();

    for(long i=0;i<kNumParams-1;i++)
        setParameter(i,0.f);

    randomise();

    S.SampleRateF = getSampleRate();
    S2.SampleRateF = getSampleRate();
    F.T.SampleRateF = getSampleRate();
}

VstXSynth::~VstXSynth()
{
}

void VstXSynth::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames)
{
    if (isDirty)
    {
        LP.SetParams(1.f - SCALED[kdamp]);

        float x1 = fabsf(SCALED[kfreq1] - SCALED[kdfreq1]);
        float x2 = fabsf(SCALED[kfreq1] + SCALED[kdfreq1]);

        if (x1 < 0.01f)
            x1 = 0.01f;
        if (x2 < 0.01f)
            x2 = 0.01f;

        x1 = getSampleRate() / x1;
        if (x1 > 500000.f) x1 = 500000.f;
        x2 = getSampleRate() / x2;
        if (x2 > 500000.f) x2 = 500000.f;

        S.SetParams(x1, x2);
        F.SetParams(SCALED[kfreq2], SCALED[kfeed2], SCALED[kdrywet], SCALED[kdist], SCALED[kmindelay], SCALED[kmaxdelay]);

        E.SetParams(SCALED[kattack], SCALED[krelease]);

        isDirty = false;
    }

    float *out1 = outputs[0];
    float *out2 = outputs[1];
    float *in1 = inputs[0];
    bool murder = SCALED[kmurder] > 0.5f;
    short mask = (short)SCALED[kbits];
    bool b_and = SCALED[kand] > 0.5f;
    bool b_xor = SCALED[kxor] > 0.5f;
    bool power = SCALED[kpower] > 0.5f;
    float tmp1 = (1.f - SCALED[kdrywet]);
    float tmp2 = SCALED[kdrywet] * SCALED[kamp];
    float tmp4 = (1.f - SCALED[kfeed1]);
    float tmp3 = SCALED[klimiter] * 0.5f;

    for (long i = 0; i<sampleFrames; i++)
    {
        float bitout;

        if (murder)
            bitout = process1(in1[i], mask, b_xor, b_and, power);
        else
            bitout = in1[i];

        ivan_output = F.GetVal(Delay.GetVal(LP.GetVal(SCALED[kfeed1] * ivan_output + tmp4*bitout), S.GetVal(TriToSine(S2.GetVal(SCALED[kvibfreq]))*SCALED[kdvibfreq])));

        float env_out = E.GetVal(ivan_output);

        if (env_out < 1.f)
            ivan_output *= 1.f - tmp3 * env_out;
        else
            ivan_output *= (1.f - tmp3 * 2.f) + tmp3 / env_out;

        out1[i] = in1[i] * tmp1 + ivan_output*tmp2;
        out2[i] = in1[i] * tmp1 + ivan_output*tmp2;
    }

    if (fabs(ivan_output) < 1e-10 || fabs(ivan_output) > 1e10)
        ivan_output = 0.f;
}

void VstXSynth::getProgramName(char *name)
{
    strcpy(name, "");
}

void VstXSynth::setParameter(VstInt32 index, float value)
{
    SAVE[index] = value;

    switch (index)
    {
        case kfreq1:
        case kdfreq1: 
            SCALED[kfreq1] = sq(SAVE[kfreq1]) * 300.f;
            SCALED[kdfreq1] = SCALED[kfreq1] * SAVE[kdfreq1];
            break;
        case kvibfreq: SCALED[index] = sq(SAVE[index]) * 10.f; break;
        case kdvibfreq: SCALED[index] = sq(SAVE[index]) * 10.f; break;
        case kfeed1: SCALED[index] = SAVE[index] * 0.99f; break;
        case kfeed2: SCALED[index] = SAVE[index] * 0.99f; break;
        case kmindelay: SCALED[index] = SAVE[index] * 10.f; break;
        case kmaxdelay: SCALED[index] = SAVE[index] * 15000.f; break;
        case kdist: SCALED[index] = SAVE[index] * 30.f; break;
        case kbits: SCALED[index] = SAVE[index] * 32768.f * 0.49f; break;
        case kattack:
        case krelease: SCALED[index] = SAVE[index] * 200.f; break;
        case klimiter: SCALED[index] = SAVE[index]; break;
        case kdamp:
        case kfreq2:
        case kmurder:
        case kand:
        case kxor:
        case kpower:
        case kdrywet: SCALED[index] = SAVE[index]; break;
        case kamp: SCALED[index] = (SAVE[index] * SAVE[index])*2.f; break;
        case krandomise: 
            if (SAVE[index] > 0.5f)
            {
                randomise();
                updateDisplay();
            };
            break;
    }

    isDirty = true;
}

float VstXSynth::getParameter(VstInt32 index)
{
    return SAVE[index];
}

void VstXSynth::getParameterLabel(VstInt32 index, char *label)
{
    switch(index)
    {
        case kmurder: strcpy(label, ""); break;
        case kbits:   strcpy(label, ""); break;
        case kand:    strcpy(label, ""); break;
        case kxor:    strcpy(label, ""); break;
        case kpower:  strcpy(label, ""); break;
    
        case kfreq1:    strcpy(label, "Hz"); break;
        case kdfreq1:   strcpy(label, "Hz"); break;
        case kvibfreq:  strcpy(label, "Hz"); break;
        case kdvibfreq: strcpy(label, "Hz"); break;
        case kfeed1:    strcpy(label, ""); break;
        
        case kfreq2:    strcpy(label, "Hz"); break;
        case kmindelay: strcpy(label, "samples"); break;
        case kmaxdelay: strcpy(label, "samples"); break;
        case kdist:     strcpy(label, ""); break;
        case kfeed2:    strcpy(label, ""); break;
        case kdamp:     strcpy(label, ""); break;
        
        case klimiter: strcpy(label, "amount"); break;
        case kattack:  strcpy(label, "ms"); break;
        case krelease: strcpy(label, "ms"); break;
        
        case kamp:       strcpy(label, "dB"); break;
        case kdrywet:    strcpy(label, ""); break;
        case krandomise: strcpy(label, ""); break;
    }
}

void VstXSynth::getParameterDisplay(VstInt32 index, char *text)
{
    switch(index)
    {
        case kamp: dB2string(SCALED[index],text, kVstMaxParamStrLen); break;
        case kmurder:
        case kand:
        case kxor:
        case kpower:
        case krandomise :
            if(SCALED[index] > 0.5f)
                strcpy(text,"ON");
            else
                strcpy(text,"OFF");
            break;
        default : float2string(SCALED[index],text, kVstMaxParamStrLen); break;
    }
}

void VstXSynth::getParameterName(VstInt32 index, char *label)
{
    switch(index)
    {
        case kmurder: strcpy(label, "bitmurderer"); break;
        case kbits:   strcpy(label, "  bitmask"); break;
        case kand:    strcpy(label, "  AND"); break;
        case kxor:    strcpy(label, "  XOR"); break;
        case kpower:  strcpy(label, "  power"); break;

        case kfreq1:    strcpy(label, "frequency 1"); break;
        case kdfreq1:   strcpy(label, "  diff frequency 1"); break;
        case kvibfreq:  strcpy(label, "  vib frequency"); break;
        case kdvibfreq: strcpy(label, "  diff vib frequency"); break;
        case kfeed1:    strcpy(label, "  feedback 1"); break;

        case kfreq2:    strcpy(label, "frequency 2"); break;
        case kmindelay: strcpy(label, "  min delay"); break;
        case kmaxdelay: strcpy(label, "  max delay"); break;
        case kdist:     strcpy(label, "  distortion"); break;
        case kfeed2:    strcpy(label, "  feedback 2"); break;
        case kdamp:     strcpy(label, "  damping"); break;
        case kamp:      strcpy(label, "amplify"); break;

        case klimiter: strcpy(label, "limiter"); break;
        case kattack:  strcpy(label, "  attack"); break;
        case krelease: strcpy(label, "  release"); break;

        case kdrywet:    strcpy(label, "dry-wet"); break;
        case krandomise: strcpy(label, "randomise"); break;
    }
}

void VstXSynth::setSampleRate(float sampleRate)
{
    AudioEffectX::setSampleRate(sampleRate);

    S.SampleRateF = getSampleRate();
    S2.SampleRateF = getSampleRate();
    F.T.SampleRateF = getSampleRate();
    E.SampleRateF = getSampleRate();

    isDirty = true;
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

bool VstXSynth::getOutputProperties(VstInt32 index, VstPinProperties* properties)
{
    if(index < 2)
    {
        properties->flags = kVstPinIsActive;
        return true;
    }
    return false;
}

bool VstXSynth::getEffectName(char* name)
{
    strcpy (name, "Crazy Ivan");
    return true;
}

bool VstXSynth::getVendorString(char* text)
{
    strcpy (text, "Bram @ Smartelectronix");
    return true;
}

bool VstXSynth::getProductString(char* text)
{
    strcpy (text, "Crazy Ivan");
    return true;
}

VstInt32 VstXSynth::canDo(char* text)
{
    if (!strcmp (text, "receiveVstEvents"))
        return -1;
    if (!strcmp (text, "receiveVstMidiEvent"))
        return -1;
    return -1;    // explicitly can't do; 0 => don't know
}

void VstXSynth::randomise()
{
    setParameter(kmurder, randbool());
    setParameter(kbits, randfloat());
    setParameter(kand, randbool());
    setParameter(kxor, randbool());
    setParameter(kpower, randbool());
    setParameter(kfreq1, randfloat());
    setParameter(kdfreq1, randfloat());
    setParameter(kvibfreq, randfloat());
    setParameter(kdvibfreq, randfloat());
    setParameter(kfeed1, randfloat());
    setParameter(kfreq2, randfloat());
    setParameter(kmindelay, randfloat());
    setParameter(kmaxdelay, randfloat());
    setParameter(kdist, randfloat());
    setParameter(kfeed2, randfloat());
    setParameter(kdamp, randfloat());
    setParameter(klimiter, 0.3f);
    setParameter(kattack, randfloat());
    setParameter(krelease, randfloat());
    setParameter(kamp, 0.5f);
    setParameter(kdrywet, 1.f);
}
