#include "PluginEditor.h"
#include "BinaryData.h"

AnechoicRoomSimEditor::AnechoicRoomSimEditor(AnechoicRoomSimProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      sizeKnob(juce::ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize), 111),
      sizeDisplay(juce::ImageCache::getFromMemory(BinaryData::type_png, BinaryData::type_pngSize),
                  juce::Colour(118, 118, 118)),
      splashOverlay(juce::ImageCache::getFromMemory(BinaryData::splash_png, BinaryData::splash_pngSize))
{
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::back_png, BinaryData::back_pngSize);

    // Set up size knob
    addAndMakeVisible(sizeKnob);
    sizeKnobAttachment = std::make_unique<FilmStripKnobAttachment>(
        processorRef.apvts, AnechoicRoomSimProcessor::SIZE_ID, sizeKnob);

    // Set up size display
    addAndMakeVisible(sizeDisplay);

    // Set up splash overlay (hidden initially)
    addChildComponent(splashOverlay);

    // Set up invisible splash trigger area
    splashTrigger.onClick = [this]() {
        splashOverlay.show();
    };
    addAndMakeVisible(splashTrigger);

    // Start timer for display updates
    startTimerHz(30);

    setSize(backgroundImage.getWidth(), backgroundImage.getHeight());
}

AnechoicRoomSimEditor::~AnechoicRoomSimEditor()
{
    stopTimer();
}

void AnechoicRoomSimEditor::paint(juce::Graphics& g)
{
    g.drawImage(backgroundImage, getLocalBounds().toFloat());
}

void AnechoicRoomSimEditor::resized()
{
    // Position knob at (117, 63) matching original
    // Knob image is 111 frames, each 68 pixels high
    auto knobImage = juce::ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize);
    int knobWidth = knobImage.getWidth();
    int knobHeight = knobImage.getHeight() / 111;
    sizeKnob.setBounds(117, 63, knobWidth, knobHeight);

    // Position text display at (39, 159) with width 64, height matching font
    auto fontImage = juce::ImageCache::getFromMemory(BinaryData::type_png, BinaryData::type_pngSize);
    sizeDisplay.setBounds(39, 159, 64, fontImage.getHeight());

    // Splash overlay covers entire editor
    splashOverlay.setBounds(getLocalBounds());

    // Splash trigger area matches original: (220, 23) to (275, 74)
    splashTrigger.setBounds(220, 23, 55, 51);
}

void AnechoicRoomSimEditor::timerCallback()
{
    updateDisplayText();
}

void AnechoicRoomSimEditor::updateDisplayText()
{
    // Get the parameter value and format it like the original: "XXXXm" in uppercase
    float value = processorRef.apvts.getRawParameterValue(AnechoicRoomSimProcessor::SIZE_ID)->load();
    int meters = static_cast<int>(value * 10000.0f);
    juce::String text = juce::String(meters) + "M";  // Original uses uppercase
    sizeDisplay.setText(text);
}
