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
    
    OnePingOnlyProcessor& processorRef;
    
    // UI components
    juce::ComboBox programSelector;
    juce::Slider delayTimeSlider;
    juce::Slider feedbackSlider;
    juce::Slider masterVolumeSlider;
    juce::Label titleLabel;
    
    // Attachments to plugin parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> programAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterVolumeAttachment;
    
    // Labels
    juce::Label delayTimeLabel;
    juce::Label feedbackLabel;
    juce::Label masterVolumeLabel;
    juce::Label programLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OnePingOnlyEditor)
};