#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class SupaTriggaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SupaTriggaLookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

};

class SupaTriggaEditor : public juce::AudioProcessorEditor
{
public:
    SupaTriggaEditor(SupaTriggaProcessor&, juce::AudioProcessorValueTreeState&);
    ~SupaTriggaEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    [[maybe_unused]] SupaTriggaProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& apvts;

    SupaTriggaLookAndFeel customLookAndFeel;

    enum class Section { Global, Speed, Reverse, Repeat };

    void setupKnob(juce::Slider& slider, Section section);
    void setupToggle(juce::ToggleButton& toggle, Section section);

    // Header Components
    // Not strictly needed as component, can be drawn in paint() instead to save overhead.

    // Global Section
    juce::Slider rearrangeKnob;
    juce::Slider slicesKnob;
    juce::Slider silenceKnob;

    // Brake Section
    juce::Slider brakeProbKnob;
    juce::Slider brakeTimeKnob;
    juce::ToggleButton brakeInstantToggle;

    // Reverse Section
    juce::Slider reverseProbKnob;
    juce::ToggleButton reverseInstantToggle;

    // Repeat Section
    juce::Slider repeatProbKnob;
    juce::ToggleButton repeatInstantToggle;

    // Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> rearrangeAttach;
    std::unique_ptr<SliderAttachment> slicesAttach;
    std::unique_ptr<SliderAttachment> silenceAttach;
    std::unique_ptr<SliderAttachment> brakeProbAttach;
    std::unique_ptr<SliderAttachment> brakeTimeAttach;
    std::unique_ptr<ButtonAttachment> brakeInstantAttach;
    std::unique_ptr<SliderAttachment> reverseProbAttach;
    std::unique_ptr<ButtonAttachment> reverseInstantAttach;
    std::unique_ptr<SliderAttachment> repeatProbAttach;
    std::unique_ptr<ButtonAttachment> repeatInstantAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SupaTriggaEditor)
};
