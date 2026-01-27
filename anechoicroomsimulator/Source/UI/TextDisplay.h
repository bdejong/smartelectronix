#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Text display using bitmap font - direct translation of original TextDisplay.cpp
class TextDisplay : public juce::Component
{
public:
    TextDisplay(const juce::Image& fontImage, juce::Colour bgColor);
    ~TextDisplay() override = default;

    void paint(juce::Graphics& g) override;

    void setText(const juce::String& newText);
    const juce::String& getText() const { return text; }

    void setRightAligned(bool rightAlign) { rightAligned = rightAlign; }

private:
    juce::Image fontImage;
    juce::Colour backgroundColor;
    juce::String text;
    bool rightAligned = true;  // Original aligns right

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextDisplay)
};
