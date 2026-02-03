#include "PluginEditor.h"
#include "BinaryData.h"

SmexoscopeEditor::SmexoscopeEditor(SmexoscopeProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(825, 300);
    setWantsKeyboardFocus(true);

    // Load light skin images
    lightSkin.background = juce::ImageCache::getFromMemory(BinaryData::body_png, BinaryData::body_pngSize);
    lightSkin.knob = juce::ImageCache::getFromMemory(BinaryData::blue_knob1_4_png, BinaryData::blue_knob1_4_pngSize);
    lightSkin.heads = juce::ImageCache::getFromMemory(BinaryData::heads_png, BinaryData::heads_pngSize);
    lightSkin.readout = juce::ImageCache::getFromMemory(BinaryData::readout_png, BinaryData::readout_pngSize);
    lightSkin.freeEtc = juce::ImageCache::getFromMemory(BinaryData::free_etc_png, BinaryData::free_etc_pngSize);
    lightSkin.onOff = juce::ImageCache::getFromMemory(BinaryData::off_on_png, BinaryData::off_on_pngSize);
    lightSkin.channel = juce::ImageCache::getFromMemory(BinaryData::lefr_right_png, BinaryData::lefr_right_pngSize);
    lightSkin.slider = juce::ImageCache::getFromMemory(BinaryData::slider_new_png, BinaryData::slider_new_pngSize);

    // Load dark skin images (JUCE names them with "2" suffix for files in dark/ subfolder)
    darkSkin.background = juce::ImageCache::getFromMemory(BinaryData::body_png2, BinaryData::body_png2Size);
    darkSkin.knob = juce::ImageCache::getFromMemory(BinaryData::blue_knob1_4_png2, BinaryData::blue_knob1_4_png2Size);
    darkSkin.heads = juce::ImageCache::getFromMemory(BinaryData::heads_png2, BinaryData::heads_png2Size);
    darkSkin.readout = juce::ImageCache::getFromMemory(BinaryData::readout_png2, BinaryData::readout_png2Size);
    darkSkin.freeEtc = juce::ImageCache::getFromMemory(BinaryData::free_etc_png2, BinaryData::free_etc_png2Size);
    darkSkin.onOff = juce::ImageCache::getFromMemory(BinaryData::off_on_png2, BinaryData::off_on_png2Size);
    darkSkin.channel = juce::ImageCache::getFromMemory(BinaryData::lefr_right_png2, BinaryData::lefr_right_png2Size);
    darkSkin.slider = juce::ImageCache::getFromMemory(BinaryData::slider_new_png2, BinaryData::slider_new_png2Size);

    // Use skin preference from processor (defaults to OS dark mode, or saved state)
    useDarkSkin = processorRef.useDarkSkin;
    auto& initialSkin = useDarkSkin ? darkSkin : lightSkin;
    backgroundImage = initialSkin.background;
    auto blueKnobImg = initialSkin.knob;
    auto headsImg = initialSkin.heads;
    auto readoutImg = initialSkin.readout;
    auto freeEtcImg = initialSkin.freeEtc;
    auto onOffImg = initialSkin.onOff;
    auto channelImg = initialSkin.channel;
    auto sliderImg = initialSkin.slider;

    int knobW = blueKnobImg.getWidth();
    int knobFrameH = blueKnobImg.getHeight() / 75;

    // Wave display (38,16) to (665,285) => width=627, height=269
    waveDisplay = std::make_unique<WaveDisplay>(processorRef, backgroundImage, headsImg, readoutImg);
    waveDisplay->setBounds(38, 16, 627, 269);
    addAndMakeVisible(*waveDisplay);

    // Time window knob
    timeWindowKnob = std::make_unique<FilmStripKnob>(blueKnobImg, 75);
    timeWindowKnob->setBounds(697, 31, knobW, knobFrameH);
    addAndMakeVisible(*timeWindowKnob);

    // Amp window knob
    ampWindowKnob = std::make_unique<FilmStripKnob>(blueKnobImg, 75);
    ampWindowKnob->setBounds(762, 31, knobW, knobFrameH);
    addAndMakeVisible(*ampWindowKnob);

    // Trigger speed knob
    triggerSpeedKnob = std::make_unique<FilmStripKnob>(blueKnobImg, 75);
    triggerSpeedKnob->setBounds(700, 134, knobW, knobFrameH);
    addAndMakeVisible(*triggerSpeedKnob);

    // Trigger limit knob
    triggerLimitKnob = std::make_unique<FilmStripKnob>(blueKnobImg, 75);
    triggerLimitKnob->setBounds(765, 134, knobW, knobFrameH);
    addAndMakeVisible(*triggerLimitKnob);

    // Labels (grey background, white text, small font)
    juce::Colour grey(118, 118, 118);
    juce::Font smallFont(juce::FontOptions(10.0f));

    auto makeLabel = [&](std::unique_ptr<juce::Label>& label, int x1, int y1, int x2, int y2) {
        label = std::make_unique<juce::Label>();
        label->setBounds(x1, y1, x2 - x1, y2 - y1);
        label->setColour(juce::Label::backgroundColourId, grey);
        label->setColour(juce::Label::textColourId, juce::Colours::white);
        label->setFont(smallFont);
        label->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*label);
    };

    makeLabel(timeLabel,   696, 72,  730, 83);
    makeLabel(ampLabel,    760, 72,  794, 83);
    makeLabel(trigLabel,   698, 175, 732, 186);
    makeLabel(threshLabel, 765, 175, 799, 186);

    // Trigger type (5 states, frame height = image height / 6)
    int freeEtcFrameH = freeEtcImg.getHeight() / 6;
    triggerTypeButton = std::make_unique<MultiStateButton>(freeEtcImg, 5, freeEtcFrameH);
    triggerTypeButton->setBounds(718, 94, freeEtcImg.getWidth(), freeEtcFrameH);
    addAndMakeVisible(*triggerTypeButton);

    // Toggle buttons
    int onOffFrameH = onOffImg.getHeight() / 2;

    syncDrawToggle = std::make_unique<FilmStripKnob>(onOffImg, 2, true);
    syncDrawToggle->setBounds(696, 221, onOffImg.getWidth(), onOffFrameH);
    addAndMakeVisible(*syncDrawToggle);

    dcKillToggle = std::make_unique<FilmStripKnob>(onOffImg, 2, true);
    dcKillToggle->setBounds(690, 259, onOffImg.getWidth(), onOffFrameH);
    addAndMakeVisible(*dcKillToggle);

    freezeToggle = std::make_unique<FilmStripKnob>(onOffImg, 2, true);
    freezeToggle->setBounds(754, 221, onOffImg.getWidth(), onOffFrameH);
    addAndMakeVisible(*freezeToggle);

    channelToggle = std::make_unique<FilmStripKnob>(channelImg, 2, true);
    channelToggle->setBounds(748, 259, channelImg.getWidth(), channelImg.getHeight() / 2);
    addAndMakeVisible(*channelToggle);

    // Trigger level slider
    // Original: CSlider at (11, 13, 11+sliderW, 13+277), minPos=13, maxPos=13+277-sliderH-1, kVertical|kBottom
    int sliderTop = 13;
    int sliderHeight = 277;
    int sliderHandleH = sliderImg.getHeight();
    int sliderW = sliderImg.getWidth();
    int minPos = 0; // relative to component
    int maxPos = sliderHeight - sliderHandleH - 1;

    triggerLevelSlider = std::make_unique<FilmStripSlider>(sliderImg, minPos, maxPos);
    triggerLevelSlider->setBounds(11, sliderTop, sliderW, sliderHeight);
    addAndMakeVisible(*triggerLevelSlider);

    // Create attachments
    timeWindowAtt    = std::make_unique<FilmStripKnobAttachment>(p.apvts, SmexoscopeProcessor::PARAM_TIME_WINDOW, *timeWindowKnob);
    ampWindowAtt     = std::make_unique<FilmStripKnobAttachment>(p.apvts, SmexoscopeProcessor::PARAM_AMP_WINDOW, *ampWindowKnob);
    triggerSpeedAtt  = std::make_unique<FilmStripKnobAttachment>(p.apvts, SmexoscopeProcessor::PARAM_TRIGGER_SPEED, *triggerSpeedKnob);
    triggerLimitAtt  = std::make_unique<FilmStripKnobAttachment>(p.apvts, SmexoscopeProcessor::PARAM_TRIGGER_LIMIT, *triggerLimitKnob);
    triggerTypeAtt   = std::make_unique<MultiStateButtonAttachment>(p.apvts, SmexoscopeProcessor::PARAM_TRIGGER_TYPE, *triggerTypeButton);
    syncDrawAtt      = std::make_unique<FilmStripKnobAttachment>(p.apvts, SmexoscopeProcessor::PARAM_SYNC_DRAW, *syncDrawToggle);
    freezeAtt        = std::make_unique<FilmStripKnobAttachment>(p.apvts, SmexoscopeProcessor::PARAM_FREEZE, *freezeToggle);
    dcKillAtt        = std::make_unique<FilmStripKnobAttachment>(p.apvts, SmexoscopeProcessor::PARAM_DC_KILL, *dcKillToggle);
    channelAtt       = std::make_unique<FilmStripKnobAttachment>(p.apvts, SmexoscopeProcessor::PARAM_CHANNEL, *channelToggle);
    triggerLevelAtt  = std::make_unique<FilmStripSliderAttachment>(p.apvts, SmexoscopeProcessor::PARAM_TRIGGER_LEVEL, *triggerLevelSlider);

    // Initial label update
    updateLabels();

    // Start timer for label updates at 30Hz
    startTimerHz(30);
}

