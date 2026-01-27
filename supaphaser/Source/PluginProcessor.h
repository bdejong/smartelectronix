#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/WavetableFPOsc.h"
#include <cmath>

#define DIST_FIX 0.8f

inline int Float2Int(double f, double min, double max)
{
    double retval = min + (max - min) * f;
    return (int)floor(retval + 0.5);
}

inline float ScaleFreq(float in, float param, float max)
{
    return (float)((exp(in * param) - 1) / (exp(param) - 1) * max);
}

const int MaxnStages = 23;
const float MaxnStagesF = 23.f;

class SupaPhaserProcessor : public juce::AudioProcessor
{
public:
    SupaPhaserProcessor();
    ~SupaPhaserProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "SupaPhaser"; }
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

    // Parameter IDs
    static constexpr const char* PARAM_ATTACK   = "attack";
    static constexpr const char* PARAM_RELEASE  = "release";
    static constexpr const char* PARAM_MIN_ENV  = "minEnv";
    static constexpr const char* PARAM_MAX_ENV  = "maxEnv";
    static constexpr const char* PARAM_MIXTURE  = "mixture";
    static constexpr const char* PARAM_FREQ     = "freq";
    static constexpr const char* PARAM_MIN_FREQ = "minFreq";
    static constexpr const char* PARAM_MAX_FREQ = "maxFreq";
    static constexpr const char* PARAM_EXTEND   = "extend";
    static constexpr const char* PARAM_STEREO   = "stereo";
    static constexpr const char* PARAM_STAGES   = "stages";
    static constexpr const char* PARAM_DISTORT  = "distort";
    static constexpr const char* PARAM_FEED     = "feedback";
    static constexpr const char* PARAM_DRY_WET  = "dryWet";
    static constexpr const char* PARAM_GAIN     = "gain";
    static constexpr const char* PARAM_INVERT   = "invert";

    // Display text generation
    juce::String getDisplayText(const juce::String& paramId) const;

    float gainMap(float mm) const
    {
        float db;
        mm = 100.f - mm * 100.f;
        if (mm <= 0.f) {
            db = 10.f;
        } else if (mm < 48.f) {
            db = 10.f - 5.f / 12.f * mm;
        } else if (mm < 84.f) {
            db = -10.f - 10.f / 12.f * (mm - 48.f);
        } else if (mm < 96.f) {
            db = -40.f - 20.f / 12.f * (mm - 84.f);
        } else if (mm < 100.f) {
            db = -60.f - 35.f * (mm - 96.f);
        } else db = -200.f;

        return powf(10, (db / 20.f));
    }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP state
    float y1[MaxnStages];
    float in_11[MaxnStages];
    float y2[MaxnStages];
    float in_12[MaxnStages];

    float prevOutL = 0.f, prevOutR = 0.f;
    float ENV1 = 0.f, ENV2 = 0.f;

    CWavetableFPOsc Osc1, Osc2;

    std::vector<float> Noise;

    // Atomic parameter pointers
    std::atomic<float>* attackParam   = nullptr;
    std::atomic<float>* releaseParam  = nullptr;
    std::atomic<float>* minEnvParam   = nullptr;
    std::atomic<float>* maxEnvParam   = nullptr;
    std::atomic<float>* mixtureParam  = nullptr;
    std::atomic<float>* freqParam     = nullptr;
    std::atomic<float>* minFreqParam  = nullptr;
    std::atomic<float>* maxFreqParam  = nullptr;
    std::atomic<float>* extendParam   = nullptr;
    std::atomic<float>* stereoParam   = nullptr;
    std::atomic<float>* stagesParam   = nullptr;
    std::atomic<float>* distortParam  = nullptr;
    std::atomic<float>* feedParam     = nullptr;
    std::atomic<float>* dryWetParam   = nullptr;
    std::atomic<float>* gainParam     = nullptr;
    std::atomic<float>* invertParam   = nullptr;

    void initNoise(int bufSize);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SupaPhaserProcessor)
};
