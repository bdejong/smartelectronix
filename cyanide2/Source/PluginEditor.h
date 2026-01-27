#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/RotaryKnob.h"
#include "UI/ShaperViewComponent.h"
#include "UI/PreviewComponent.h"
#include "UI/SplashOverlay.h"
#include "UI/FilmStripKnob.h"

class Cyanide2Editor : public juce::AudioProcessorEditor,
                       public juce::Timer,
                       public juce::AudioProcessorValueTreeState::Listener
{
public:
    Cyanide2Editor(Cyanide2Processor& processor);
    ~Cyanide2Editor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void timerCallback() override;
    void parameterChanged(const juce::String& parameterID, float newValue) override;

private:
    Cyanide2Processor& processorRef;

    juce::Image backgroundImage;

    // Knobs
    std::unique_ptr<RotaryKnob> preGainKnob, preTypeKnob, preFilterKnob;
    std::unique_ptr<RotaryKnob> postGainKnob, postTypeKnob, postFilterKnob;
    std::unique_ptr<RotaryKnob> dryWetKnob;

    // Knob attachments
    std::unique_ptr<RotaryKnobAttachment> preGainAttach, preTypeAttach, preFilterAttach;
    std::unique_ptr<RotaryKnobAttachment> postGainAttach, postTypeAttach, postFilterAttach;
    std::unique_ptr<RotaryKnobAttachment> dryWetAttach;

    // Toggle buttons (using FilmStripKnob pattern from H2O)
    std::unique_ptr<FilmStripKnob> oversampleButton;
    std::unique_ptr<FilmStripKnobAttachment> oversampleAttach;

    std::unique_ptr<FilmStripKnob> guidesButton;
    std::unique_ptr<FilmStripKnob> resetButton;

    // Shaper and preview
    std::unique_ptr<ShaperViewComponent> shaperView;
    std::unique_ptr<PreviewComponent> previewComponent;

    // Label
    std::unique_ptr<juce::Label> infoLabel;

    // Splash
    std::unique_ptr<SplashOverlay> splashOverlay;
    juce::Rectangle<int> splashTriggerArea;

    void mouseDown(const juce::MouseEvent& event) override;
    void updateDisplayText(const juce::String& paramId);

    int openedCount = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Cyanide2Editor)
};
