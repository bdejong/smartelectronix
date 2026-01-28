#include "RotaryKnob.h"
#include <cmath>

RotaryKnob::RotaryKnob(const juce::Image& handle)
    : handleImage(handle)
{
    setRepaintsOnMouseActivity(true);
}

RotaryKnob::~RotaryKnob()
{
}

void RotaryKnob::paint(juce::Graphics& g)
{
    // Draw handle positioned on circular arc (VSTGUI CKnob style)
    // startAngle = 3pi/4 (135°), rangeAngle = 3pi/2 (270°)
    if (!handleImage.isNull())
    {
        static constexpr float startAngle = 3.0f * juce::MathConstants<float>::pi / 4.0f;
        static constexpr float rangeAngle = 3.0f * juce::MathConstants<float>::pi / 2.0f;
        static constexpr int inset = 3;

        float alpha = startAngle + value * rangeAngle;

        float cx = getWidth() * 0.5f;
        float cy = getHeight() * 0.5f;
        float hw = handleImage.getWidth() * 0.5f;
        float hh = handleImage.getHeight() * 0.5f;
        float radius = cx - hw - (float)inset;

        float hx = cx + std::cos(alpha) * radius - hw;
        float hy = cy + std::sin(alpha) * radius - hh;

        g.drawImage(handleImage, (int)hx, (int)hy,
                    handleImage.getWidth(), handleImage.getHeight(),
                    0, 0, handleImage.getWidth(), handleImage.getHeight());
    }
}

void RotaryKnob::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isCtrlDown() || event.mods.isCommandDown())
    {
        setValue(defaultValue);
        if (onValueChange)
            onValueChange(value);
        return;
    }

    dragStartValue = value;
    dragStartY = event.getPosition().y;
}

void RotaryKnob::mouseDrag(const juce::MouseEvent& event)
{
    int deltaY = dragStartY - event.getPosition().y;
    float newValue = dragStartValue + deltaY * dragSensitivity;
    newValue = juce::jlimit(0.0f, 1.0f, newValue);

    if (std::fabs(newValue - value) > 1e-10f)
    {
        value = newValue;
        repaint();
        if (onValueChange)
            onValueChange(value);
    }
}

void RotaryKnob::mouseDoubleClick(const juce::MouseEvent&)
{
    setValue(defaultValue);
    if (onValueChange)
        onValueChange(value);
}

void RotaryKnob::setValue(float newValue)
{
    newValue = juce::jlimit(0.0f, 1.0f, newValue);
    if (std::fabs(value - newValue) > 1e-10f)
    {
        value = newValue;
        repaint();
    }
}

// RotaryKnobAttachment implementation
RotaryKnobAttachment::RotaryKnobAttachment(
    juce::AudioProcessorValueTreeState& state,
    const juce::String& parameterID,
    RotaryKnob& knobRef)
    : apvts(state), paramId(parameterID), knob(knobRef)
{
    if (auto* param = apvts.getParameter(parameterID))
    {
        knob.setValue(param->getValue());
        knob.setDefaultValue(param->getDefaultValue());
    }

    apvts.addParameterListener(parameterID, this);

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

RotaryKnobAttachment::~RotaryKnobAttachment()
{
    apvts.removeParameterListener(paramId, this);
}

void RotaryKnobAttachment::parameterChanged(const juce::String& /*parameterID*/, float newValue)
{
    if (!isUpdating)
    {
        isUpdating = true;
        juce::MessageManager::callAsync([this, newValue]()
        {
            knob.setValue(newValue);
            isUpdating = false;
        });
    }
}
