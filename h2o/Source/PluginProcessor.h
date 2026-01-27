#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/Compressor.h"

class H2OProcessor : public juce::AudioProcessor
{
public:
    H2OProcessor();
    ~H2OProcessor() override;

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
    CCompressor& getCompressor() { return *compressor; }

    // Parameter IDs
    static constexpr const char* PREAMP_ID = "preamp";
    static constexpr const char* ATTACK_ID = "attack";
    static constexpr const char* RELEASE_ID = "release";
    static constexpr const char* AMOUNT_ID = "amount";
    static constexpr const char* POSTAMP_ID = "postamp";
    static constexpr const char* SATURATE_ID = "saturate";

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::unique_ptr<CCompressor> compressor;

    // Atomic parameter values for thread-safe access
    std::atomic<float>* preampParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* amountParam = nullptr;
    std::atomic<float>* postampParam = nullptr;
    std::atomic<float>* saturateParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(H2OProcessor)
};
