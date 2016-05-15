#pragma once

#ifdef MODULAR
	#define SIMPLE_VERSION 0
	#define PLUGIN_NAME "s(M)exoscopeModular"
#else
	#define SIMPLE_VERSION 1
	#define PLUGIN_NAME "s(M)exoscope"
#endif

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
