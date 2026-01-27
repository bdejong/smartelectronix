#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class NumberBoxDisplay : public juce::Component
{
public:
    NumberBoxDisplay(const juce::Image& backgroundImage, float defaultValue,
                     std::function<juce::String(float)> stringConverter = nullptr,
                     juce::Colour textColour = juce::Colours::white,
                     float fontSize = 10.0f,
                     bool circularFill = false)
        : bgImage(backgroundImage), defaultValue(defaultValue),
          stringConverter(std::move(stringConverter)),
          textColour(textColour), fontSize(fontSize), circularFill(circularFill)
    {
        setOpaque(true);
    }

    void paint(juce::Graphics& g) override
    {
        auto w = getWidth();
        auto h = getHeight();
        auto pos = getBoundsInParent();

        if (circularFill)
        {
            // Draw the background behind the circle (so corners match the parent)
            if (!bgImage.isNull())
            {
                g.drawImage(bgImage,
                            0, 0, w, h,
                            pos.getX(), pos.getY(), w, h);
            }

            // Clip to circle
            juce::Path circle;
            circle.addEllipse(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h));
            g.saveState();
            g.reduceClipRegion(circle);

            // Fill circle with black
            g.setColour(juce::Colours::black);
            g.fillRect(getLocalBounds());

            // Draw red pie from 3 o'clock, going clockwise
            if (value > 0.0f)
            {
                float startAngle = 0.0f;
                float endAngle = value * juce::MathConstants<float>::twoPi;

                juce::Path pie;
                pie.addPieSegment(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h),
                                  startAngle, endAngle, 0.0f);

                g.setColour(juce::Colour(239, 0, 0));
                g.fillPath(pie);
            }

            g.restoreState();
        }
        else
        {
            // Simple background cutout (for rootKeyBox etc.)
            if (!bgImage.isNull())
            {
                g.drawImage(bgImage,
                            0, 0, w, h,
                            pos.getX(), pos.getY(), w, h);
            }
        }

        // Draw value text
        juce::String text;
        if (stringConverter)
            text = stringConverter(value);
        else
            text = juce::String(static_cast<int>(value * 100.0f)) + "%";

        g.setColour(textColour);
        g.setFont(juce::Font(fontSize));
        g.drawText(text, getLocalBounds(), juce::Justification::centred, false);
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

        if (std::fabsf(newValue - value) > 1e-10f)
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

    std::function<void(float)> onValueChange;

private:
    juce::Image bgImage;
    float defaultValue;
    float value = 0.0f;
    float dragStartValue = 0.0f;
    int dragStartY = 0;
    std::function<juce::String(float)> stringConverter;
    juce::Colour textColour;
    float fontSize;
    bool circularFill;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NumberBoxDisplay)
};

class NumberBoxDisplayAttachment : private juce::AudioProcessorValueTreeState::Listener
{
public:
    NumberBoxDisplayAttachment(juce::AudioProcessorValueTreeState& apvts,
                               const juce::String& parameterID,
                               NumberBoxDisplay& display)
        : apvts(apvts), paramId(parameterID), display(display)
    {
        if (auto* param = apvts.getParameter(parameterID))
            display.setValue(param->getValue());

        apvts.addParameterListener(parameterID, this);

        display.onValueChange = [this, parameterID](float newValue)
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

    ~NumberBoxDisplayAttachment() override
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
                display.setValue(newValue);
                isUpdating = false;
            });
        }
    }

    juce::AudioProcessorValueTreeState& apvts;
    juce::String paramId;
    NumberBoxDisplay& display;
    bool isUpdating = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NumberBoxDisplayAttachment)
};
