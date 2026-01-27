#pragma once

class CCompressor {
public:
    CCompressor(float samplerate);
    virtual ~CCompressor();

    //set values, in [0..1], Cubase style...
    void setSaturate(float s);
    void setPostamp(float p);
    void setAmount(float a);
    void setPreamp(float p);
    void setRelease(float r);
    void setAttack(float a);

    void setSamplerate(float sr = 44100.f);

    //these return SCALED values
    float getPostamp() const { return postamp; }
    float getAmount() const { return a * 50.f; }
    float getPreamp() const { return preamp; }
    float getRelease() const { return release; }
    float getAttack() const { return attack; }
    bool getSaturate() const { return saturate; }

    void Suspend();

    void process(float** input, float** output, long nSamples);
    void processReplacing(float** input, float** output, long nSamples);

private:
    void CalcLUT();

    float ga;
    float gr;
    float e, eLP;
    float a;

    float *LUT, *dLUT;

    float preamp;
    float amount;
    float attack;
    float release;
    float postamp;

    float samplerate;

    int nLUT;

    bool saturate;

    float msmooth;
};
