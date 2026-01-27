#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/OnePoleLP.h"
#include "DSP/MovingDelay.h"
#include "DSP/TriOsc.h"
#include "DSP/Flanger.h"
#include "DSP/EnvFollower.h"

class CrazyIvanProcessor : public juce::AudioProcessor,
                           private juce::AsyncUpdater
{
public:
    CrazyIvanProcessor();
    ~CrazyIvanProcessor() override;

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

    void randomise();
    void handleAsyncUpdate() override;

    // Parameter IDs
    static constexpr const char* MURDER_ID = "murder";
    static constexpr const char* BITS_ID = "bits";
    static constexpr const char* AND_ID = "andOp";
    static constexpr const char* XOR_ID = "xorOp";
    static constexpr const char* POWER_ID = "power";
    static constexpr const char* FREQ1_ID = "freq1";
    static constexpr const char* DFREQ1_ID = "dfreq1";
    static constexpr const char* VIBFREQ_ID = "vibfreq";
    static constexpr const char* DVIBFREQ_ID = "dvibfreq";
    static constexpr const char* FEED1_ID = "feed1";
    static constexpr const char* FREQ2_ID = "freq2";
    static constexpr const char* MINDELAY_ID = "mindelay";
    static constexpr const char* MAXDELAY_ID = "maxdelay";
    static constexpr const char* DIST_ID = "dist";
    static constexpr const char* DAMP_ID = "damp";
    static constexpr const char* FEED2_ID = "feed2";
    static constexpr const char* LIMITER_ID = "limiter";
    static constexpr const char* ATTACK_ID = "attack";
    static constexpr const char* RELEASE_ID = "release";
    static constexpr const char* AMP_ID = "amp";
    static constexpr const char* DRYWET_ID = "drywet";
    static constexpr const char* RANDOMISE_ID = "randomise";

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP objects
    COnePoleLP LP;
    CMovingDelay Delay;
    CTriOsc S, S2;
    CFlanger F;
    EnvFollower E;

    float ivanOutput = 0.0f;
    float currentSampleRate = 44100.0f;

    // Parameter pointers
    std::atomic<float>* murderParam = nullptr;
    std::atomic<float>* bitsParam = nullptr;
    std::atomic<float>* andParam = nullptr;
    std::atomic<float>* xorParam = nullptr;
    std::atomic<float>* powerParam = nullptr;
    std::atomic<float>* freq1Param = nullptr;
    std::atomic<float>* dfreq1Param = nullptr;
    std::atomic<float>* vibfreqParam = nullptr;
    std::atomic<float>* dvibfreqParam = nullptr;
    std::atomic<float>* feed1Param = nullptr;
    std::atomic<float>* freq2Param = nullptr;
    std::atomic<float>* mindelayParam = nullptr;
    std::atomic<float>* maxdelayParam = nullptr;
    std::atomic<float>* distParam = nullptr;
    std::atomic<float>* dampParam = nullptr;
    std::atomic<float>* feed2Param = nullptr;
    std::atomic<float>* limiterParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* ampParam = nullptr;
    std::atomic<float>* drywetParam = nullptr;
    std::atomic<float>* randomiseParam = nullptr;

    bool lastRandomiseState = false;
    std::atomic<bool> pendingRandomise { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CrazyIvanProcessor)
};
