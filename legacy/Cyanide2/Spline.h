#pragma once

struct SplinePoint
{
	float x;
	float y;
};

struct KnoopVector
{
	long I;
	float i;
};

//Bezier

SplinePoint RecursiveSpline(SplinePoint *data, long i, long j, float t);

//B-Splines

void getpoint(SplinePoint &P, SplinePoint *Control, long n, float u);
void bspline(long n, long t, SplinePoint *control, SplinePoint *output, long num_output);
void compute_intervals(KnoopVector *u, long n, long t);
void compute_point(KnoopVector *u, long n, long t, float v, SplinePoint *control, SplinePoint *output);
