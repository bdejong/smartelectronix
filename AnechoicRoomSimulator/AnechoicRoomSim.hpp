#pragma once

#include "public.sdk/source/vst2.x/audioeffectx.h"

class AnechoicRoomSim : public AudioEffectX
{
public:

    enum 
    {
        kSize,
        kNumParams,
        kNumPrograms = 0,
        kNumOutputChannels = 2,
        kNumInputChannels = 2
    };

    AnechoicRoomSim(audioMasterCallback audioMaster);
    ~AnechoicRoomSim();

    virtual void processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames) override;

    //virtual void setProgramName(char* name) override;
    //virtual void setProgram(VstInt32 index) override;
    virtual void getProgramName(char* name) override;
    virtual void setParameter(VstInt32 index, float value) override;
    virtual float getParameter(VstInt32 index) override;
    virtual void getParameterLabel(VstInt32 index, char* label) override;
    virtual void getParameterDisplay(VstInt32 index, char* text) override;
    virtual void getParameterName(VstInt32 index, char* text) override;

    virtual VstInt32 canDo(char* text) override;
    virtual bool getEffectName(char* name) override;
    virtual bool getVendorString(char* text) override;
    virtual bool getProductString(char* text) override;

    //virtual void suspend() override;
    //virtual void resume() override;
    //virtual void setSampleRate(float sampleRate) override;
    //virtual void setBlockSize(VstInt32 blockSize) override;

    void trim(char* text)
    {
        long j = 0, i = 0;
        while (text[i])
        {
            if (text[i] == ' ')
            {
                ++i;
            }
            else
            {
                text[j++] = text[i++];
            }
        }
        text[j] = 0;
    }

    void getDisplay(long index, char* text)
    {
        getParameterDisplay(index, text);
        trim(text);
        getParameterLabel(index, &text[strlen(text)]);
        strupr(text);
    }

protected:

    float savedParameters[kNumParams];

};
