#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class TextDisplay : public juce::Component
{
public:
    TextDisplay(const juce::Image& fontImage, juce::Colour bgColor);
    ~TextDisplay() override = default;

    void paint(juce::Graphics& g) override;

    void setText(const juce::String& newText);
    const juce::String& getText() const { return text; }

    void setRightAligned(bool rightAlign) { rightAligned = rightAlign; }
    void setMiddleAligned(bool midAlign) { middleAligned = midAlign; }

private:
    juce::Image fontImage;
    juce::Colour backgroundColor;
    juce::String text;
    bool rightAligned = true;
    bool middleAligned = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextDisplay)
};
