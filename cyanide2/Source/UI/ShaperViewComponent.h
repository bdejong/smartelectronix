#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../DSP/Shaper.h"
#include "../DSP/Spline.h"
#include <array>

static constexpr float DISTANCE = 0.002f;
static constexpr int N_LINES = 200;

class ShaperViewComponent : public juce::Component
{
public:
    ShaperViewComponent(CShaper& shaper, const juce::Image& backgroundImage,
                        const juce::Image& handleImage);
    ~ShaperViewComponent() override;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    void setGuides(bool g);
    void reset();

    bool getSelectedXY(float& x, float& y) const;

    std::function<void()> onChanged;
    std::function<void()> onSelectionChanged;

private:
    void convertToScreen(SplinePoint p, float& sx, float& sy);
    void convertFromScreen(float sx, float sy, SplinePoint& p);

    CShaper& shaper;
    juce::Image bgImage;
    juce::Image handleImg;

    float minx, miny, maxx, maxy;
    long selectedIndex = -1;
    bool guides = false;

    std::array<SplinePoint, N_LINES> lines {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShaperViewComponent)
};
