#pragma once

#define MAX_FLOAT 150000.f

#define BACK_WIDTH 825
#define BACK_HEIGHT 300

#define OSC_WIDTH 627
#define OSC_HEIGHT 269

inline float clip(float x, float max = 1.f)
{
	if(x > max)
		return max;
	if(x < -max)
		return -max;
	return x;
}
