#pragma once

#include <cmath>

//////////////////////////////////////////////////////////////////////////////////////
// some constants...

const int maxn = 50; //max n of points in graph

static constexpr int extra_bits = 8;
static constexpr int table_bits_ = 10;
static constexpr int table_size = (1 << table_bits_);
static constexpr int frac_bits_ = (16 - table_bits_ + extra_bits);
static constexpr int frac_size = (1 << frac_bits_);
static constexpr float MAXERROR = 0.000001f;
static constexpr int FRAME_SIZE = 64;

//////////////////////////////////////////////////////////////////////////////////////
// some handy functions

inline bool IS_DENORMAL(float x) { return fabsf(x) < 0.000001f; }

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
}
