#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class FilmStripSlider : public juce::Component
{
public:
    FilmStripSlider(const juce::Image& handleImage, int minY, int maxY, int handleOffsetX = 0);
    ~FilmStripSlider() override = default;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;

    void setValue(float newValue);
    float getValue() const { return value; }
    void setImage(const juce::Image& newImage) { handleImage = newImage; repaint(); }

    std::function<void(float)> onValueChange;

private:
    juce::Image handleImage;
    int minY, maxY;
    int handleOffsetX = 0;
    float value = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilmStripSlider)
};

class FilmStripSliderAttachment : private juce::AudioProcessorValueTreeState::Listener
{
public:
    FilmStripSliderAttachment(juce::AudioProcessorValueTreeState& apvts,
                              const juce::String& parameterID,
                              FilmStripSlider& slider);
    ~FilmStripSliderAttachment() override;

private:
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState& apvts;
    juce::String paramId;
    FilmStripSlider& slider;
    bool isUpdating = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilmStripSliderAttachment)
};
