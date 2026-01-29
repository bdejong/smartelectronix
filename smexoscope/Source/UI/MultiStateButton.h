#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class MultiStateButton : public juce::Component
{
public:
    MultiStateButton(const juce::Image& strip, int states, int explicitFrameHeight = 0)
        : filmStrip(strip), numStates(states)
    {
        frameHeight = (explicitFrameHeight > 0) ? explicitFrameHeight
                                                 : filmStrip.getHeight() / numStates;
        setRepaintsOnMouseActivity(true);
    }

    void paint(juce::Graphics& g) override
    {
        if (filmStrip.isNull())
            return;

        int frameIndex = juce::jlimit(0, numStates - 1, currentState);
        g.drawImage(filmStrip,
                    0, 0, getWidth(), getHeight(),
                    0, frameIndex * frameHeight, filmStrip.getWidth(), frameHeight);
    }

    void mouseDown(const juce::MouseEvent&) override
    {
        currentState = (currentState + 1) % numStates;
        repaint();
        if (onValueChange)
            onValueChange(getValueForState());
    }

    void setValue(float v)
    {
        int newState = juce::jlimit(0, numStates - 1,
            static_cast<int>(v * static_cast<float>(numStates) + 0.0001f));
        if (newState != currentState)
        {
            currentState = newState;
            repaint();
        }
    }

    float getValue() const { return getValueForState(); }
    void setImage(const juce::Image& newImage, int explicitFrameHeight = 0)
    {
        filmStrip = newImage;
        frameHeight = (explicitFrameHeight > 0) ? explicitFrameHeight
                                                 : filmStrip.getHeight() / numStates;
        repaint();
    }

    std::function<void(float)> onValueChange;

private:
    float getValueForState() const
    {
        if (numStates <= 1) return 0.0f;
        // Match original VST2 convention: value = state / numStates
        // This ensures value never reaches 1.0, so triggerType * kNumTriggerTypes
        // maps correctly for all states including the last one.
        return static_cast<float>(currentState) / static_cast<float>(numStates);
    }

    juce::Image filmStrip;
    int numStates;
    int frameHeight;
    int currentState = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiStateButton)
};

class MultiStateButtonAttachment : private juce::AudioProcessorValueTreeState::Listener
{
public:
    MultiStateButtonAttachment(juce::AudioProcessorValueTreeState& state,
                               const juce::String& parameterID,
                               MultiStateButton& btn)
        : apvts(state), paramId(parameterID), button(btn)
    {
        if (auto* param = apvts.getParameter(parameterID))
            button.setValue(param->getValue());

        apvts.addParameterListener(parameterID, this);

        button.onValueChange = [this, parameterID](float newValue)
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

    ~MultiStateButtonAttachment() override
    {
        apvts.removeParameterListener(paramId, this);
    }

private:
    void parameterChanged(const juce::String& /*parameterID*/, float newValue) override
    {
        if (!isUpdating)
        {
            isUpdating = true;
            juce::MessageManager::callAsync([this, newValue]()
            {
                button.setValue(newValue);
                isUpdating = false;
            });
        }
    }

    juce::AudioProcessorValueTreeState& apvts;
    juce::String paramId;
    MultiStateButton& button;
    bool isUpdating = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiStateButtonAttachment)
};
