#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/VerticalFillSlider.h"
#include "UI/NumberBoxDisplay.h"
#include "UI/FilmStripKnob.h"
#include "UI/ClickArea.h"
#include "UI/SplashOverlay.h"

class MadshiftaEditor : public juce::AudioProcessorEditor,
                        private juce::Timer
{
public:
    explicit MadshiftaEditor(MadshiftaProcessor&);
    ~MadshiftaEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    MadshiftaProcessor& processorRef;

    juce::Image backgroundImage;
    juce::Image filterModeImage;
    juce::Image midiModeImage;
    juce::Image splashImage;

    // Faders
    std::unique_ptr<VerticalFillSlider> tuneFader;
    std::unique_ptr<VerticalFillSlider> fineTuneFader;
    std::unique_ptr<VerticalFillSlider> dryWetFader;
    std::unique_ptr<VerticalFillSlider> gainFader;

    // Number box displays
    std::unique_ptr<NumberBoxDisplay> delayCircle;
    std::unique_ptr<NumberBoxDisplay> feedbackCircle;
    std::unique_ptr<NumberBoxDisplay> cutoffCircle;
    std::unique_ptr<NumberBoxDisplay> resonanceCircle;
    std::unique_ptr<NumberBoxDisplay> rootKeyBox;

    // Toggle buttons
    std::unique_ptr<FilmStripKnob> filterModeButton;
    std::unique_ptr<FilmStripKnob> midiModeButton;

    // Click areas
    std::unique_ptr<ClickArea> randomizeAll;
    std::unique_ptr<ClickArea> randomizePitch;
    std::unique_ptr<ClickArea> splashZone;

    // Splash overlay
    std::unique_ptr<SplashOverlay> splashOverlay;

    // Attachments
    std::unique_ptr<VerticalFillSliderAttachment> tuneAttachment;
    std::unique_ptr<VerticalFillSliderAttachment> fineAttachment;
    std::unique_ptr<VerticalFillSliderAttachment> dryWetAttachment;
    std::unique_ptr<VerticalFillSliderAttachment> gainAttachment;

    std::unique_ptr<NumberBoxDisplayAttachment> delayAttachment;
    std::unique_ptr<NumberBoxDisplayAttachment> feedbackAttachment;
    std::unique_ptr<NumberBoxDisplayAttachment> cutoffAttachment;
    std::unique_ptr<NumberBoxDisplayAttachment> resonanceAttachment;
    std::unique_ptr<NumberBoxDisplayAttachment> rootAttachment;

    std::unique_ptr<FilmStripKnobAttachment> filterModeAttachment;
    std::unique_ptr<FilmStripKnobAttachment> midiModeAttachment;

    static constexpr int windowWidth = 432;
    static constexpr int windowHeight = 360;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MadshiftaEditor)
};
