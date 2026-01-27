#include "PluginEditor.h"

struct ClickArea : public juce::Component
{
    std::function<void()> onClick;
    void mouseDown(const juce::MouseEvent&) override { if (onClick) onClick(); }
};

BitmurdererEditor::BitmurdererEditor(BitmurdererProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(428, 220);

    backgroundImage = juce::ImageCache::getFromMemory(
        BinaryData::bitmurderer_png, BinaryData::bitmurderer_pngSize);

    auto bitImage = juce::ImageCache::getFromMemory(
        BinaryData::_1_png, BinaryData::_1_pngSize);
    auto modeImage = juce::ImageCache::getFromMemory(
        BinaryData::_2_png, BinaryData::_2_pngSize);

    // Bit toggle positions (5 columns x 3 rows)
    const int bitX[] = { 160, 205, 250, 295, 340 };
    const int bitY[] = { 7, 77, 147 };

    for (int i = 0; i < 15; ++i)
    {
        buttons[i] = std::make_unique<FilmStripKnob>(bitImage, 2, true);
        int col = i % 5;
        int row = i / 5;
        buttons[i]->setBounds(bitX[col], bitY[row], 45, 70);
        addAndMakeVisible(buttons[i].get());
        attachments[i] = std::make_unique<FilmStripKnobAttachment>(
            processor.apvts, "bit" + juce::String(i), *buttons[i]);
    }

    // Mode toggles: AND, OR, Sigmoid
    struct { const char* id; int x, y; } modes[] = {
        { "andMode",  70, 75 },
        { "orMode",   10, 100 },
        { "sigmoid",  70, 150 }
    };

    for (int i = 0; i < 3; ++i)
    {
        int idx = 15 + i;
        buttons[idx] = std::make_unique<FilmStripKnob>(modeImage, 2, true);
        buttons[idx]->setBounds(modes[i].x, modes[i].y, 55, 55);
        addAndMakeVisible(buttons[idx].get());
        attachments[idx] = std::make_unique<FilmStripKnobAttachment>(
            processor.apvts, modes[i].id, *buttons[idx]);
    }

    // Splash overlay
    auto splashImage = juce::ImageCache::getFromMemory(
        BinaryData::credits_png, BinaryData::credits_pngSize);
    splashOverlay = std::make_unique<SplashOverlay>(splashImage);
    splashOverlay->setBounds(0, 0, 428, 220);
    addChildComponent(splashOverlay.get());

    // Splash trigger area
    auto trigger = std::make_unique<ClickArea>();
    trigger->setBounds(395, 165, 33, 36);
    trigger->onClick = [this]() { splashOverlay->show(); };
    addAndMakeVisible(trigger.get());
    splashTrigger = std::move(trigger);
}

BitmurdererEditor::~BitmurdererEditor() {}

void BitmurdererEditor::paint(juce::Graphics& g)
{
    if (!backgroundImage.isNull())
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
}
