#include "SplashOverlay.h"

SplashOverlay::SplashOverlay(const juce::Image& image)
    : splashImage(image)
{
    setInterceptsMouseClicks(true, false);
}

SplashOverlay::~SplashOverlay()
{
}

void SplashOverlay::paint(juce::Graphics& g)
{
    if (visible && !splashImage.isNull())
    {
        g.drawImage(splashImage, getLocalBounds().toFloat());
    }
}

void SplashOverlay::mouseDown(const juce::MouseEvent& /*event*/)
{
    if (visible)
    {
        hide();
    }
}

void SplashOverlay::show()
{
    visible = true;
    setVisible(true);
    toFront(true);
    repaint();
}

void SplashOverlay::hide()
{
    visible = false;
    setVisible(false);
    repaint();
}
