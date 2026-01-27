#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/Bouncy.h"

class BouncyProcessor : public juce::AudioProcessor
{
public:
    BouncyProcessor();
    ~BouncyProcessor() override;

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

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // Parameter IDs
    static constexpr const char* MAX_DELAY_ID = "maxDelay";
    static constexpr const char* DELAY_SHAPE_ID = "delayShape";
    static constexpr const char* AMP_SHAPE_ID = "ampShape";
    static constexpr const char* RAND_AMP_ID = "randAmp";
    static constexpr const char* RENEW_RAND_ID = "renewRand";

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::unique_ptr<Bouncy> delayL;
    std::unique_ptr<Bouncy> delayR;

    // Atomic parameter values for thread-safe access
    std::atomic<float>* maxDelayParam = nullptr;
    std::atomic<float>* delayShapeParam = nullptr;
    std::atomic<float>* ampShapeParam = nullptr;
    std::atomic<float>* randAmpParam = nullptr;
    std::atomic<float>* renewRandParam = nullptr;

    float currentBPM = 120.0f;
    bool isDirty = true;
    bool lastRenewRandState = false;

    static constexpr float NUM_BEATS = 16.f;
    static constexpr float MAX_DELAY_SECONDS = 20.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BouncyProcessor)
};
