#include "PluginEditor.h"
#include "BinaryData.h"

static const int nDisp = 12;
static const char* disp[nDisp] = {
    "Like this? Make a donation!", "Welcome back!", "Crunch that beat!",
    "Zap that beat!!", "Zap that beat!!", "Zap that beat!!",
    "Zap that beat!!", "Zap that beat!!", "Stop reopening me!",
    "Nooooooo!", "Have MERCY!!", "*arghl* I'm dead."
};

Cyanide2Editor::Cyanide2Editor(Cyanide2Processor& p)
    : AudioProcessorEditor(p), processorRef(p)
{
    // Load images
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::background_png,
                                                       BinaryData::background_pngSize);
    auto knobHandleImg = juce::ImageCache::getFromMemory(BinaryData::knob_handle_png,
                                                          BinaryData::knob_handle_pngSize);
    auto switchImg = juce::ImageCache::getFromMemory(BinaryData::switch_png,
                                                      BinaryData::switch_pngSize);
    auto guidesImg = juce::ImageCache::getFromMemory(BinaryData::guides_png,
                                                      BinaryData::guides_pngSize);
    auto resetImg = juce::ImageCache::getFromMemory(BinaryData::reset_png,
                                                     BinaryData::reset_pngSize);
    auto shaperHandleImg = juce::ImageCache::getFromMemory(BinaryData::shaper_handle_png,
                                                            BinaryData::shaper_handle_pngSize);
    auto splashImg = juce::ImageCache::getFromMemory(BinaryData::splash_png,
                                                      BinaryData::splash_pngSize);

    // Create rotary knobs (background is part of the main background image)
    preGainKnob = std::make_unique<RotaryKnob>(knobHandleImg);
    preTypeKnob = std::make_unique<RotaryKnob>(knobHandleImg);
    preFilterKnob = std::make_unique<RotaryKnob>(knobHandleImg);
    postGainKnob = std::make_unique<RotaryKnob>(knobHandleImg);
    postTypeKnob = std::make_unique<RotaryKnob>(knobHandleImg);
    postFilterKnob = std::make_unique<RotaryKnob>(knobHandleImg);
    dryWetKnob = std::make_unique<RotaryKnob>(knobHandleImg);

    addAndMakeVisible(*preGainKnob);
    addAndMakeVisible(*preTypeKnob);
    addAndMakeVisible(*preFilterKnob);
    addAndMakeVisible(*postGainKnob);
    addAndMakeVisible(*postTypeKnob);
    addAndMakeVisible(*postFilterKnob);
    addAndMakeVisible(*dryWetKnob);

    // Create attachments
    preGainAttach = std::make_unique<RotaryKnobAttachment>(p.apvts, Cyanide2Processor::PARAM_PREGAIN, *preGainKnob);
    preTypeAttach = std::make_unique<RotaryKnobAttachment>(p.apvts, Cyanide2Processor::PARAM_PRETYPE, *preTypeKnob);
    preFilterAttach = std::make_unique<RotaryKnobAttachment>(p.apvts, Cyanide2Processor::PARAM_PREFILTER, *preFilterKnob);
    postGainAttach = std::make_unique<RotaryKnobAttachment>(p.apvts, Cyanide2Processor::PARAM_POSTGAIN, *postGainKnob);
    postTypeAttach = std::make_unique<RotaryKnobAttachment>(p.apvts, Cyanide2Processor::PARAM_POSTTYPE, *postTypeKnob);
    postFilterAttach = std::make_unique<RotaryKnobAttachment>(p.apvts, Cyanide2Processor::PARAM_POSTFILTER, *postFilterKnob);
    dryWetAttach = std::make_unique<RotaryKnobAttachment>(p.apvts, Cyanide2Processor::PARAM_DRYWET, *dryWetKnob);

    // Oversample toggle (2-frame film strip)
    oversampleButton = std::make_unique<FilmStripKnob>(switchImg, 2, true);
    addAndMakeVisible(*oversampleButton);
    oversampleAttach = std::make_unique<FilmStripKnobAttachment>(p.apvts, Cyanide2Processor::PARAM_OVERSAMPLE, *oversampleButton);

    // Guides toggle (local state only)
    guidesButton = std::make_unique<FilmStripKnob>(guidesImg, 2, true);
    addAndMakeVisible(*guidesButton);

    // Reset button (momentary)
    resetButton = std::make_unique<FilmStripKnob>(resetImg, 2, true);
    addAndMakeVisible(*resetButton);

    // Shaper view
    shaperView = std::make_unique<ShaperViewComponent>(p.getShaper(), backgroundImage, shaperHandleImg);
    addAndMakeVisible(*shaperView);

    // Preview
    previewComponent = std::make_unique<PreviewComponent>(p.getShaper(), backgroundImage);
    addAndMakeVisible(*previewComponent);

    // Info label
    infoLabel = std::make_unique<juce::Label>("info", disp[openedCount++ % nDisp]);
    infoLabel->setFont(juce::FontOptions(12.0f));
    infoLabel->setColour(juce::Label::textColourId, juce::Colours::black);
    infoLabel->setColour(juce::Label::backgroundColourId, juce::Colours::white);
    infoLabel->setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(*infoLabel);

    // Splash overlay
    splashOverlay = std::make_unique<SplashOverlay>(splashImg);
    addChildComponent(*splashOverlay);
    splashTriggerArea = juce::Rectangle<int>(0, 0, 380, 45);

    // Connect guides button
    guidesButton->onValueChange = [this](float val)
    {
        if (shaperView)
            shaperView->setGuides(val > 0.5f);
    };

    // Connect reset button
    resetButton->onValueChange = [this](float val)
    {
        if (val > 0.5f)
        {
            if (shaperView)
            {
                shaperView->reset();
                if (previewComponent)
                    previewComponent->markDirty();
            }
        }
        // Reset the button back to off
        resetButton->setValue(0.0f);
    };

    // Connect shaper changes to preview
    shaperView->onChanged = [this]()
    {
        if (previewComponent)
            previewComponent->markDirty();
    };

    // Connect shaper selection to info label
    shaperView->onSelectionChanged = [this]()
    {
        float x, y;
        if (shaperView->getSelectedXY(x, y))
        {
            infoLabel->setText(juce::String(x, 3) + " , " + juce::String(y, 3),
                              juce::dontSendNotification);
        }
    };

    // Listen for parameter changes to update info label
    p.apvts.addParameterListener(Cyanide2Processor::PARAM_PREGAIN, this);
    p.apvts.addParameterListener(Cyanide2Processor::PARAM_PREFILTER, this);
    p.apvts.addParameterListener(Cyanide2Processor::PARAM_POSTGAIN, this);
    p.apvts.addParameterListener(Cyanide2Processor::PARAM_POSTFILTER, this);
    p.apvts.addParameterListener(Cyanide2Processor::PARAM_PRETYPE, this);
    p.apvts.addParameterListener(Cyanide2Processor::PARAM_POSTTYPE, this);
    p.apvts.addParameterListener(Cyanide2Processor::PARAM_DRYWET, this);
    p.apvts.addParameterListener(Cyanide2Processor::PARAM_OVERSAMPLE, this);

    startTimer(2000);

    // setSize must be last — it triggers resized() which dereferences all component pointers
    setSize(380, 350);
}

