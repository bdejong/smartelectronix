#pragma once

#include "PluginProcessor.h"
#include "UI/FilmStripKnob.h"
#include "UI/FilmStripSlider.h"
#include "UI/MultiStateButton.h"
#include "UI/WaveDisplay.h"

class SmexoscopeEditor : public juce::AudioProcessorEditor,
                         private juce::Timer
{
public:
    SmexoscopeEditor(SmexoscopeProcessor&);
    ~SmexoscopeEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override {}
    bool keyPressed(const juce::KeyPress& key) override;

private:
    void timerCallback() override;
    void updateLabels();
    void applySkin(bool dark);

    SmexoscopeProcessor& processorRef;

    // Skin images
    struct SkinImages {
        juce::Image background, knob, heads, readout, freeEtc, onOff, channel, slider;
    };
    SkinImages lightSkin, darkSkin;
    bool useDarkSkin = false;

    juce::Image backgroundImage;

    std::unique_ptr<WaveDisplay> waveDisplay;

    // Knobs
    std::unique_ptr<FilmStripKnob> timeWindowKnob, ampWindowKnob;
    std::unique_ptr<FilmStripKnob> triggerSpeedKnob, triggerLimitKnob;

    // Trigger type
    std::unique_ptr<MultiStateButton> triggerTypeButton;

    // Toggles
    std::unique_ptr<FilmStripKnob> syncDrawToggle, freezeToggle;
    std::unique_ptr<FilmStripKnob> dcKillToggle, channelToggle;

    // Slider
    std::unique_ptr<FilmStripSlider> triggerLevelSlider;

    // Labels
    std::unique_ptr<juce::Label> timeLabel, ampLabel, trigLabel, threshLabel;

    // Attachments
    std::unique_ptr<FilmStripKnobAttachment> timeWindowAtt, ampWindowAtt;
    std::unique_ptr<FilmStripKnobAttachment> triggerSpeedAtt, triggerLimitAtt;
    std::unique_ptr<MultiStateButtonAttachment> triggerTypeAtt;
    std::unique_ptr<FilmStripKnobAttachment> syncDrawAtt, freezeAtt;
    std::unique_ptr<FilmStripKnobAttachment> dcKillAtt, channelAtt;
    std::unique_ptr<FilmStripSliderAttachment> triggerLevelAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SmexoscopeEditor)
};
