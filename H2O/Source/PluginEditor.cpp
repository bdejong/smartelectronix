#include "PluginEditor.h"
#include "BinaryData.h"
#include <cmath>

// Infinity character code (from original asciitable.h)
static constexpr char INF_CHAR = (char)250;

H2OEditor::H2OEditor(H2OProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Load images from binary data
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::back_png, BinaryData::back_pngSize);
    knobImage = juce::ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize);
    saturateImage = juce::ImageCache::getFromMemory(BinaryData::saturate_png, BinaryData::saturate_pngSize);
    splashImage = juce::ImageCache::getFromMemory(BinaryData::splash_png, BinaryData::splash_pngSize);
    asciiImage = juce::ImageCache::getFromMemory(BinaryData::ascii_png, BinaryData::ascii_pngSize);

    // Create knobs (60 frames each)
    preampKnob = std::make_unique<FilmStripKnob>(knobImage, 60);
    attackKnob = std::make_unique<FilmStripKnob>(knobImage, 60);
    releaseKnob = std::make_unique<FilmStripKnob>(knobImage, 60);
    amountKnob = std::make_unique<FilmStripKnob>(knobImage, 60);
    postampKnob = std::make_unique<FilmStripKnob>(knobImage, 60);

    // Create saturate toggle button (2 frames)
    saturateButton = std::make_unique<FilmStripKnob>(saturateImage, 2, true);

    // Create text display
    textDisplay = std::make_unique<BitmapTextDisplay>(asciiImage);
    textDisplay->setText("H2O");

    // Create splash overlay
    splashOverlay = std::make_unique<SplashOverlay>(splashImage);

    // Add all components
    addAndMakeVisible(*preampKnob);
    addAndMakeVisible(*attackKnob);
    addAndMakeVisible(*releaseKnob);
    addAndMakeVisible(*amountKnob);
    addAndMakeVisible(*postampKnob);
    addAndMakeVisible(*saturateButton);
    addAndMakeVisible(*textDisplay);
    addChildComponent(*splashOverlay);  // Initially hidden

    // Create parameter attachments
    auto& apvts = processorRef.getAPVTS();
    preampAttachment = std::make_unique<FilmStripKnobAttachment>(apvts, H2OProcessor::PREAMP_ID, *preampKnob);
    attackAttachment = std::make_unique<FilmStripKnobAttachment>(apvts, H2OProcessor::ATTACK_ID, *attackKnob);
    releaseAttachment = std::make_unique<FilmStripKnobAttachment>(apvts, H2OProcessor::RELEASE_ID, *releaseKnob);
    amountAttachment = std::make_unique<FilmStripKnobAttachment>(apvts, H2OProcessor::AMOUNT_ID, *amountKnob);
    postampAttachment = std::make_unique<FilmStripKnobAttachment>(apvts, H2OProcessor::POSTAMP_ID, *postampKnob);
    saturateAttachment = std::make_unique<FilmStripKnobAttachment>(apvts, H2OProcessor::SATURATE_ID, *saturateButton);

    // Listen for parameter changes to update text display
    apvts.addParameterListener(H2OProcessor::PREAMP_ID, this);
    apvts.addParameterListener(H2OProcessor::ATTACK_ID, this);
    apvts.addParameterListener(H2OProcessor::RELEASE_ID, this);
    apvts.addParameterListener(H2OProcessor::AMOUNT_ID, this);
    apvts.addParameterListener(H2OProcessor::POSTAMP_ID, this);
    apvts.addParameterListener(H2OProcessor::SATURATE_ID, this);

    setSize(windowWidth, windowHeight);

    // Start timer for resetting display text
    startTimer(2000);
}

H2OEditor::~H2OEditor()
{
    stopTimer();

    // Remove parameter listeners
    auto& apvts = processorRef.getAPVTS();
    apvts.removeParameterListener(H2OProcessor::PREAMP_ID, this);
    apvts.removeParameterListener(H2OProcessor::ATTACK_ID, this);
    apvts.removeParameterListener(H2OProcessor::RELEASE_ID, this);
    apvts.removeParameterListener(H2OProcessor::AMOUNT_ID, this);
    apvts.removeParameterListener(H2OProcessor::POSTAMP_ID, this);
    apvts.removeParameterListener(H2OProcessor::SATURATE_ID, this);
}

