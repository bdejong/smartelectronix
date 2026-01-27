#include "PluginEditor.h"
#include "BinaryData.h"

MadshiftaEditor::MadshiftaEditor(MadshiftaProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Load images
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
    filterModeImage = juce::ImageCache::getFromMemory(BinaryData::filter_mode_button_png, BinaryData::filter_mode_button_pngSize);
    midiModeImage = juce::ImageCache::getFromMemory(BinaryData::midi_mode_button_png, BinaryData::midi_mode_button_pngSize);
    splashImage = juce::ImageCache::getFromMemory(BinaryData::info_png, BinaryData::info_pngSize);

    auto& apvts = processorRef.getAPVTS();

    // --- Vertical fill sliders ---

    tuneFader = std::make_unique<VerticalFillSlider>(0.5f, 0.5f,
        [&p](float v) {
            int note = p.lastMidiNote.load();
            if (note >= 0 && note < 128)
                return juce::String(midiNoteNames[note]);
            int s = tuneScaled(v);
            if (s > 0) return juce::String("+") + juce::String(s);
            return juce::String(s);
        });
    tuneFader->setTextPosition(66, 15);
    addAndMakeVisible(*tuneFader);
    tuneAttachment = std::make_unique<VerticalFillSliderAttachment>(apvts, MadshiftaProcessor::TUNE_ID, *tuneFader);

    fineTuneFader = std::make_unique<VerticalFillSlider>(0.5f, 0.5f,
        [](float v) {
            int ft = static_cast<int>(fineTuneScaled(v));
            if (ft > 0) return juce::String("+") + juce::String(ft) + "%";
            return juce::String(ft) + "%";
        });
    fineTuneFader->setTextPosition(66, 15);
    addAndMakeVisible(*fineTuneFader);
    fineAttachment = std::make_unique<VerticalFillSliderAttachment>(apvts, MadshiftaProcessor::FINE_ID, *fineTuneFader);

    dryWetFader = std::make_unique<VerticalFillSlider>(0.0f, 1.0f,
        [](float v) { return juce::String(static_cast<int>(v * 100.0f)) + "%"; });
    dryWetFader->setTextPosition(66, 15);
    addAndMakeVisible(*dryWetFader);
    dryWetAttachment = std::make_unique<VerticalFillSliderAttachment>(apvts, MadshiftaProcessor::DRY_WET_ID, *dryWetFader);

    gainFader = std::make_unique<VerticalFillSlider>(0.0f, 1.0f,
        [](float v) { return juce::String(static_cast<int>(v * 100.0f)) + "%"; });
    gainFader->setTextPosition(66, 15);
    addAndMakeVisible(*gainFader);
    gainAttachment = std::make_unique<VerticalFillSliderAttachment>(apvts, MadshiftaProcessor::OUT_VOL_ID, *gainFader);

    // --- Number box displays ---

    auto percentConvert = [](float v) { return juce::String(static_cast<int>(v * 100.0f)) + "%"; };

    delayCircle = std::make_unique<NumberBoxDisplay>(backgroundImage, 0.0f, percentConvert,
        juce::Colours::white, 10.0f, true);
    addAndMakeVisible(*delayCircle);
    delayAttachment = std::make_unique<NumberBoxDisplayAttachment>(apvts, MadshiftaProcessor::DELAY_LEN_ID, *delayCircle);

    feedbackCircle = std::make_unique<NumberBoxDisplay>(backgroundImage, 0.0f, percentConvert,
        juce::Colours::white, 10.0f, true);
    addAndMakeVisible(*feedbackCircle);
    feedbackAttachment = std::make_unique<NumberBoxDisplayAttachment>(apvts, MadshiftaProcessor::DELAY_FB_ID, *feedbackCircle);

    cutoffCircle = std::make_unique<NumberBoxDisplay>(backgroundImage, 1.0f, percentConvert,
        juce::Colours::white, 10.0f, true);
    addAndMakeVisible(*cutoffCircle);
    cutoffAttachment = std::make_unique<NumberBoxDisplayAttachment>(apvts, MadshiftaProcessor::CUTOFF_ID, *cutoffCircle);

    resonanceCircle = std::make_unique<NumberBoxDisplay>(backgroundImage, 0.0f, percentConvert,
        juce::Colours::white, 10.0f, true);
    addAndMakeVisible(*resonanceCircle);
    resonanceAttachment = std::make_unique<NumberBoxDisplayAttachment>(apvts, MadshiftaProcessor::RESONANCE_ID, *resonanceCircle);

    rootKeyBox = std::make_unique<NumberBoxDisplay>(backgroundImage, 0.36f,
        [](float v) {
            int idx = juce::jlimit(0, 127, rootKeyScaled(v));
            return juce::String(midiNoteNames[idx]);
        },
        juce::Colours::black, 12.0f);
    addAndMakeVisible(*rootKeyBox);
    rootAttachment = std::make_unique<NumberBoxDisplayAttachment>(apvts, MadshiftaProcessor::ROOT_ID, *rootKeyBox);

    // --- Toggle buttons ---

    filterModeButton = std::make_unique<FilmStripKnob>(filterModeImage, 2, true);
    addAndMakeVisible(*filterModeButton);
    filterModeAttachment = std::make_unique<FilmStripKnobAttachment>(apvts, MadshiftaProcessor::FTYPE_ID, *filterModeButton);

    midiModeButton = std::make_unique<FilmStripKnob>(midiModeImage, 2, true);
    addAndMakeVisible(*midiModeButton);
    midiModeAttachment = std::make_unique<FilmStripKnobAttachment>(apvts, MadshiftaProcessor::MMODE_ID, *midiModeButton);

    // --- Click areas ---

    randomizeAll = std::make_unique<ClickArea>();
    randomizeAll->onClick = [this]()
    {
        auto& a = processorRef.getAPVTS();
        juce::Random rng;

        // Randomize all except root, mMode, dryWet, outVol
        auto setRand = [&](const char* id) {
            if (auto* param = a.getParameter(id))
                param->setValueNotifyingHost(rng.nextFloat());
        };
        setRand(MadshiftaProcessor::TUNE_ID);
        setRand(MadshiftaProcessor::FINE_ID);
        setRand(MadshiftaProcessor::DELAY_LEN_ID);
        setRand(MadshiftaProcessor::DELAY_FB_ID);
        setRand(MadshiftaProcessor::CUTOFF_ID);
        setRand(MadshiftaProcessor::RESONANCE_ID);

        // fType gets random 0 or 1
        if (auto* param = a.getParameter(MadshiftaProcessor::FTYPE_ID))
            param->setValueNotifyingHost(rng.nextBool() ? 1.0f : 0.0f);
    };
    addAndMakeVisible(*randomizeAll);

    randomizePitch = std::make_unique<ClickArea>();
    randomizePitch->onClick = [this]()
    {
        auto& a = processorRef.getAPVTS();
        juce::Random rng;
        if (auto* param = a.getParameter(MadshiftaProcessor::TUNE_ID))
            param->setValueNotifyingHost(rng.nextFloat());
        if (auto* param = a.getParameter(MadshiftaProcessor::FINE_ID))
            param->setValueNotifyingHost(rng.nextFloat());
    };
    addAndMakeVisible(*randomizePitch);

    // --- Splash ---

    splashZone = std::make_unique<ClickArea>();
    splashOverlay = std::make_unique<SplashOverlay>(splashImage);
    splashZone->onClick = [this]() { splashOverlay->show(); };
    addAndMakeVisible(*splashZone);
    addChildComponent(*splashOverlay);

    setSize(windowWidth, windowHeight);
    startTimerHz(30);
}

