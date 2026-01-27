#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../DSP/Shaper.h"

static constexpr int X_PAD = 8;
static constexpr int Y_PAD = 8;

class PreviewComponent : public juce::Component, public juce::Timer
{
public:
    PreviewComponent(CShaper& shaper, const juce::Image& backgroundImage);
    ~PreviewComponent() override;

    void paint(juce::Graphics& g) override;

    void markDirty() { dirty = true; }

    void timerCallback() override;

private:
    void fillSine();

    CShaper& shaper;
    juce::Image bgImage;

    std::vector<float> sinArray;
    std::vector<float> tmpArray;
    bool dirty = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PreviewComponent)
};
