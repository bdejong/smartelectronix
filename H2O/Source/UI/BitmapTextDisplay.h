#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class BitmapTextDisplay : public juce::Component
{
public:
    BitmapTextDisplay(const juce::Image& fontImage);
    ~BitmapTextDisplay() override;

    void paint(juce::Graphics& g) override;

    void setText(const juce::String& newText);
    const juce::String& getText() const { return text; }

private:
    juce::Image fontImage;
    juce::String text;

    // Character lookup - maps character to (x position in image, width)
    static int getCharIndex(char c);
    static int getCharWidth(int index);
    static int getCharPosition(int index);

    // Character width table (from original asciitable.cpp)
    static constexpr int charWidths[73] = {
        10, 8, 10, 10, 8, 7, 10, 9, 4, 7, 9, 8, 11, 9, 11, 8, 12, 9, 8, 10,  // A-T
        9, 10, 13, 9, 9, 9,                                                   // U-Z
        9, 8, 8, 9, 8, 7, 9, 8, 4, 7, 8, 4, 12, 8, 9, 8, 9, 6, 7, 6,         // a-t
        8, 8, 12, 9, 8, 9,                                                     // u-z
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9,                                         // 0-9
        5, 5, 6, 6, 14, 8, 7, 10, 10, 8, 5                                    // . , ; : INF / - + = ? space
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitmapTextDisplay)
};