MadshiftaEditor::~MadshiftaEditor()
{
    stopTimer();
}

void MadshiftaEditor::timerCallback()
{
    // Force repaint of tune fader during MIDI so note name displays
    if (processorRef.lastMidiNote.load() >= 0)
    {
        tuneFader->repaint();
    }
}

void MadshiftaEditor::paint(juce::Graphics& g)
{
    if (!backgroundImage.isNull())
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
    else
        g.fillAll(juce::Colours::darkgrey);
}

void MadshiftaEditor::resized()
{
    // Faders - positions from original madshiftaeditor.cpp
    tuneFader->setBounds(28, 98, 25, 100);
    fineTuneFader->setBounds(68, 98, 25, 100);
    dryWetFader->setBounds(339, 98, 23, 126);
    gainFader->setBounds(381, 98, 23, 126);

    // Number boxes
    delayCircle->setBounds(157, 100, 38, 38);
    feedbackCircle->setBounds(236, 100, 38, 38);
    cutoffCircle->setBounds(157, 199, 38, 38);
    resonanceCircle->setBounds(236, 199, 38, 38);
    rootKeyBox->setBounds(20, 227, 38, 17);

    // Toggle buttons
    int filterBtnH = filterModeImage.getHeight() / 2;
    filterModeButton->setBounds(209, 246, filterModeImage.getWidth(), filterBtnH);

    int midiBtnH = midiModeImage.getHeight() / 2;
    midiModeButton->setBounds(61, 223, midiModeImage.getWidth(), midiBtnH);

    // Click areas
    randomizeAll->setBounds(7, 270, 38, 26);
    randomizePitch->setBounds(388, 265, 31, 35);
    splashZone->setBounds(9, 13, 414, 42);

    // Splash overlay covers entire window
    splashOverlay->setBounds(getLocalBounds());
}
