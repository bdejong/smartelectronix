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

private:
    juce::Image splashImage;
    bool visible = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SplashOverlay)
};
