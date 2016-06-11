#pragma once

#include "public.sdk/source/vst2.x/audioeffectx.h"
#include <string.h>

class H2Oeffect;
class CCompressor;

class H2Oprogram {
    H2Oprogram();
    ~H2Oprogram();
    friend class H2Oeffect;

private:
    char name[24];

    float fPreAmp, fAttack, fRelease, fAmount, fPostAmp, fSaturate;
};

enum {
    kNumPrograms = 2,
    kNumOutputs = 2,
    kNumInputs = 2,

    kPreAmp = 0,
    kAttack,
    kRelease,
    kAmount,
    kPostAmp,
    kSaturate,

    kNumParams
};

class H2Oeffect : public AudioEffectX {
    friend class H2Oprogram;

public:
    H2Oeffect(audioMasterCallback audioMaster);
    virtual ~H2Oeffect();

    virtual void process(float** inputs, float** outputs, VstInt32 sampleFrames);
    virtual void processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames);

    virtual void resume();
    virtual void suspend();

    virtual void setProgramName(char* name);
    virtual void getProgramName(char* name);
    virtual void setProgram(VstInt32 program);
    virtual void setParameter(VstInt32 index, float value);
    virtual float getParameter(VstInt32 index);
    virtual void getParameterLabel(VstInt32 index, char* label);
    virtual void getParameterDisplay(VstInt32 index, char* text);
    virtual void getParameterName(VstInt32 index, char* text);
    virtual void setSampleRate(float sampleRate);

    virtual bool getOutputProperties(VstInt32 index, VstPinProperties* properties);
    virtual bool getEffectName(char* name);
    virtual bool getVendorString(char* text);
    virtual bool getProductString(char* text);
    virtual VstInt32 canDo(char* text);

private:
    H2Oprogram* programs;
    float fPreAmp, fAttack, fRelease, fAmount, fPostAmp, fSaturate;

    CCompressor* comp;
};
