#pragma once

#include "PluginProcessor.h"
#include "UI/FilmStripKnob.h"
#include "UI/TextDisplay.h"
#include "UI/SplashOverlay.h"
#include "UI/ClickArea.h"

class AnechoicRoomSimEditor : public juce::AudioProcessorEditor,
                               private juce::Timer
{
public:
    explicit AnechoicRoomSimEditor(AnechoicRoomSimProcessor&);
    ~AnechoicRoomSimEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void updateDisplayText();

    AnechoicRoomSimProcessor& processorRef;

    juce::Image backgroundImage;

    FilmStripKnob sizeKnob;
    std::unique_ptr<FilmStripKnobAttachment> sizeKnobAttachment;

    TextDisplay sizeDisplay;

    SplashOverlay splashOverlay;

    // Invisible clickable area to trigger splash
    ClickArea splashTrigger;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnechoicRoomSimEditor)
};
