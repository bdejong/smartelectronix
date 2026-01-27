#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>
#include <cstdlib>

// Constants from original
constexpr int NUMBERIO = 2;
constexpr int MAXSIZE = 2000000;
constexpr int BITSLIDES = 7;
constexpr int MAXSLIDES = 1 << BITSLIDES;
constexpr int FADETIME = 150;

// Glitch parameters for each slice
struct GlitchParams
{
    unsigned long offset = 0;
    bool reverse = false;
    bool stop = false;
    bool silence = false;
};

// Hermite interpolation (inverse direction)
inline float hermiteInverse(float* wavetable, unsigned long nearest_sample, float x)
{
    float y3 = (nearest_sample == 0) ? 0.f : wavetable[nearest_sample - 1];
    float y2 = wavetable[nearest_sample];
    float y1 = wavetable[nearest_sample + 1];
    float y0 = wavetable[nearest_sample + 2];

    x = 1.f - x;

    float c0 = y1;
    float c1 = 0.5f * (y2 - y0);
    float c2 = y0 - 2.5f * y1 + 2.f * y2 - 0.5f * y3;
    float c3 = 1.5f * (y1 - y2) + 0.5f * (y3 - y0);

    return ((c3 * x + c2) * x + c1) * x + c0;
}

class SupaTriggaProcessor : public juce::AudioProcessor
{
public:
    SupaTriggaProcessor();
    ~SupaTriggaProcessor() override;

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

    // Parameter IDs
    static constexpr const char* GRANULARITY_ID = "granularity";
    static constexpr const char* SPEED_ID = "speed";
    static constexpr const char* PROB_REVERSE_ID = "probReverse";
    static constexpr const char* PROB_SPEED_ID = "probSpeed";
    static constexpr const char* PROB_REARRANGE_ID = "probRearrange";
    static constexpr const char* PROB_SILENCE_ID = "probSilence";
    static constexpr const char* PROB_REPEAT_ID = "probRepeat";
    static constexpr const char* INSTANT_REVERSE_ID = "instantReverse";
    static constexpr const char* INSTANT_SPEED_ID = "instantSpeed";
    static constexpr const char* INSTANT_REPEAT_ID = "instantRepeat";

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void randomize();

    // Buffers
    std::unique_ptr<float[]> leftBuffer;
    std::unique_ptr<float[]> rightBuffer;

    // Sequencer
    GlitchParams sequencer[MAXSLIDES];

    // State
    unsigned long positionInMeasure = 0;
    unsigned long previousSliceIndex = 0xffffffff;
    unsigned long granularityMask = 0;
    unsigned long granularity = 0;

    float gain = 0.0f;
    float speed = 0.0f;
    float position = 0.0f;
    bool first = true;

    bool instantReverse = false;
    bool instantSlow = false;
    bool instantRepeat = false;

    float currentSampleRate = 44100.0f;
    float fadeCoeff = 0.0f;

    // Last playback state for detecting transport changes
    bool wasPlaying = false;

    // Parameter pointers
    std::atomic<float>* granularityParam = nullptr;
    std::atomic<float>* speedParam = nullptr;
    std::atomic<float>* probReverseParam = nullptr;
    std::atomic<float>* probSpeedParam = nullptr;
    std::atomic<float>* probRearrangeParam = nullptr;
    std::atomic<float>* probSilenceParam = nullptr;
    std::atomic<float>* probRepeatParam = nullptr;
    std::atomic<float>* instantReverseParam = nullptr;
    std::atomic<float>* instantSpeedParam = nullptr;
    std::atomic<float>* instantRepeatParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SupaTriggaProcessor)
};
