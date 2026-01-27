#include "PluginEditor.h"
#include "BinaryData.h"

SupaPhaserEditor::SupaPhaserEditor(SupaPhaserProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(600, 212);

    // Load images
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::base_png, BinaryData::base_pngSize);
    auto blueKnobImg = juce::ImageCache::getFromMemory(BinaryData::blue_knob1_4_png, BinaryData::blue_knob1_4_pngSize);
    auto distKnobImg = juce::ImageCache::getFromMemory(BinaryData::bigknob01rotated_cropped2_png, BinaryData::bigknob01rotated_cropped2_pngSize);
    auto smallKnobImg = juce::ImageCache::getFromMemory(BinaryData::small_knob02_png, BinaryData::small_knob02_pngSize);
    auto greyTextImg = juce::ImageCache::getFromMemory(BinaryData::type_greyback_white_png, BinaryData::type_greyback_white_pngSize);
    auto whiteTextImg = juce::ImageCache::getFromMemory(BinaryData::type_whiteback_png, BinaryData::type_whiteback_pngSize);
    auto onOffImg = juce::ImageCache::getFromMemory(BinaryData::extend_on_off_png, BinaryData::extend_on_off_pngSize);
    auto sliderImg = juce::ImageCache::getFromMemory(BinaryData::slider_png, BinaryData::slider_pngSize);
    auto headsImg = juce::ImageCache::getFromMemory(BinaryData::heads_png, BinaryData::heads_pngSize);
    auto splashImg = juce::ImageCache::getFromMemory(BinaryData::splash_png, BinaryData::splash_pngSize);

    int blueKnobW = blueKnobImg.getWidth();
    int blueKnobH = blueKnobImg.getHeight() / 75;

    juce::Colour grey(118, 118, 118);
    juce::Colour white(255, 255, 255);

    int displayWidth = 30;
    int displayHeight = 6;

    // Helper to create a blue knob
    auto makeBlueKnob = [&](std::unique_ptr<FilmStripKnob>& knob, int x, int y) {
        knob = std::make_unique<FilmStripKnob>(blueKnobImg, 75);
        knob->setBounds(x, y, blueKnobW, blueKnobH);
        addAndMakeVisible(*knob);
    };

    // Helper to create grey text display
    auto makeGreyDisplay = [&](std::unique_ptr<TextDisplay>& disp, int x, int y, int w, int h) {
        disp = std::make_unique<TextDisplay>(greyTextImg, grey);
        disp->setBounds(x, y, w, h);
        addAndMakeVisible(*disp);
    };

    // Upper row - envelope section
    makeBlueKnob(attackKnob, 106, 43);
    makeGreyDisplay(attackDisplay, 102, 89, 27, displayHeight);

    makeBlueKnob(releaseKnob, 162, 43);
    makeGreyDisplay(releaseDisplay, 161, 89, 27, displayHeight);

    makeBlueKnob(envMinKnob, 219, 43);
    makeGreyDisplay(envMinDisplay, 224, 89, displayWidth - 3, displayHeight);

    makeBlueKnob(envMaxKnob, 276, 43);
    makeGreyDisplay(envMaxDisplay, 280, 89, displayWidth - 3, displayHeight);

    // Lower row - LFO section
    makeBlueKnob(freqKnob, 106, 135);
    makeGreyDisplay(freqDisplay, 102, 119, 28, displayHeight);

    makeBlueKnob(stereoKnob, 162, 135);
    makeGreyDisplay(stereoDisplay, 167, 119, displayWidth - 4, displayHeight);

    makeBlueKnob(lfoMinKnob, 219, 135);
    makeGreyDisplay(lfoMinDisplay, 224, 119, displayWidth - 3, displayHeight);

    makeBlueKnob(lfoMaxKnob, 276, 135);
    makeGreyDisplay(lfoMaxDisplay, 280, 119, displayWidth - 3, displayHeight);

    // Feedback
    makeBlueKnob(feedbackKnob, 452, 48);
    makeGreyDisplay(feedbackDisplay, 461, 94, displayWidth - 7, displayHeight);

    // Stages
    makeBlueKnob(stagesKnob, 452, 131);
    makeGreyDisplay(stagesDisplay, 465, 115, displayWidth - 11, displayHeight);

    // Distortion (big knob, 111 frames, 68px)
    distortKnob = std::make_unique<FilmStripKnob>(distKnobImg, 111);
    distortKnob->setBounds(349, 72, distKnobImg.getWidth(), distKnobImg.getHeight() / 111);
    addAndMakeVisible(*distortKnob);

    // Mixture (small knob, 62 frames, 38px)
    mixtureKnob = std::make_unique<FilmStripKnob>(smallKnobImg, 62);
    mixtureKnob->setInverseBitmap(true);
    mixtureKnob->setBounds(28, 87, smallKnobImg.getWidth(), smallKnobImg.getHeight() / 62);
    addAndMakeVisible(*mixtureKnob);

    // Mixture displays (white text)
    mixtureDisplay1 = std::make_unique<TextDisplay>(whiteTextImg, white);
    mixtureDisplay1->setBounds(30, 68, 25, 6);
    addAndMakeVisible(*mixtureDisplay1);

    mixtureDisplay2 = std::make_unique<TextDisplay>(whiteTextImg, white);
    mixtureDisplay2->setBounds(30, 139, 25, 6);
    addAndMakeVisible(*mixtureDisplay2);

    // Extend toggle (2 frames)
    extendToggle = std::make_unique<FilmStripKnob>(onOffImg, 2, true);
    extendToggle->setBounds(326, 155, onOffImg.getWidth(), onOffImg.getHeight() / 2);
    addAndMakeVisible(*extendToggle);

    // Dry/Wet slider
    int sliderHandleH = sliderImg.getHeight();
    int sliderHandleW = sliderImg.getWidth();
    dryWetSlider = std::make_unique<FilmStripSlider>(sliderImg, 0, 160 - 45, 1);
    dryWetSlider->setBounds(525, 45, sliderHandleW + 1, 160 - 45 + sliderHandleH);
    addAndMakeVisible(*dryWetSlider);

    // Gain slider
    gainSlider = std::make_unique<FilmStripSlider>(sliderImg, 0, 160 - 45, 2);
    gainSlider->setBounds(553, 45, sliderHandleW + 1, 160 - 45 + sliderHandleH);
    addAndMakeVisible(*gainSlider);

    // Gain display (white text, middle aligned)
    gainDisplay = std::make_unique<TextDisplay>(whiteTextImg, white);
    gainDisplay->setBounds(549, 174, 23, 5);
    gainDisplay->setMiddleAligned(true);
    gainDisplay->setRightAligned(false);
    addAndMakeVisible(*gainDisplay);

    // Dry/wet display - not shown in original, but let's skip it (no dryWetDisplay in original editor)

    // Invert button (4 states)
    invertButton = std::make_unique<MultiStateButton>(headsImg, 4);
    invertButton->setBounds(319, 22, headsImg.getWidth(), headsImg.getHeight() / 4);
    addAndMakeVisible(*invertButton);

    // Splash
    splashOverlay = std::make_unique<SplashOverlay>(splashImg);
    splashOverlay->setBounds(201, 44, splashImg.getWidth(), splashImg.getHeight());
    splashOverlay->setLinkArea(juce::Rectangle<int>(5, 55, 155, 20),
                               juce::URL("https://paypal.me/BramdeJong"));
    addChildComponent(*splashOverlay);

    splashArea = std::make_unique<ClickArea>();
    splashArea->setBounds(396, 6, 120, 23);
    splashArea->onClick = [this]() { splashOverlay->show(); };
    addAndMakeVisible(*splashArea);

    // Create attachments
    attackAtt   = std::make_unique<FilmStripKnobAttachment>(p.apvts, SupaPhaserProcessor::PARAM_ATTACK, *attackKnob);
    releaseAtt  = std::make_unique<FilmStripKnobAttachment>(p.apvts, SupaPhaserProcessor::PARAM_RELEASE, *releaseKnob);
    envMinAtt   = std::make_unique<FilmStripKnobAttachment>(p.apvts, SupaPhaserProcessor::PARAM_MIN_ENV, *envMinKnob);
    envMaxAtt   = std::make_unique<FilmStripKnobAttachment>(p.apvts, SupaPhaserProcessor::PARAM_MAX_ENV, *envMaxKnob);
    freqAtt     = std::make_unique<FilmStripKnobAttachment>(p.apvts, SupaPhaserProcessor::PARAM_FREQ, *freqKnob);
    stereoAtt   = std::make_unique<FilmStripKnobAttachment>(p.apvts, SupaPhaserProcessor::PARAM_STEREO, *stereoKnob);
    lfoMinAtt   = std::make_unique<FilmStripKnobAttachment>(p.apvts, SupaPhaserProcessor::PARAM_MIN_FREQ, *lfoMinKnob);
    lfoMaxAtt   = std::make_unique<FilmStripKnobAttachment>(p.apvts, SupaPhaserProcessor::PARAM_MAX_FREQ, *lfoMaxKnob);
    feedbackAtt = std::make_unique<FilmStripKnobAttachment>(p.apvts, SupaPhaserProcessor::PARAM_FEED, *feedbackKnob);
    stagesAtt   = std::make_unique<FilmStripKnobAttachment>(p.apvts, SupaPhaserProcessor::PARAM_STAGES, *stagesKnob);
    distortAtt  = std::make_unique<FilmStripKnobAttachment>(p.apvts, SupaPhaserProcessor::PARAM_DISTORT, *distortKnob);
    mixtureAtt  = std::make_unique<FilmStripKnobAttachment>(p.apvts, SupaPhaserProcessor::PARAM_MIXTURE, *mixtureKnob);
    dryWetAtt   = std::make_unique<FilmStripSliderAttachment>(p.apvts, SupaPhaserProcessor::PARAM_DRY_WET, *dryWetSlider);
    gainAtt     = std::make_unique<FilmStripSliderAttachment>(p.apvts, SupaPhaserProcessor::PARAM_GAIN, *gainSlider);
    extendAtt   = std::make_unique<FilmStripKnobAttachment>(p.apvts, SupaPhaserProcessor::PARAM_EXTEND, *extendToggle);
    invertAtt   = std::make_unique<MultiStateButtonAttachment>(p.apvts, SupaPhaserProcessor::PARAM_INVERT, *invertButton);

    // Initial display update
    updateDisplays();

    // Start timer for display updates at 30Hz
    startTimerHz(30);
}

