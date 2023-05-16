#pragma once

#include <string.h>

#include "public.sdk/source/vst2.x/audioeffectx.h"

#include "OnePoleLP.h"
#include "MovingDelay.h"
#include "TriOsc.h"
#include "Flanger.h"
#include "EnvFollower.h"

enum
{
    kmurder,
        kbits,
        kand,
        kxor,
        kpower,
    kfreq1,
        kdfreq1,
        kvibfreq,
        kdvibfreq,
        kfeed1,
    kfreq2,
        kmindelay,
        kmaxdelay,
        kdist,
        kdamp,
        kfeed2,
    klimiter,
        kattack,
        krelease,
    kamp,
    kdrywet,
    krandomise, //always keep this 1 last
    kNumParams
};


class VstXSynth : public AudioEffectX
{
public:
    VstXSynth(audioMasterCallback audioMaster);
    ~VstXSynth();

    virtual void processReplacing(float **inputs, float **outputs, VstInt32 sampleframes) override;

    virtual void setProgram(VstInt32 program) override {};
    virtual void setProgramName(char *name) override {};
    virtual void getProgramName(char *name) override;
    virtual void setParameter(VstInt32 index, float value) override;
    virtual float getParameter(VstInt32 index) override;
    virtual void getParameterLabel(VstInt32 index, char *label) override;
    virtual void getParameterDisplay(VstInt32 index, char *text) override;
    virtual void getParameterName(VstInt32 index, char *text) override;
    virtual void setSampleRate(float sampleRate) override;
    virtual void resume() override;

    virtual bool getOutputProperties(VstInt32 index, VstPinProperties* properties) override;
    virtual bool getEffectName(char* name) override;
    virtual bool getVendorString(char* text) override;
    virtual bool getProductString(char* text) override;
    virtual VstInt32 getVendorVersion() override {return 1;}
    virtual VstInt32 canDo (char* text) override;

    void randomise();

    short mask;
    float ivan_output;

    COnePoleLP LP;
    CMovingDelay Delay;
    CTriOsc S,S2;
    CFlanger F;
    EnvFollower E;

    bool isDirty;

    float SAVE[kNumParams];
    float SCALED[kNumParams];
};
