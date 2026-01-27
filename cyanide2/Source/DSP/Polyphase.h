#pragma once

#include <array>

class CPolyphase
{
public:
	void Downsample(float *input, float *output, long nSamples);
	void Upsample(float *input, float *output, long nSamples);
	void suspend();
	void denormalise();
	CPolyphase();
	~CPolyphase();
private:
	std::array<float, 3> uout {};
	std::array<float, 3> uin_1 {};

	std::array<float, 3> lout {};
	std::array<float, 3> lin_1 {};
};
