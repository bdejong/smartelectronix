#pragma once

#include "public.sdk/source/vst2.x/audioeffectx.h"

#include "Defines.h"
#include "vstgui.h"

class FFXTimeInfo
{
public:
    FFXTimeInfo() {};
    ~FFXTimeInfo() {};

    // false if the VstTimeInfo pointer returned is null
    // i.e. the host doesn't support VstTimeInfo
    bool isValid;

    // always valid
    double samplePos;        // current location in samples
    double sampleRate;

    bool tempoIsValid;       // kVstTempoValid
    double tempo;            // in beats/minute
    double tempoBPS;         // in beats/second
    double numSamplesInBeat; // number of samples in 1 beat

    bool ppqPosIsValid;      // kVstPpqPosValid
    double ppqPos;           // 1 ppq = 1 MIDI beat (primary note division value)

    bool barsIsValid;        // kVstBarsValid
    double barStartPos;      // last bar start position, in ppq, relative to ppqPos

    bool timeSigIsValid;     // kVstTimeSigValid
    long timeSigNumerator;   // time signature
    long timeSigDenominator;

    bool samplesToNextBarIsValid;
    double numSamplesToNextBar;

    bool cyclePosIsValid;    // kVstCyclePosValid
    double cycleStartPos;    // in terms of ppq
    double cycleEndPos;      // in terms of ppq

    bool playbackChanged;    // kVstTransportChanged
    bool playbackIsOccuring; // kVstTransportPlaying
    bool cycleIsActive;      // kVstTransportCycleActive
};

class CSmartelectronixDisplay : public AudioEffectX {
public:
    /*
trigger type : free / rising / falling / internal / external   (this order)
channel : left / right
all else : on / off
*/

    // VST parameters
    enum {
        kTriggerSpeed, // internal trigger speed, knob
        kTriggerType, // trigger type, selection
        kTriggerLevel, // trigger level, slider
        kTriggerLimit, // retrigger threshold, knob
        kTimeWindow, // X-range, knob
        kAmpWindow, // Y-range, knob
        kSyncDraw, // sync redraw, on/off
        kChannel, // channel selection, left/right
        kFreeze, // freeze display, on/off
        kDCKill, // kill DC, on/off
        kNumParams
    };

    // VST elements
    enum {
        kNumPrograms = 0,
        kNumInputChannels = 2,
        kNumOutputChannels = 2, // VST doesn't like 0 output channels ;-)
    };

    // trigger types
    enum {
        kTriggerFree = 0,
        kTriggerTempo,
        kTriggerRising,
        kTriggerFalling,
        kTriggerInternal,
        kNumTriggerTypes
    };

    CSmartelectronixDisplay(audioMasterCallback audioMaster);
    ~CSmartelectronixDisplay();

    virtual void process(float** inputs, float** outputs, VstInt32 sampleFrames);
    virtual void processReplacing(float** inputs, float** outputs,
        VstInt32 sampleFrames);

    virtual void setParameter(VstInt32 index, float value);
    virtual float getParameter(VstInt32 index);
    virtual void getParameterLabel(VstInt32 index, char* label);
    virtual void getParameterDisplay(VstInt32 index, char* text);
    virtual void getParameterName(VstInt32 index, char* text);

    void getDisplay(VstInt32 index, char* text);

    virtual VstInt32 canDo(char* text);
    virtual bool getEffectName(char* name);
    virtual bool getVendorString(char* text);
    virtual bool getProductString(char* text);

    virtual void suspend();
    virtual void resume();

    // maps time knob to beats for tempo matching
    static double timeKnobToBeats(double x);

    const std::vector<CPoint>& getPeaks() const { return peaks; }
    const std::vector<CPoint>& getCopy() const { return copy; }

protected:
    std::vector<CPoint> peaks;
    std::vector<CPoint> copy;

    // a convenience wrapper for getTimeInfo()
    void convertVstTimeInfo(FFXTimeInfo *ffxtime);

    // the actual algo :-)
    void processSub(float** inputs, long sampleFrames);

    // index into the peak-array
    unsigned long index;

    // counter which is used to set the amount of samples / pixel
    double counter;

    // used to calculate the time interval from last trigger
    double lastTriggerSamples;

    // max/min peak in this block
    float max, min, maxR, minR;

    // the last peak we encountered was a maximum!
    bool lastIsMax;

    // the previous sample (for edge-triggers)
    float previousSample;

    // the internal trigger oscillator
    double triggerPhase;

    // stupid VST parameter save
    float SAVE[kNumParams];

    // trigger limiter!
    long triggerLimitPhase;

    // dc killer
    double dcKill, dcFilterTemp;
};
