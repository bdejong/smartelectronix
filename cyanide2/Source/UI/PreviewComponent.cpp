#include "PreviewComponent.h"
#include <cmath>

PreviewComponent::PreviewComponent(CShaper& s, const juce::Image& bg)
    : shaper(s), bgImage(bg)
{
    startTimerHz(30);
}

PreviewComponent::~PreviewComponent()
{
    stopTimer();
}

void PreviewComponent::fillSine()
{
    int w = getWidth() - X_PAD;
    if (w <= 0) return;

    sinArray.resize((size_t)w);
    tmpArray.resize((size_t)w);

    for (int i = 0; i < w; i++)
        sinArray[(size_t)i] = (float)std::sin((float)i * 2.0f * 3.1415f / (float)w);
}

void PreviewComponent::timerCallback()
{
    if (dirty)
    {
        dirty = false;
        repaint();
    }
}

void PreviewComponent::paint(juce::Graphics& g)
{
    int w = getWidth();
    int h = getHeight();

    // Ensure arrays are properly sized
    if ((int)sinArray.size() != w - X_PAD)
        fillSine();

    if (sinArray.empty()) return;

    // Draw background region from main bg image
    if (!bgImage.isNull())
    {
        g.drawImage(bgImage, 0, 0, w, h,
                    80, 284, w, h);
    }

    int dataW = w - X_PAD;
    int halfH = h / 2;

    // Center line (black)
    g.setColour(juce::Colours::black);
    g.drawLine((float)(X_PAD / 2), (float)halfH,
               (float)(w - X_PAD / 2), (float)halfH);

    // Original sine wave (black)
    {
        juce::Path sinPath;
        sinPath.startNewSubPath((float)(X_PAD / 2), (float)halfH);
        for (int i = 0; i < dataW; i++)
        {
            float y = (float)halfH - (float)(h - Y_PAD) * sinArray[(size_t)i] * 0.5f;
            sinPath.lineTo((float)(X_PAD / 2 + i), y);
        }
        sinPath.lineTo((float)(w - X_PAD / 2), (float)halfH);
        g.strokePath(sinPath, juce::PathStrokeType(1.0f));
    }

    // Shaped wave (white)
    shaper.process(sinArray.data(), tmpArray.data(), (long)dataW);

    g.setColour(juce::Colours::white);
    {
        juce::Path shapedPath;
        shapedPath.startNewSubPath((float)(X_PAD / 2), (float)halfH);
        for (int i = 0; i < dataW; i++)
        {
            float y = (float)halfH - (float)(h - Y_PAD) * tmpArray[(size_t)i] * 0.5f;
            shapedPath.lineTo((float)(X_PAD / 2 + i), y);
        }
        shapedPath.lineTo((float)(w - X_PAD / 2), (float)halfH);
        g.strokePath(shapedPath, juce::PathStrokeType(1.0f));
    }
}
