#pragma once

#include <string.h>

#include "public.sdk/source/vst2.x/audioeffectx.h"

enum
{
    kBits = 15,
    kAnd = 15,
    kOr,
    kSig,
    kNumParams,
    kNumOutputs = 1
};

class BitMurderer : public AudioEffectX
{
public:
    BitMurderer(audioMasterCallback audioMaster);
    ~BitMurderer();

    virtual void processReplacing(float** inputs, float** outputs, VstInt32 sampleframes) override;

    virtual void setProgram(VstInt32 program) override;
    virtual void setProgramName(char *name) override;
    virtual void getProgramName(char *name) override;
    virtual void setParameter(VstInt32 index, float value) override;
    virtual float getParameter(VstInt32 index) override;
    virtual void getParameterLabel(VstInt32 index, char *label) override;
    virtual void getParameterDisplay(VstInt32 index, char *text) override;
    virtual void getParameterName(VstInt32 index, char *text) override;
    virtual void setSampleRate(float sampleRate) override;

    virtual bool getOutputProperties(VstInt32 index, VstPinProperties* properties) override;
    virtual bool getEffectName(char* name) override;
    virtual bool getVendorString(char* text) override;
    virtual bool getProductString(char* text) override;
    virtual VstInt32 getVendorVersion() override { return 1; }
    virtual VstInt32 canDo(char* text) override;

    float SAVE[kNumParams];
};
