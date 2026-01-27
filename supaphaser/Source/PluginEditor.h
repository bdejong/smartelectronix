#pragma once

#include "PluginProcessor.h"
#include "UI/FilmStripKnob.h"
#include "UI/FilmStripSlider.h"
#include "UI/MultiStateButton.h"
#include "UI/TextDisplay.h"
#include "UI/SplashOverlay.h"
#include "UI/ClickArea.h"

class SupaPhaserEditor : public juce::AudioProcessorEditor,
                         private juce::Timer
{
public:
    SupaPhaserEditor(SupaPhaserProcessor&);
    ~SupaPhaserEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override {}

private:
    void timerCallback() override;
    void updateDisplays();

    SupaPhaserProcessor& processorRef;

    juce::Image backgroundImage;

    // Blue knobs (75 frames, 33px high each)
    std::unique_ptr<FilmStripKnob> attackKnob, releaseKnob, envMinKnob, envMaxKnob;
    std::unique_ptr<FilmStripKnob> freqKnob, stereoKnob, lfoMinKnob, lfoMaxKnob;
    std::unique_ptr<FilmStripKnob> feedbackKnob, stagesKnob;

    // Large distortion knob (111 frames, 68px)
    std::unique_ptr<FilmStripKnob> distortKnob;

    // Small mixture knob (62 frames, 38px)
    std::unique_ptr<FilmStripKnob> mixtureKnob;

    // Vertical sliders
    std::unique_ptr<FilmStripSlider> dryWetSlider, gainSlider;

    // Extend toggle
    std::unique_ptr<FilmStripKnob> extendToggle;

    // Invert multi-state button
    std::unique_ptr<MultiStateButton> invertButton;

    // Text displays
    std::unique_ptr<TextDisplay> attackDisplay, releaseDisplay;
    std::unique_ptr<TextDisplay> envMinDisplay, envMaxDisplay;
    std::unique_ptr<TextDisplay> freqDisplay, stereoDisplay;
    std::unique_ptr<TextDisplay> lfoMinDisplay, lfoMaxDisplay;
    std::unique_ptr<TextDisplay> feedbackDisplay, stagesDisplay;
    std::unique_ptr<TextDisplay> mixtureDisplay1, mixtureDisplay2;
    std::unique_ptr<TextDisplay> dryWetDisplay, gainDisplay;

    // Splash
    std::unique_ptr<SplashOverlay> splashOverlay;
    std::unique_ptr<ClickArea> splashArea;

    // Attachments
    std::unique_ptr<FilmStripKnobAttachment> attackAtt, releaseAtt, envMinAtt, envMaxAtt;
    std::unique_ptr<FilmStripKnobAttachment> freqAtt, stereoAtt, lfoMinAtt, lfoMaxAtt;
    std::unique_ptr<FilmStripKnobAttachment> feedbackAtt, stagesAtt;
    std::unique_ptr<FilmStripKnobAttachment> distortAtt, mixtureAtt;
    std::unique_ptr<FilmStripSliderAttachment> dryWetAtt, gainAtt;
    std::unique_ptr<FilmStripKnobAttachment> extendAtt;
    std::unique_ptr<MultiStateButtonAttachment> invertAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SupaPhaserEditor)
};
