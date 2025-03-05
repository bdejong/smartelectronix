#include "Delay.h"

Delay::Delay(int maxDelay)
    : feedback(0.0f)
    , feedbackTemp(1.0f)
    , bufferIndex(0)
    , readPosition(0)
    , delaySamples(0)
    , maxDelaySamples(maxDelay)
{
    buffer = std::make_unique<float[]>(static_cast<size_t>(maxDelaySamples));
    clearBuffer();
}

Delay::~Delay()
{
}

void Delay::setDelay(int delayInSamples)
{
    delaySamples = delayInSamples;
    
    readPosition = bufferIndex - delayInSamples;
    
    // Wrap readPosition to valid buffer range
    while (readPosition >= maxDelaySamples)
        readPosition -= maxDelaySamples;
    
    while (readPosition < 0)
        readPosition += maxDelaySamples;
}

float Delay::getVal(float input)
{
    // Read from buffer
    float delayedSample = buffer[static_cast<size_t>(readPosition)];
    
    // Calculate output with feedback - matching original implementation exactly
    float output = input + feedback * delayedSample;
    
    // Write to buffer
    buffer[static_cast<size_t>(bufferIndex)] = output;
    
    // Increment and wrap indices
    bufferIndex++;
    if (bufferIndex >= maxDelaySamples)
        bufferIndex = 0;
    
    readPosition++;
    if (readPosition >= maxDelaySamples)
        readPosition = 0;
    
    return output;
}

void Delay::setMaxDelay(int maxDelay)
{
    maxDelaySamples = maxDelay;
    
    buffer = std::make_unique<float[]>(static_cast<size_t>(maxDelaySamples));
    clearBuffer();
    
    // Reset indices
    bufferIndex = 0;
    
    // Recalculate read position based on new buffer size
    setDelay(delaySamples);
}

void Delay::clearBuffer()
{
    for (size_t i = 0; i < static_cast<size_t>(maxDelaySamples); i++)
        buffer[i] = 0.0f;
}

void Delay::setFeedback(float feedbackAmount)
{
    feedback = feedbackAmount;
    feedbackTemp = 1.0f - feedback; // Match the original implementation
}