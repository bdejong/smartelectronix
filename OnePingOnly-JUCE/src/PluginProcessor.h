#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Delay.h"

#define NUM_VOICES 128
#define NUM_PARAMS_PER_VOICE 6
#define NUM_GLOBAL_PARAMS 3

// Parameter indices per voice
enum VoiceParams
{
    kFreq = 0,      // Frequency/pitch
    kDuration,      // Duration of ping
    kAmp,           // Amplitude
    kBalance,       // Balance (panning)
    kNoise,         // Noise amount
    kDistortion     // Distortion amount
};

// Global parameter indices
enum GlobalParams
{
    kFeedback = 0,  // Delay feedback
    kDelayTime,     // Delay time
    kMasterVolume   // Master volume
};

// Voice structure for holding voice-specific data
#define MAX_MIDI_NOTES 200

// Holds MIDI note data for processing, mimicking NoteData from original
struct MidiNoteData
{
    int note;       // MIDI note number
    float velocity; // Note velocity (0-1)
    int deltaFrames;// Sample offset in the buffer
};

struct PingVoice
{
    bool active = false;
    int note = -1;
    float amplitude = 0.0f;
    float balance = 0.5f;
    float noiseAmount = 0.0f;
    float distortionAmount = 0.0f;
    float alpha = 1.9701f;
    float beta = 0.99f;
    float gamma = -0.99f;
    float d1 = 0.0f;
    float d2 = 0.0f;
    float gain = 0.4f;  // Gain factor used in original (g[i])
    float in = 0.0f;
    float in_1 = 0.0f;
    float in_2 = 0.0f;
    float out_1 = 0.0f;
    float out_2 = 0.0f;
};

class OnePingOnlyProcessor : public juce::AudioProcessor,
                             private juce::ValueTree::Listener
{
public:
    // A public value to track the currently selected note in the UI
    int currentUISelectedNote = 60;
    
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

    // Parameter management
    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }
    
    // Make these public so the editor can directly call them when UI controls change
    void updateVoiceFromParameters(int voiceIndex);
    void updateAllVoicesFromParameters();
    
    // Method to manually set a voice's active state (for UI-driven updates)
    void setVoiceActiveState(int voiceIndex, bool active)
    {
        if (voiceIndex >= 0 && voiceIndex < NUM_VOICES)
        {
            voices[static_cast<size_t>(voiceIndex)].active = active;
            DBG("Voice " + juce::String(voiceIndex) + " active state set to: " + juce::String(active));
        }
    }

private:
    // Voice management
    void handleMidiEvent(const juce::MidiMessage& midiMessage);
    void noteOn(int midiNoteNumber, float velocity);
    void noteOff(int midiNoteNumber, float velocity);
    
    // Parameter handling
    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;
    
    // Audio processor state
    juce::AudioProcessorValueTreeState parameters;
    std::array<PingVoice, NUM_VOICES> voices;
    
    // Lookup table for MIDI note frequencies
    std::array<double, 128> midiNoteFrequencies;
    
    // Global parameters
    float delayTime;
    float feedback;
    float masterVolume;
    
    // Delay effects
    std::unique_ptr<Delay> delayLeft;
    std::unique_ptr<Delay> delayRight;
    
    // Current sample rate
    double currentSampleRate;
    
    // Store MIDI notes for processing
    std::vector<MidiNoteData> midiNotes;
    
    // Program management
    int currentProgram;
    std::array<juce::String, 12> programNames;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OnePingOnlyProcessor)
};