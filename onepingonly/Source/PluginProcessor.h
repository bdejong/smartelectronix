#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>
#include <cstdlib>
#include <random>

// Constants from original
constexpr int NUM_PINGS = 128;
constexpr int NUM_PARAMS_PER_PING = 6;
constexpr int LOW_NOTE = 0;
constexpr float MAX_DELAY_SECONDS = 3.0f;

// Parameter indices for each ping
enum PingParam
{
    kFreq = 0,
    kDuration,
    kAmp,
    kBal,
    kNoise,
    kDist
};

// Note data structure
struct NoteData
{
    int note = -1;
    int time = 0;
    float velocity = 0.0f;
};

class CDelay;

class OnePingOnlyProcessor : public juce::AudioProcessor
{
public:
    OnePingOnlyProcessor();
    ~OnePingOnlyProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Parameter ID helpers
    static juce::String getFreqParamID(int pingIndex) { return "freq" + juce::String(pingIndex + 1); }
    static juce::String getDurationParamID(int pingIndex) { return "dur" + juce::String(pingIndex + 1); }
    static juce::String getAmpParamID(int pingIndex) { return "amp" + juce::String(pingIndex + 1); }
    static juce::String getBalParamID(int pingIndex) { return "bal" + juce::String(pingIndex + 1); }
    static juce::String getNoiseParamID(int pingIndex) { return "noise" + juce::String(pingIndex + 1); }
    static juce::String getDistParamID(int pingIndex) { return "dist" + juce::String(pingIndex + 1); }

    static constexpr const char* DELAY_ID = "delay";
    static constexpr const char* FEED_ID = "feed";
    static constexpr const char* MASTER_ID = "master";

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP parameter update methods
    void setFreq(int index, float freq);
    void setDuration(int index, float duration);
    void setAmp(int index, float amp);
    void setBalance(int index, float balance);
    void setNoise(int index, float noise);
    void setDistortion(int index, float dist);
    void setDelay(float delay);
    void setFeed(float feed);

    void clearFilterStates();

    // Delays
    std::unique_ptr<CDelay> delayL;
    std::unique_ptr<CDelay> delayR;

    // Note buffer
    NoteData notes[200];
    int numNotes = 0;

    // Per-ping DSP state
    float amp[NUM_PINGS] = {};
    float bal[NUM_PINGS] = {};
    float baltmp[NUM_PINGS] = {};
    float noiseAmount[NUM_PINGS] = {};
    float d1[NUM_PINGS] = {};
    float d2[NUM_PINGS] = {};

    // Filter state
    float alpha[NUM_PINGS] = {};
    float beta[NUM_PINGS] = {};
    float gamma[NUM_PINGS] = {};
    float g[NUM_PINGS] = {};
    float in[NUM_PINGS] = {};
    float in_1[NUM_PINGS] = {};
    float in_2[NUM_PINGS] = {};
    float out_1[NUM_PINGS] = {};
    float out_2[NUM_PINGS] = {};

    // Parameter values (raw 0-1)
    float fFreq[NUM_PINGS] = {};
    float fDuration[NUM_PINGS] = {};
    float fAmp[NUM_PINGS] = {};
    float fBal[NUM_PINGS] = {};
    float fNoise[NUM_PINGS] = {};
    float fDist[NUM_PINGS] = {};

    float fDelay = 0.25f;
    float fFeed = 0.6f;
    float fMaster = 1.0f;

    float currentSampleRate = 44100.0f;

    // Random number generator
    std::mt19937 rng;
    std::uniform_real_distribution<float> noiseDist{-1.0f, 1.0f};

    // Parameter pointers for globals
    std::atomic<float>* delayParam = nullptr;
    std::atomic<float>* feedParam = nullptr;
    std::atomic<float>* masterParam = nullptr;

    // Parameter pointers for per-ping (stored in arrays)
    std::atomic<float>* freqParams[NUM_PINGS] = {};
    std::atomic<float>* durationParams[NUM_PINGS] = {};
    std::atomic<float>* ampParams[NUM_PINGS] = {};
    std::atomic<float>* balParams[NUM_PINGS] = {};
    std::atomic<float>* noiseParams[NUM_PINGS] = {};
    std::atomic<float>* distParams[NUM_PINGS] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OnePingOnlyProcessor)
};
