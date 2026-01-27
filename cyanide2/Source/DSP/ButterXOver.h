#pragma once

#include <array>

static constexpr int nPre = 1024;

struct XOverCoeffs
{
	float aA1;
	float aA2;
	float bA2;
};

class CButterXOver
{
public:
	float getFreq(long index);
	void processchange(float *input, float *output, long filter, long nSamp);
	void process(float *input, float *output, long nSamp);

	void precompute();
	void denormalise();
	void suspend();
	void setrate(float sr);
	void settype(float t);
	CButterXOver();
	~CButterXOver();
private:
	float outA1,in_1,out_2_A2,out_1_A2,m1,m2,in_2;

	float samplerate;
	long freq;

	static std::array<XOverCoeffs, nPre> Pre;
	static std::array<float, nPre> PreFreq;
};
