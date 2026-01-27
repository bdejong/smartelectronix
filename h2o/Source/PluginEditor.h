#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/FilmStripKnob.h"
#include "UI/BitmapTextDisplay.h"
#include "UI/SplashOverlay.h"

class H2OEditor : public juce::AudioProcessorEditor,
                  public juce::Timer,
                  private juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit H2OEditor(H2OProcessor&);
    ~H2OEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

    void timerCallback() override;

private:
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    H2OProcessor& processorRef;

    // Images
    juce::Image backgroundImage;
    juce::Image knobImage;
    juce::Image saturateImage;
    juce::Image splashImage;
    juce::Image asciiImage;

    // UI components
    std::unique_ptr<FilmStripKnob> preampKnob;
    std::unique_ptr<FilmStripKnob> attackKnob;
    std::unique_ptr<FilmStripKnob> releaseKnob;
    std::unique_ptr<FilmStripKnob> amountKnob;
    std::unique_ptr<FilmStripKnob> postampKnob;
    std::unique_ptr<FilmStripKnob> saturateButton;
    std::unique_ptr<BitmapTextDisplay> textDisplay;
    std::unique_ptr<SplashOverlay> splashOverlay;

    // Parameter attachments
    std::unique_ptr<FilmStripKnobAttachment> preampAttachment;
    std::unique_ptr<FilmStripKnobAttachment> attackAttachment;
    std::unique_ptr<FilmStripKnobAttachment> releaseAttachment;
    std::unique_ptr<FilmStripKnobAttachment> amountAttachment;
    std::unique_ptr<FilmStripKnobAttachment> postampAttachment;
    std::unique_ptr<FilmStripKnobAttachment> saturateAttachment;

    // Splash trigger areas (from original H2Oeditor.cpp)
    juce::Rectangle<int> splashArea1{208, 94, 26, 27};   // Logo area
    juce::Rectangle<int> splashArea2{124, 134, 147, 25}; // Bottom text
    juce::Rectangle<int> splashArea3{15, 8, 79, 27};     // H2O text

    // Display formatting helpers
    void updateDisplayText(const juce::String& paramId);
    juce::String formatParameterValue(const juce::String& paramId);
    juce::String formatFloat(float value);
    juce::String formatDb(float linearValue);

    static constexpr int windowWidth = 287;
    static constexpr int windowHeight = 166;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(H2OEditor)
};
