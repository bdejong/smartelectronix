#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class Delay
{
public:
    Delay(int maxDelay = 44100 * 5);
    ~Delay();
    
    void setFeedback(float feedback);
    void clearBuffer();
    void setMaxDelay(int maxDelay);
    float getVal(float input); // Renamed to match original implementation
    void setDelay(int delayInSamples);
    
private:
    std::unique_ptr<float[]> buffer;
    float feedback;
    float feedbackTemp; // Added to match original implementation
    int bufferIndex;
    int readPosition;
    int delaySamples;
    int maxDelaySamples;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Delay)
};