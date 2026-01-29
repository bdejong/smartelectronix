#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class FilmStripKnob : public juce::Component
{
public:
    FilmStripKnob(const juce::Image& filmStrip, int numFrames, bool isToggle = false);
    ~FilmStripKnob() override;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    void setValue(float newValue);
    float getValue() const { return value; }

    void setRange(float minVal, float maxVal);
    void setInverseBitmap(bool inverse) { inverseBitmap = inverse; }
    void setImage(const juce::Image& newImage) { filmStrip = newImage; frameHeight = filmStrip.getHeight() / numFrames; repaint(); }

    std::function<void(float)> onValueChange;

private:
    juce::Image filmStrip;
    int numFrames;
    int frameHeight;
    float value = 0.0f;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    bool isToggle;
    bool inverseBitmap = false;

    float dragStartValue = 0.0f;
    int dragStartY = 0;
    static constexpr float dragSensitivity = 0.005f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilmStripKnob)
};

class FilmStripKnobAttachment : private juce::AudioProcessorValueTreeState::Listener
{
public:
    FilmStripKnobAttachment(juce::AudioProcessorValueTreeState& apvts,
                            const juce::String& parameterID,
                            FilmStripKnob& knob);
    ~FilmStripKnobAttachment() override;

private:
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState& apvts;
    juce::String paramId;
    FilmStripKnob& knob;
    bool isUpdating = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilmStripKnobAttachment)
};
