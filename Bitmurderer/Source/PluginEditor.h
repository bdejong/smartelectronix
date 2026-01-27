#pragma once

#include "PluginProcessor.h"
#include "UI/FilmStripKnob.h"
#include "UI/SplashOverlay.h"
#include "BinaryData.h"

class BitmurdererEditor : public juce::AudioProcessorEditor
{
public:
    BitmurdererEditor(BitmurdererProcessor&);
    ~BitmurdererEditor() override;

    void paint(juce::Graphics&) override;

private:
    BitmurdererProcessor& processor;

    juce::Image backgroundImage;

    // 15 bit toggles + 3 mode toggles
    std::unique_ptr<FilmStripKnob> buttons[18];
    std::unique_ptr<FilmStripKnobAttachment> attachments[18];

    // Splash
    std::unique_ptr<SplashOverlay> splashOverlay;
    std::unique_ptr<juce::Component> splashTrigger;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitmurdererEditor)
};
