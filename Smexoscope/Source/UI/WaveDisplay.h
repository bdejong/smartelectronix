#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"

class WaveDisplay : public juce::Component,
                    private juce::Timer
{
public:
    WaveDisplay(SmexoscopeProcessor& processor,
                const juce::Image& backgroundImage,
                const juce::Image& headsImage,
                const juce::Image& readoutImage);
    ~WaveDisplay() override;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    void timerCallback() override;

    SmexoscopeProcessor& processor;
    juce::Image backgroundImage;
    juce::Image headsImage;
    juce::Image readoutImage;

    juce::Point<int> mousePos { -1, 0 };
    unsigned char headsFrame;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveDisplay)
};
