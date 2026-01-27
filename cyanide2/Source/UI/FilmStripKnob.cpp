#include "FilmStripKnob.h"
#include <cmath>

FilmStripKnob::FilmStripKnob(const juce::Image& strip, int frames, bool toggle)
    : filmStrip(strip), numFrames(frames), isToggle(toggle)
{
    frameHeight = filmStrip.getHeight() / numFrames;
    setRepaintsOnMouseActivity(true);
}

FilmStripKnob::~FilmStripKnob()
{
}

void FilmStripKnob::paint(juce::Graphics& g)
{
    if (filmStrip.isNull())
        return;

    // Calculate which frame to show based on value
    float normalizedValue = (value - minValue) / (maxValue - minValue);
    int frameIndex = static_cast<int>(normalizedValue * (numFrames - 1) + 0.5f);
    frameIndex = juce::jlimit(0, numFrames - 1, frameIndex);

    // Draw the frame
    g.drawImage(filmStrip,
                0, 0, getWidth(), getHeight(),
                0, frameIndex * frameHeight, filmStrip.getWidth(), frameHeight);
}

void FilmStripKnob::mouseDown(const juce::MouseEvent& event)
{
    if (isToggle)
    {
        // Toggle behavior
        setValue(value < 0.5f ? 1.0f : 0.0f);
        if (onValueChange)
            onValueChange(value);
    }
    else
    {
        // Start drag
        dragStartValue = value;
        dragStartY = event.getPosition().y;
    }
}

void FilmStripKnob::mouseDrag(const juce::MouseEvent& event)
{
    if (isToggle)
        return;

    // Linear vertical drag mode (matching VSTGUI kLinearMode)
    int deltaY = dragStartY - event.getPosition().y;
    float newValue = dragStartValue + deltaY * dragSensitivity * (maxValue - minValue);
    newValue = juce::jlimit(minValue, maxValue, newValue);

    if (std::fabsf(newValue - value) > 1e-10f)
    {
        value = newValue;
        repaint();
        if (onValueChange)
            onValueChange(value);
    }
}

void FilmStripKnob::mouseUp(const juce::MouseEvent& /*event*/)
{
    // Nothing specific needed
}

void FilmStripKnob::setValue(float newValue)
{
    newValue = juce::jlimit(minValue, maxValue, newValue);
    if (std::fabsf(value - newValue) > 1e-10f)
    {
        value = newValue;
        repaint();
    }
}

void FilmStripKnob::setRange(float minVal, float maxVal)
{
    minValue = minVal;
    maxValue = maxVal;
}

// FilmStripKnobAttachment implementation
FilmStripKnobAttachment::FilmStripKnobAttachment(
    juce::AudioProcessorValueTreeState& state,
    const juce::String& parameterID,
    FilmStripKnob& knobRef)
    : apvts(state), paramId(parameterID), knob(knobRef)
{
    // Get initial value from parameter
    if (auto* param = apvts.getParameter(parameterID))
    {
        knob.setValue(param->getValue());
    }

    // Listen for parameter changes (from automation, presets, etc.)
    apvts.addParameterListener(parameterID, this);

    // When knob changes, update parameter
    knob.onValueChange = [this, parameterID](float newValue)
    {
        if (!isUpdating)
        {
            isUpdating = true;
            if (auto* param = this->apvts.getParameter(parameterID))
            {
                param->setValueNotifyingHost(newValue);
            }
            isUpdating = false;
        }
    };
}

FilmStripKnobAttachment::~FilmStripKnobAttachment()
{
    apvts.removeParameterListener(paramId, this);
}

void FilmStripKnobAttachment::parameterChanged(const juce::String& /*parameterID*/, float newValue)
{
    if (!isUpdating)
    {
        isUpdating = true;
        // Update on message thread for thread safety
        juce::MessageManager::callAsync([this, newValue]()
        {
            knob.setValue(newValue);
            isUpdating = false;
        });
    }
}
