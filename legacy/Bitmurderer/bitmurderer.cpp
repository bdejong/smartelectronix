#include "bitmurderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

extern bool oome;

#include "MurderEditor.h"

short createmask(float *bits)
{
    unsigned short tmp = 0;
    for (char i = 0; i < 15; i++)
        tmp = (tmp << 1) + (bits[i] > 0.5f);
    tmp |= 0x8000;
    return *(short *)&tmp;
}

float clip(float x)
{
    if (x > 1.f)
        return 1.f;
    return x;
}

float process1(float in, short mask, bool orFlag, bool andFlag, bool power)
{
    bool sign = in < 0.f;

    in = clip(fabsf(in));

    if (power)
        in = powf(in, 1 / 3.f);

    short x = (short)(in * 32767.f);

    if (orFlag)
        x = x ^ mask;

    if (andFlag)
        x = x & mask;

    x &= 0x7fff; //reset the sign bit

    float out = (float)x / 32767.f;

    if (power)
        out = powf(out, 3.f);

    return sign ? -out : out;
}

BitMurderer::BitMurderer(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, 0, kNumParams)
{
    setNumInputs(2);
    setNumOutputs(2);
    canProcessReplacing();
    setUniqueID('BITM');

    resume();

    for (char i = 0; i < kNumParams; i++)
        SAVE[i] = 0.f;

    editor = new MurderEditor(this);
}

BitMurderer::~BitMurderer()
{
}

void BitMurderer::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames)
{
    float *out1 = outputs[0];
    float *out2 = outputs[1];
    float *in1 = inputs[0];
    float *in2 = inputs[1];

    short mask = createmask(SAVE);
    bool orFlag = SAVE[kOr] > 0.5f;
    bool andFlag = SAVE[kAnd] > 0.5f;
    bool sig = SAVE[kSig] > 0.5f;

    for (long i = 0; i < sampleFrames; i++)
    {
        out1[i] = process1(in1[i], mask, orFlag, andFlag, sig);
        out2[i] = process1(in2[i], mask, orFlag, andFlag, sig);
    }
}

void BitMurderer::getParameterLabel(VstInt32 index, char *label)
{
    strcpy(label, "");
}

void float2bool(float x, char *text)
{
    strcpy(text, x > 0.5f ? "ON" : "OFF");
}

void BitMurderer::getParameterDisplay(VstInt32 index, char *text)
{
    float2bool(SAVE[index], text);

    /*
    switch(index)
    {
        case kDist : if(SAVE[kDist] > 0.5f) strcpy(text,"ON"); else  strcpy(text,"OFF"); break;
        case kCrazy : if(SAVE[kCrazy] > 0.5f) strcpy(text,"ON"); else  strcpy(text,"OFF"); break;
        case kRand : strcpy(text,"--------"); break;
        case kTame : dB2string(SAVE[kTame],text); break;
    }*/
}

void BitMurderer::getParameterName(VstInt32 index, char *label)
{
    if (index < kBits)
        strcpy(label, "bit");
    else
        switch (index)
        {
        case kAnd:	strcpy(label, "AND"); break;
        case kOr:	strcpy(label, "OR"); break;
        case kSig:	strcpy(label, "Sigmoid"); break;
        }
}

void BitMurderer::setParameter(VstInt32 index, float value)
{
    SAVE[index] = value;

    if (editor)
        ((AEffGUIEditor*)editor)->setParameter(index, value);
}

float BitMurderer::getParameter(VstInt32 index)
{
    return SAVE[index];
}

bool BitMurderer::getOutputProperties(VstInt32 index, VstPinProperties* properties)
{
    if (index < kNumOutputs)
    {
        properties->flags = kVstPinIsActive;
        return true;
    }
    return false;
}

bool BitMurderer::getEffectName(char* name)
{
    strcpy(name, "Bit Murderer");
    return true;
}

bool BitMurderer::getVendorString(char* text)
{
    strcpy(text, "Bram @ Smartelectronix");
    return true;
}

bool BitMurderer::getProductString(char* text)
{
    strcpy(text, "Bit Murderer");
    return true;
}

void BitMurderer::setProgram(VstInt32 program)
{
}

void BitMurderer::setProgramName(char *name)
{
}

void BitMurderer::getProgramName(char *name)
{
    strcpy(name, "");
}

void BitMurderer::setSampleRate(float sampleRate)
{
    AudioEffectX::setSampleRate(sampleRate);
}

VstInt32 BitMurderer::canDo(char* text)
{
    if (!strcmp(text, "receiveVstEvents"))    return -1;
    if (!strcmp(text, "receiveVstMidiEvent")) return -1;
    return -1;	// explicitly can't do; 0 => don't know
}
