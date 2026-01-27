// MovingDelay.h: interface for the CMovingDelay class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CMovingDelay
{
public:
    CMovingDelay(long maxSize = 1000000)
    {
        buffer = new float[static_cast<unsigned long>(maxSize)];
        index = static_cast<unsigned long>(maxSize) - 1;
        max = static_cast<unsigned long>(maxSize);
        suspend();
    }

    virtual ~CMovingDelay()
    {
        delete[] buffer;
    }

    inline float GetVal(float in, float delay)
    {
        buffer[index] = in;

        float pointer = (float)index + delay;
        unsigned long outindex = (unsigned long) pointer;
        float alpha = pointer - outindex;
        
        if(outindex >= max)
            outindex -= max;

        unsigned long outindex1 = outindex + 1;
        if(outindex1 >= max)
            outindex1 -= max;
        
        index--;
        if(index >= max)
            index = max - 1;
    
        return (1.f - alpha)*buffer[outindex] + alpha*buffer[outindex1];
    }

    void suspend()
    {
        for(unsigned long i=0;i<max;i++)
            buffer[i] = 0;
    }

    unsigned long max;

private:
    float *buffer;
    unsigned long index;
};
