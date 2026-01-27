#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class RotaryKnob : public juce::Component
{
public:
    RotaryKnob(const juce::Image& handleImage);
    ~RotaryKnob() override;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;

    void setValue(float newValue);
    float getValue() const { return value; }

    void setDefaultValue(float v) { defaultValue = v; }

    std::function<void(float)> onValueChange;

private:
    juce::Image handleImage;
    float value = 0.0f;
    float defaultValue = 0.5f;

    float dragStartValue = 0.0f;
    int dragStartY = 0;
    static constexpr float dragSensitivity = 0.005f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnob)
};

class RotaryKnobAttachment : private juce::AudioProcessorValueTreeState::Listener
{
public:
    RotaryKnobAttachment(juce::AudioProcessorValueTreeState& apvts,
                         const juce::String& parameterID,
                         RotaryKnob& knob);
    ~RotaryKnobAttachment() override;

private:
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState& apvts;
    juce::String paramId;
    RotaryKnob& knob;
    bool isUpdating = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnobAttachment)
};