SupaPhaserEditor::~SupaPhaserEditor()
{
    stopTimer();
}

void SupaPhaserEditor::paint(juce::Graphics& g)
{
    if (!backgroundImage.isNull())
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
}

void SupaPhaserEditor::timerCallback()
{
    updateDisplays();
}

void SupaPhaserEditor::updateDisplays()
{
    attackDisplay->setText(processorRef.getDisplayText(SupaPhaserProcessor::PARAM_ATTACK));
    releaseDisplay->setText(processorRef.getDisplayText(SupaPhaserProcessor::PARAM_RELEASE));
    envMinDisplay->setText(processorRef.getDisplayText(SupaPhaserProcessor::PARAM_MIN_ENV));
    envMaxDisplay->setText(processorRef.getDisplayText(SupaPhaserProcessor::PARAM_MAX_ENV));
    freqDisplay->setText(processorRef.getDisplayText(SupaPhaserProcessor::PARAM_FREQ));
    stereoDisplay->setText(processorRef.getDisplayText(SupaPhaserProcessor::PARAM_STEREO));
    lfoMinDisplay->setText(processorRef.getDisplayText(SupaPhaserProcessor::PARAM_MIN_FREQ));
    lfoMaxDisplay->setText(processorRef.getDisplayText(SupaPhaserProcessor::PARAM_MAX_FREQ));
    feedbackDisplay->setText(processorRef.getDisplayText(SupaPhaserProcessor::PARAM_FEED));
    stagesDisplay->setText(processorRef.getDisplayText(SupaPhaserProcessor::PARAM_STAGES));
    gainDisplay->setText(processorRef.getDisplayText(SupaPhaserProcessor::PARAM_GAIN));

    // Mixture has dual display
    float mix = 0.f;
    if (auto* p = processorRef.apvts.getRawParameterValue(SupaPhaserProcessor::PARAM_MIXTURE))
        mix = p->load();
    int mixPct = (int)(mix * 100.f);
    mixtureDisplay1->setText(juce::String(mixPct) + "%");
    mixtureDisplay2->setText(juce::String(100 - mixPct) + "%");
}
