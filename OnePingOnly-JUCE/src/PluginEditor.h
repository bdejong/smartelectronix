#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

class OnePingOnlyEditor : public juce::AudioProcessorEditor,
                          private juce::Timer
{
public:
    OnePingOnlyEditor(OnePingOnlyProcessor&);
    ~OnePingOnlyEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    
private:
    void timerCallback() override;
    void updateNoteParameterControls();
    juce::String createVoiceParamID(int voiceIndex, int paramType);
    
    OnePingOnlyProcessor& processorRef;
    
    // UI components
    juce::Label titleLabel;
    
    // Global parameters section
    juce::ComboBox programSelector;
    juce::Slider delayTimeSlider;
    juce::Slider feedbackSlider;
    juce::Slider masterVolumeSlider;
    
    // Note parameters section
    juce::ComboBox noteSelector;
    juce::Slider frequencySlider;
    juce::Slider durationSlider;
    juce::Slider amplitudeSlider;
    juce::Slider balanceSlider;
    juce::Slider distortionSlider;
    juce::Slider noiseSlider;
    
    // Current selected MIDI note - starts at note 60 (middle C) by default
    int currentNoteIndex = 60;
    
    // Global parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> programAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterVolumeAttachment;
    
    // Note parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> frequencyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> durationAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amplitudeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> balanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> distortionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseAttachment;
    
    // Label components
    juce::Label globalParamsLabel;
    juce::Label noteParamsLabel;
    juce::Label programLabel;
    juce::Label delayTimeLabel;
    juce::Label feedbackLabel;
    juce::Label masterVolumeLabel;
    juce::Label noteSelectorLabel;
    juce::Label frequencyLabel;
    juce::Label durationLabel;
    juce::Label amplitudeLabel;
    juce::Label balanceLabel;
    juce::Label distortionLabel;
    juce::Label noiseLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OnePingOnlyEditor)
};