void H2OEditor::paint(juce::Graphics& g)
{
    // Draw background
    if (!backgroundImage.isNull())
    {
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
    }
    else
    {
        g.fillAll(juce::Colours::darkgrey);
    }
}

void H2OEditor::resized()
{
    // Knob size (from original: hKnob->getWidth() x hKnob->getHeight() / 60)
    int knobWidth = knobImage.getWidth();
    int knobHeight = knobImage.getHeight() / 60;
    int offset = 39;

    // Position knobs according to original H2Oeditor.cpp
    // ingain at (52, 55)
    preampKnob->setBounds(52, 55, knobWidth, knobHeight);

    // attack at offset from ingain
    attackKnob->setBounds(52 + offset, 55, knobWidth, knobHeight);

    // release at 2*offset from ingain
    releaseKnob->setBounds(52 + 2 * offset, 55, knobWidth, knobHeight);

    // amount at (168, 55) - note: original skips to 168, not continuing from release
    amountKnob->setBounds(168, 55, knobWidth, knobHeight);

    // outgain at offset from amount
    postampKnob->setBounds(168 + offset, 55, knobWidth, knobHeight);

    // Saturate button at (169, 94)
    int saturateWidth = saturateImage.getWidth();
    int saturateHeight = saturateImage.getHeight() / 2;
    saturateButton->setBounds(169, 94, saturateWidth, saturateHeight);

    // Text display at (58, 98, 94, 16)
    textDisplay->setBounds(58, 98, 94, 16);

    // Splash overlay covers the whole window
    splashOverlay->setBounds(getLocalBounds());
}

void H2OEditor::mouseDown(const juce::MouseEvent& event)
{
    auto pos = event.getPosition();

    // Check splash trigger areas
    if (splashArea1.contains(pos) || splashArea2.contains(pos) || splashArea3.contains(pos))
    {
        splashOverlay->show();
    }
}

void H2OEditor::timerCallback()
{
    // Reset display to show "H2O" after idle
    textDisplay->setText("H2O");
}

void H2OEditor::parameterChanged(const juce::String& parameterID, float /*newValue*/)
{
    // Update text display on message thread
    juce::MessageManager::callAsync([this, parameterID]()
    {
        updateDisplayText(parameterID);
    });
}

void H2OEditor::updateDisplayText(const juce::String& paramId)
{
    // Restart the timer whenever a parameter changes
    stopTimer();
    startTimer(2000);

    textDisplay->setText(formatParameterValue(paramId));
}

juce::String H2OEditor::formatParameterValue(const juce::String& paramId)
{
    auto& comp = processorRef.getCompressor();

    if (paramId == H2OProcessor::PREAMP_ID)
    {
        return formatDb(comp.getPreamp()) + " dB";
    }
    else if (paramId == H2OProcessor::ATTACK_ID)
    {
        return formatFloat(comp.getAttack()) + " ms";
    }
    else if (paramId == H2OProcessor::RELEASE_ID)
    {
        return formatFloat(comp.getRelease()) + " ms";
    }
    else if (paramId == H2OProcessor::AMOUNT_ID)
    {
        return formatFloat(comp.getAmount());
    }
    else if (paramId == H2OProcessor::POSTAMP_ID)
    {
        return formatDb(comp.getPostamp()) + " dB";
    }
    else if (paramId == H2OProcessor::SATURATE_ID)
    {
        return comp.getSaturate() ? "on" : "off";
    }

    return "H2O";
}

juce::String H2OEditor::formatFloat(float value)
{
    // Format similar to original float2string2
    if (value > 99999999.f)
        return " Huge!  ";

    // Format with decimal places
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.2f", value);
    return juce::String(buffer);
}

juce::String H2OEditor::formatDb(float linearValue)
{
    // Format similar to original dB2string2
    if (linearValue <= 0.f)
    {
        // Return infinity symbol
        juce::String result;
        result += '-';
        result += INF_CHAR;
        return result;
    }

    float dB = 20.f * std::log10(linearValue);
    return formatFloat(dB);
}
