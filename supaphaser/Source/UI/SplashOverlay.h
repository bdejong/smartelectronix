#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class SplashOverlay : public juce::Component
{
public:
    SplashOverlay(const juce::Image& splashImage);
    ~SplashOverlay() override;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;

    void show();
    void hide();
    bool isShowing() const { return visible; }

    void setLinkArea(juce::Rectangle<int> area, const juce::URL& url);

private:
    juce::Image splashImage;
    bool visible = false;
    juce::Rectangle<int> linkArea;
    juce::URL linkUrl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SplashOverlay)
};