Cyanide2Editor::~Cyanide2Editor()
{
    stopTimer();

    processorRef.apvts.removeParameterListener(Cyanide2Processor::PARAM_PREGAIN, this);
    processorRef.apvts.removeParameterListener(Cyanide2Processor::PARAM_PREFILTER, this);
    processorRef.apvts.removeParameterListener(Cyanide2Processor::PARAM_POSTGAIN, this);
    processorRef.apvts.removeParameterListener(Cyanide2Processor::PARAM_POSTFILTER, this);
    processorRef.apvts.removeParameterListener(Cyanide2Processor::PARAM_PRETYPE, this);
    processorRef.apvts.removeParameterListener(Cyanide2Processor::PARAM_POSTTYPE, this);
    processorRef.apvts.removeParameterListener(Cyanide2Processor::PARAM_DRYWET, this);
    processorRef.apvts.removeParameterListener(Cyanide2Processor::PARAM_OVERSAMPLE, this);
}

void Cyanide2Editor::paint(juce::Graphics& g)
{
    if (!backgroundImage.isNull())
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
}

void Cyanide2Editor::resized()
{
    int knobW = preGainKnob->getWidth() > 0 ? preGainKnob->getWidth() : 27;
    int knobH = knobW; // knobs are square (background image)

    // Read actual knob image size
    // The knob_back.bmp is 27x27 from original
    knobW = 27;
    knobH = 27;

    preGainKnob->setBounds(24, 100, knobW, knobH);
    preTypeKnob->setBounds(24, 159, knobW, knobH);
    preFilterKnob->setBounds(24, 222, knobW, knobH);
    postGainKnob->setBounds(329, 100, knobW, knobH);
    postTypeKnob->setBounds(329, 159, knobW, knobH);
    postFilterKnob->setBounds(329, 222, knobW, knobH);
    dryWetKnob->setBounds(329, 306, knobW, knobH);

    // Oversample toggle (14x28 image, 2 frames of 14x14)
    int switchW = 14;
    int switchH = 14;
    oversampleButton->setBounds(14, 291, switchW, switchH);

    // Guides and reset (10x22 images, 2 frames of 10x11)
    int guidesW = 10;
    int guidesH = 11;
    guidesButton->setBounds(292, 270, guidesW, guidesH);

    int resetW = 10;
    int resetH = 11;
    resetButton->setBounds(278, 270, resetW, resetH);

    // Shaper view: (80,45) to (302,267) = 222x222
    shaperView->setBounds(80, 45, 222, 222);

    // Preview: (80,284) to (302,344) = 222x60
    previewComponent->setBounds(80, 284, 222, 60);

    // Info label
    infoLabel->setBounds(237, 25, 140, 16);

    // Splash overlay covers entire window
    splashOverlay->setBounds(getLocalBounds());
}

void Cyanide2Editor::mouseDown(const juce::MouseEvent& event)
{
    auto pos = event.getPosition();
    if (splashTriggerArea.contains(pos))
    {
        splashOverlay->show();
    }
}

void Cyanide2Editor::timerCallback()
{
    // Reset display after idle
    infoLabel->setText(disp[openedCount % nDisp], juce::dontSendNotification);
}

void Cyanide2Editor::parameterChanged(const juce::String& parameterID, float /*newValue*/)
{
    juce::MessageManager::callAsync([this, parameterID]()
    {
        updateDisplayText(parameterID);
        // Refresh shaper UI when parameters change (e.g., preset load)
        if (shaperView)
            shaperView->repaint();
        if (previewComponent)
            previewComponent->markDirty();
    });
}

void Cyanide2Editor::updateDisplayText(const juce::String& paramId)
{
    stopTimer();
    startTimer(2000);

    auto text = processorRef.getDisplayText(paramId);
    if (text.isNotEmpty())
        infoLabel->setText(text, juce::dontSendNotification);
}