SmexoscopeEditor::~SmexoscopeEditor()
{
    stopTimer();
}

void SmexoscopeEditor::paint(juce::Graphics& g)
{
    if (!backgroundImage.isNull())
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
}

void SmexoscopeEditor::timerCallback()
{
    updateLabels();
}

void SmexoscopeEditor::updateLabels()
{
    timeLabel->setText(processorRef.getDisplayText(SmexoscopeProcessor::PARAM_TIME_WINDOW), juce::dontSendNotification);
    ampLabel->setText(processorRef.getDisplayText(SmexoscopeProcessor::PARAM_AMP_WINDOW), juce::dontSendNotification);
    trigLabel->setText(processorRef.getDisplayText(SmexoscopeProcessor::PARAM_TRIGGER_SPEED), juce::dontSendNotification);
    threshLabel->setText(processorRef.getDisplayText(SmexoscopeProcessor::PARAM_TRIGGER_LIMIT), juce::dontSendNotification);
}

bool SmexoscopeEditor::keyPressed(const juce::KeyPress&)
{
    useDarkSkin = !useDarkSkin;
    processorRef.useDarkSkin = useDarkSkin;
    applySkin(useDarkSkin);
    return true;
}

void SmexoscopeEditor::applySkin(bool dark)
{
    auto& skin = dark ? darkSkin : lightSkin;

    backgroundImage = skin.background;

    timeWindowKnob->setImage(skin.knob);
    ampWindowKnob->setImage(skin.knob);
    triggerSpeedKnob->setImage(skin.knob);
    triggerLimitKnob->setImage(skin.knob);

    int freeEtcFrameH = skin.freeEtc.getHeight() / 6;
    triggerTypeButton->setImage(skin.freeEtc, freeEtcFrameH);

    syncDrawToggle->setImage(skin.onOff);
    dcKillToggle->setImage(skin.onOff);
    freezeToggle->setImage(skin.onOff);
    channelToggle->setImage(skin.channel);

    triggerLevelSlider->setImage(skin.slider);

    waveDisplay->setImages(skin.background, skin.heads, skin.readout);

    repaint();
}
