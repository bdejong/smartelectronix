// Delay.cpp: implementation of the CDelay class.
// DSP code preserved from original OnePingOnly plugin

#include "Delay.h"
#include <cstddef>

CDelay::CDelay(int maxDelay)
    : maxDelaySize(maxDelay)
{
    buffer = new float[static_cast<size_t>(maxDelaySize)];
    killBuffer();
    setFeed(0.0f);
    delay = 0;
}

CDelay::~CDelay()
{
    delete[] buffer;
}

void CDelay::setDelay(int d)
{
    pos = index - d;
    while (pos > maxDelaySize)
        pos -= maxDelaySize;
    while (pos < 0)
        pos += maxDelaySize;

    delay = d;
}

float CDelay::getVal(float in)
{
    float output = in + feed * buffer[pos];

    buffer[index] = output;

    ++index;
    if (index == maxDelaySize)
        index = 0;

    ++pos;
    if (pos == maxDelaySize)
        pos = 0;

    return output;
}

void CDelay::setMaxDelay(int maxDelay)
{
    maxDelaySize = maxDelay;

    delete[] buffer;
    buffer = new float[static_cast<size_t>(maxDelaySize)];
    killBuffer();

    index = 0;

    setDelay(delay);
}

void CDelay::killBuffer()
{
    for (int i = 0; i < maxDelaySize; i++)
        buffer[i] = 0.0f;
}

void CDelay::setFeed(float f)
{
    feed = f;
}
