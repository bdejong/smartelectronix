#pragma once

#include "Spline.h"
#include "Defines.h"
#include <functional>
#include <array>
#include <vector>

class CShaper
{
public:
	void process(float *input, float *output, long nSamp);

	CShaper();
	~CShaper();

	void SetData(SplinePoint *data, long n);
	void GetData(SplinePoint *data);
	long GetNPoints();

	SplinePoint& getPoint(int index) { return p[index]; }
	void setPoint(int index, SplinePoint pt) { p[index] = pt; }
	long& getN() { return n; }

	void insertPoint(float x, float y);
	void removePoint(int index);

	void updatedata();

	void reset();

	std::function<void()> onDataChanged;

private:
	std::array<SplinePoint, maxn> p {};
	long n;

	void FillP2();
	float GetSplinePoint(float wantedX);
	float GetBSplinePoint(float wantedX, KnoopVector *u);
	float guess;

	std::array<float, table_size + 1> data {};
	float a, b;
	long degree;

	std::array<SplinePoint, maxn> p2 {};
};
