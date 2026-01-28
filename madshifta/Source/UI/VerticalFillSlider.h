#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class VerticalFillSlider : public juce::Component
{
public:
    VerticalFillSlider(float axis, float defaultValue,
                       std::function<juce::String(float)> stringConverter = nullptr)
        : axis(axis), defaultValue(defaultValue), stringConverter(std::move(stringConverter))
    {
        setOpaque(true);
    }

    void paint(juce::Graphics& g) override
    {
        auto w = getWidth();
        auto h = getHeight();

        // Black background
        g.setColour(juce::Colour(0, 0, 0));
        g.fillRect(getLocalBounds());

        // Red fill from axis to current value
        g.setColour(juce::Colour(239, 0, 0));
        int axisY = h - static_cast<int>(h * axis);
        int valueY = h - static_cast<int>(h * value);

        int top, bottom;
        if (value > axis)
        {
            top = valueY;
            bottom = axisY;
        }
        else
        {
            top = axisY;
            bottom = valueY;
        }
        g.fillRect(0, top, w, bottom - top);

        // Text display at textY position
        juce::String text;
        if (stringConverter)
            text = stringConverter(value);
        else
            text = juce::String(value, 2);

        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(10.0f));
        g.drawText(text, 0, textY, w, textHeight, juce::Justification::centred, false);
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        if (event.mods.isCtrlDown() || event.getNumberOfClicks() == 2)
        {
            value = defaultValue;
            repaint();
            if (onValueChange)
                onValueChange(value);
            return;
        }
        dragStartValue = value;
        dragStartY = event.getPosition().y;
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        int deltaY = dragStartY - event.getPosition().y;
        float newValue = dragStartValue + deltaY * 0.005f;
        newValue = juce::jlimit(0.0f, 1.0f, newValue);

        if (std::fabs(newValue - value) > 1e-10f)
        {
            value = newValue;
            repaint();
            if (onValueChange)
                onValueChange(value);
        }
    }

    void setValue(float newValue)
    {
        newValue = juce::jlimit(0.0f, 1.0f, newValue);
        value = newValue;
        repaint();
    }

    float getValue() const { return value; }

    void setTextPosition(int y, int height) { textY = y; textHeight = height; }

    std::function<void(float)> onValueChange;

private:
    float axis;
    float defaultValue;
    float value = 0.0f;
    float dragStartValue = 0.0f;
    int dragStartY = 0;
    int textY = 66;
    int textHeight = 15;
    std::function<juce::String(float)> stringConverter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VerticalFillSlider)
};

class VerticalFillSliderAttachment : private juce::AudioProcessorValueTreeState::Listener
{
public:
    VerticalFillSliderAttachment(juce::AudioProcessorValueTreeState& apvts,
                                 const juce::String& parameterID,
                                 VerticalFillSlider& slider)
        : apvts(apvts), paramId(parameterID), slider(slider)
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

    ~VerticalFillSliderAttachment() override
    {
        apvts.removeParameterListener(paramId, this);
    }

private:
    void parameterChanged(const juce::String&, float newValue) override
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

    juce::AudioProcessorValueTreeState& apvts;
    juce::String paramId;
    VerticalFillSlider& slider;
    bool isUpdating = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VerticalFillSliderAttachment)
};
