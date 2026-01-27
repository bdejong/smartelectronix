#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>

// Constants from original madshifta.hpp
static constexpr float p4 = 1e-24f;
static constexpr int kNumChannels = 2;
static constexpr long kFadeSize = 100001;
static constexpr long kBufferSize = 65537;
static constexpr int kTuneMin = -24;
static constexpr int kTuneMax = 24;
static constexpr int kNumMidi = 128;

// CC mappings
static constexpr int kCC_Tune = 68;
static constexpr int kCC_Fine = 69;
static constexpr int kCC_Length = 70;
static constexpr int kCC_Reso = 71;
static constexpr int kCC_Feedback = 72;
static constexpr int kCC_DryWet = 73;
static constexpr int kCC_Cutoff = 74;
static constexpr int kCC_OutVol = 75;

// Scaling macros preserved from original
inline int tuneScaled(float value) { return static_cast<int>(round(value * (kTuneMax - kTuneMin) + kTuneMin)); }
inline float fineTuneScaled(float value) { return value * 200.0f - 100.0f; }
inline int rootKeyScaled(float value) { return static_cast<int>(round(value * (kNumMidi - (kTuneMax - kTuneMin)))) - kTuneMin; }

inline double powerof2(double a) { return exp(a * 0.69314718056); }

inline unsigned int n_larger(unsigned int number)
{
    unsigned int numb = 1;
    while (numb < number) numb = numb << 1;
    return numb;
}

static const char* const midiNoteNames[128] = {
    "C-1","C#-1","D-1","D#-1","E-1","F-1","F#-1","G-1","G#-1","A-1","A#-1","B-1",
    "C 0","C#0","D 0","D#0","E 0","F 0","F#0","G 0","G#0","A 0","A#0","B 0",
    "C 1","C#1","D 1","D#1","E 1","F 1","F#1","G 1","G#1","A 1","A#1","B 1",
    "C 2","C#2","D 2","D#2","E 2","F 2","F#2","G 2","G#2","A 2","A#2","B 2",
    "C 3","C#3","D 3","D#3","E 3","F 3","F#3","G 3","G#3","A 3","A#3","B 3",
    "C 4","C#4","D 4","D#4","E 4","F 4","F#4","G 4","G#4","A 4","A#4","B 4",
    "C 5","C#5","D 5","D#5","E 5","F 5","F#5","G 5","G#5","A 5","A#5","B 5",
    "C 6","C#6","D 6","D#6","E 6","F 6","F#6","G 6","G#6","A 6","A#6","B 6",
    "C 7","C#7","D 7","D#7","E 7","F 7","F#7","G 7","G#7","A 7","A#7","B 7",
    "C 8","C#8","D 8","D#8","E 8","F 8","F#8","G 8","G#8","A 8","A#8","B 8",
    "C 9","C#9","D 9","D#9","E 9","F 9","F#9","G 9"
};

class MadshiftaProcessor : public juce::AudioProcessor
{
public:
    MadshiftaProcessor();
    ~MadshiftaProcessor() override;

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
    static constexpr const char* TUNE_ID = "tune";
    static constexpr const char* FINE_ID = "fine";
    static constexpr const char* DELAY_LEN_ID = "delayLen";
    static constexpr const char* DELAY_FB_ID = "delayFB";
    static constexpr const char* CUTOFF_ID = "cutoff";
    static constexpr const char* RESONANCE_ID = "resonance";
    static constexpr const char* FTYPE_ID = "fType";
    static constexpr const char* OUT_VOL_ID = "outVol";
    static constexpr const char* DRY_WET_ID = "dryWet";
    static constexpr const char* ROOT_ID = "root";
    static constexpr const char* MMODE_ID = "mMode";

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Atomic parameter pointers
    std::atomic<float>* tuneParam = nullptr;
    std::atomic<float>* fineParam = nullptr;
    std::atomic<float>* delayLenParam = nullptr;
    std::atomic<float>* delayFBParam = nullptr;
    std::atomic<float>* cutoffParam = nullptr;
    std::atomic<float>* resonanceParam = nullptr;
    std::atomic<float>* fTypeParam = nullptr;
    std::atomic<float>* outVolParam = nullptr;
    std::atomic<float>* dryWetParam = nullptr;
    std::atomic<float>* rootParam = nullptr;
    std::atomic<float>* mModeParam = nullptr;

    // DSP state — preserved from original madshifta.cpp
    float* fade = nullptr;
    float** delay = nullptr;
    float** buffer = nullptr;

    float last[kNumChannels] = {};
    float old1[kNumChannels] = {};
    float old2[kNumChannels] = {};

    // Current parameter values used in DSP
    float fTune = 0.5f, fFine = 0.5f, fDelayLen = 0.0f, fDelayFB = 0.0f;
    float fCutoff = 1.0f, fResonance = 0.0f, fFType = 0.0f;
    float fOutVol = 1.0f, fDryWet = 1.0f, fRoot = 0.36f, fMMode = 0.0f;

    int semitones = 0;
    float cut = 1.0f, reso = 0.0f;
    unsigned long p1 = 0, p2 = 0;
    unsigned long delay_in = 0, delay_out = 0;
    unsigned long inp = 0;
    unsigned long dp = 0;
    unsigned long nBuffer = 0, nSize = 0;
    unsigned long displace = 0, nDelay = 1;

    // MIDI state
    int notecount = 0;
    int oldtune = 0; // stores courseTune (integer semitones) before MIDI took over

    // Recalculate dp from integer semitones + fine tune
    void recalcTuneFromSemitones(int semis);

public:
    // Exposed for UI: last MIDI note played (-1 = none active)
    std::atomic<int> lastMidiNote { -1 };
    // Exposed for UI: current tune display value (semitones as integer)
    std::atomic<int> currentTuneDisplay { 0 };
private:

    // DSP methods — preserved from original
    float DoFilter(float i, float cutoff, float res, unsigned char ch);
    void DoProcess(float* i);

    // Helper to recalculate pitch shift
    void recalcTune();
    // Helper to recalculate delay
    void recalcDelay(float value);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MadshiftaProcessor)
};
