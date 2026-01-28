#include "FilmStripSlider.h"
#include <cmath>

FilmStripSlider::FilmStripSlider(const juce::Image& handle, int minPos, int maxPos, int offsetX)
    : handleImage(handle), minY(minPos), maxY(maxPos), handleOffsetX(offsetX)
{
    setRepaintsOnMouseActivity(true);
}

void FilmStripSlider::paint(juce::Graphics& g)
{
    if (handleImage.isNull())
        return;

    // Value 1.0 = top (minY), value 0.0 = bottom (maxY)
    int handleTop = maxY - static_cast<int>((maxY - minY) * value + 0.5f);
    int handleX = handleOffsetX;

    g.drawImage(handleImage,
                handleX, handleTop, handleImage.getWidth(), handleImage.getHeight(),
                0, 0, handleImage.getWidth(), handleImage.getHeight());
}

void FilmStripSlider::mouseDown(const juce::MouseEvent& event)
{
    float newValue = 1.0f - static_cast<float>(event.getPosition().y - minY) / static_cast<float>(maxY - minY);
    newValue = juce::jlimit(0.0f, 1.0f, newValue);
    if (std::fabs(newValue - value) > 1e-10f)
    {
        value = newValue;
        repaint();
        if (onValueChange)
            onValueChange(value);
    }
}

void FilmStripSlider::mouseDrag(const juce::MouseEvent& event)
{
    mouseDown(event);
}

void FilmStripSlider::setValue(float newValue)
{
    newValue = juce::jlimit(0.0f, 1.0f, newValue);
    if (std::fabs(value - newValue) > 1e-10f)
    {
        value = newValue;
        repaint();
    }
}

FilmStripSliderAttachment::FilmStripSliderAttachment(
    juce::AudioProcessorValueTreeState& state,
    const juce::String& parameterID,
    FilmStripSlider& sliderRef)
    : apvts(state), paramId(parameterID), slider(sliderRef)
{
    if (auto* param = apvts.getParameter(parameterID))
        slider.setValue(param->getValue());

    apvts.addParameterListener(parameterID, this);

    slider.onValueChange = [this, parameterID](float newValue)
    {
        if (!isUpdating)
        {
            isUpdating = true;
            if (auto* param = this->apvts.getParameter(parameterID))
                param->setValueNotifyingHost(newValue);
            isUpdating = false;
        }
    };
}

FilmStripSliderAttachment::~FilmStripSliderAttachment()
{
    apvts.removeParameterListener(paramId, this);
}

void FilmStripSliderAttachment::parameterChanged(const juce::String& /*parameterID*/, float newValue)
{
    if (!isUpdating)
    {
        isUpdating = true;
        juce::MessageManager::callAsync([this, newValue]()
        {
            slider.setValue(newValue);
            isUpdating = false;
        });
    }
}
