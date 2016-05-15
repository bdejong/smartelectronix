#pragma once

#include <math.h>

//////////////////////////////////////////////////////////////////////////////////////
// da kookie

const char thisVersionKookie[] = "Cyanide 2.0";

//////////////////////////////////////////////////////////////////////////////////////
// some constants...

const int maxn = 50; //max n of points in graph

#define extra_bits (8)
#define table_bits (10)
#define table_size (1<<table_bits)
#define table_size_1 ((float)(tablesize-1))
#define frac_bits (16 - table_bits + extra_bits)
#define frac_size (1<<frac_bits)
#define MAXERROR 0.000001f
#define FRAME_SIZE 64

//////////////////////////////////////////////////////////////////////////////////////
// some handy functions

#define IS_DENORMAL(x) (fabs(x) < 0.000001f)

inline float dB2lin(float f) { return powf(10,(f/20.f)); }

inline float gainMapScaled(float mm) 
{ 
	float db; 
	mm = 100.f - mm*100.f; 
	if (mm <= 0.f) { 
		db = 10.f; 
	} else if (mm < 48.f) { 
		db = 10.f - 5.f/12.f * mm; 
	} else if (mm < 84.f) { 
		db = -10.f - 10.f/12.f * (mm - 48.f); 
	} else if (mm < 96.f) { 
		db = -40.f - 20.f/12.f * (mm - 84.f); 
	} else if (mm < 100.f) { 
		db = -60.f - 35.f * (mm - 96.f); 
	} else db = -200.f; 

	return dB2lin(db); 
};

//////////////////////////////////////////////////////////////////////////////////////
// byte-swapping
// this only works with 4-byte variables!! (long, float, ..)

template <class T> inline T reverse(T x)
{
#if MAC
	unsigned long d = *((unsigned long *)&x);
	d = ((d & 0x000000ff) << 24) | ((d & 0x0000ff00) << 8) | ((d & 0x00ff0000) >> 8) | ((d & 0xff000000) >> 24);
	return (*((T *)&d));
#else
	return x;
#endif
};
