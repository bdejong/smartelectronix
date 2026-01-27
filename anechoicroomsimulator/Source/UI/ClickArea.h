#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Simple invisible clickable area - like original CSplashScreen trigger zone
class ClickArea : public juce::Component
{
public:
    ClickArea() { setInterceptsMouseClicks(true, false); }

    void mouseDown(const juce::MouseEvent&) override
    {
        if (onClick)
            onClick();
    }

    std::function<void()> onClick;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClickArea)
};
