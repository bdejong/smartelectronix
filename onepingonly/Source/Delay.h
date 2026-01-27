#pragma once

class CDelay
{
public:
    CDelay(int maxDelay);
    ~CDelay();

    void setFeed(float feed);
    void killBuffer();
    void setMaxDelay(int maxDelay);
    float getVal(float in);
    void setDelay(int d);

private:
    float* buffer = nullptr;
    float feed = 0.0f;
    int index = 0;
    int pos = 0;
    int delay = 0;
    int maxDelaySize = 0;
};
