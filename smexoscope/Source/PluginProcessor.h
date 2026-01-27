#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <cmath>

#define MAX_FLOAT 150000.f
#define OSC_WIDTH 627
#define OSC_HEIGHT 269

inline float clip(float x, float max = 1.f)
{
    if (x > max) return max;
    if (x < -max) return -max;
    return x;
}

struct FFXTimeInfo
{
    bool isValid = false;
    double samplePos = 0.0;
    double sampleRate = 44100.0;

    bool tempoIsValid = false;
    double tempo = 120.0;
    double tempoBPS = 2.0;
    double numSamplesInBeat = 22050.0;

    bool ppqPosIsValid = false;
    double ppqPos = 0.0;

    bool barsIsValid = false;
    double barStartPos = 0.0;

    bool timeSigIsValid = false;
    long timeSigNumerator = 4;
    long timeSigDenominator = 4;

    bool samplesToNextBarIsValid = false;
    double numSamplesToNextBar = 0.0;

    bool cyclePosIsValid = false;
    double cycleStartPos = 0.0;
    double cycleEndPos = 0.0;

    bool playbackChanged = false;
    bool playbackIsOccuring = false;
    bool cycleIsActive = false;
};

struct CPoint
{
    double x = 0.0;
    double y = 0.0;
};

class SmexoscopeProcessor : public juce::AudioProcessor
{
public:
    enum ParamIndex {
        kTriggerSpeed,
        kTriggerType,
        kTriggerLevel,
        kTriggerLimit,
        kTimeWindow,
        kAmpWindow,
        kSyncDraw,
        kChannel,
        kFreeze,
        kDCKill,
        kNumParams
    };

    enum TriggerType {
        kTriggerFree = 0,
        kTriggerTempo,
        kTriggerRising,
        kTriggerFalling,
        kTriggerInternal,
        kNumTriggerTypes
    };

    SmexoscopeProcessor();
    ~SmexoscopeProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Smexoscope"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    static constexpr const char* PARAM_TRIGGER_SPEED = "triggerSpeed";
    static constexpr const char* PARAM_TRIGGER_TYPE  = "triggerType";
    static constexpr const char* PARAM_TRIGGER_LEVEL = "triggerLevel";
    static constexpr const char* PARAM_TRIGGER_LIMIT = "triggerLimit";
    static constexpr const char* PARAM_TIME_WINDOW   = "timeWindow";
    static constexpr const char* PARAM_AMP_WINDOW    = "ampWindow";
    static constexpr const char* PARAM_SYNC_DRAW     = "syncDraw";
    static constexpr const char* PARAM_CHANNEL       = "channel";
    static constexpr const char* PARAM_FREEZE        = "freeze";
    static constexpr const char* PARAM_DC_KILL       = "dcKill";

    const std::vector<CPoint>& getPeaks() const { return peaks; }
    const std::vector<CPoint>& getCopy() const { return copy; }

    static double timeKnobToBeats(double x);
    static double tempoBeatsForKnob(double x);
    static juce::String tempoDisplayForKnob(double x);
    juce::String getDisplayText(const juce::String& paramId) const;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void resetState();
    void processSub(const float* samples, int sampleFrames);
    void convertTimeInfo(FFXTimeInfo* ffxtime);

    std::vector<CPoint> peaks;
    std::vector<CPoint> copy;

    unsigned long index = 0;
    double counter = 1.0;
    double lastTriggerSamples = 0.0;
    float maxPeak = -MAX_FLOAT;
    float minPeak = MAX_FLOAT;
    bool lastIsMax = false;
    float previousSample = 0.f;
    double triggerPhase = 0.0;
    long triggerLimitPhase = 0;
    double dcKill_ = 0.0;
    double dcFilterTemp_ = 0.0;

    std::atomic<float>* triggerSpeedParam = nullptr;
    std::atomic<float>* triggerTypeParam  = nullptr;
    std::atomic<float>* triggerLevelParam = nullptr;
    std::atomic<float>* triggerLimitParam = nullptr;
    std::atomic<float>* timeWindowParam   = nullptr;
    std::atomic<float>* ampWindowParam    = nullptr;
    std::atomic<float>* syncDrawParam     = nullptr;
    std::atomic<float>* channelParam      = nullptr;
    std::atomic<float>* freezeParam       = nullptr;
    std::atomic<float>* dcKillParam       = nullptr;

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SmexoscopeProcessor)
};
