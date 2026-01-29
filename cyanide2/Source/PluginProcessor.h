#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/Shaper.h"
#include "DSP/ButterXOver.h"
#include "DSP/Polyphase.h"
#include "DSP/Defines.h"
#include <array>

struct Cyanide2Program
{
    juce::String name;
    std::array<float, 8> params {}; // preGain, preFilter, postGain, postFilter, preType, postType, dryWet, overSample
    std::vector<SplinePoint> points;
};

class Cyanide2Processor : public juce::AudioProcessor
{
public:
    Cyanide2Processor();
    ~Cyanide2Processor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Cyanide2"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    int getNumPrograms() override { return 16; }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    CShaper& getShaper() { return shaper; }

    juce::AudioProcessorValueTreeState apvts;

    // Parameter IDs
    static constexpr const char* PARAM_PREGAIN    = "preGain";
    static constexpr const char* PARAM_PREFILTER  = "preFilter";
    static constexpr const char* PARAM_POSTGAIN   = "postGain";
    static constexpr const char* PARAM_POSTFILTER = "postFilter";
    static constexpr const char* PARAM_PRETYPE    = "preType";
    static constexpr const char* PARAM_POSTTYPE   = "postType";
    static constexpr const char* PARAM_DRYWET     = "dryWet";
    static constexpr const char* PARAM_OVERSAMPLE = "overSample";

    // Display text generation
    juce::String getDisplayText(const juce::String& paramId) const;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void loadFactoryPresets();
    void saveCurrentToProgram(int index);
    void loadProgramToState(int index);

    CShaper shaper;
    CButterXOver F1L, F2L, F1R, F2R;
    std::array<CPolyphase, 8> P1;
    std::array<CPolyphase, 8> P2;

    std::array<float, FRAME_SIZE * 16> buffer1 {};
    std::array<float, FRAME_SIZE * 16> buffer2 {};
    std::array<float, FRAME_SIZE> noise {};
    bool previousOversample = false;

    int currentProgram = 0;
    std::array<Cyanide2Program, 16> programs;

    // Atomic parameter pointers
    std::atomic<float>* preGainParam    = nullptr;
    std::atomic<float>* preFilterParam  = nullptr;
    std::atomic<float>* postGainParam   = nullptr;
    std::atomic<float>* postFilterParam = nullptr;
    std::atomic<float>* preTypeParam    = nullptr;
    std::atomic<float>* postTypeParam   = nullptr;
    std::atomic<float>* dryWetParam     = nullptr;
    std::atomic<float>* overSampleParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Cyanide2Processor)
};
