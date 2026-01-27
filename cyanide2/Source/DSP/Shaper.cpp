#include "Shaper.h"

#include <cmath>
#include <vector>

CShaper::CShaper()
{
	a = b = 0.f;
	n = 2;
	p[0].x = 0;
	p[0].y = 1;
	p[1].x = 1;
	p[1].y = 0;
	degree = 4;
	updatedata();
}

CShaper::~CShaper()
{
}

long CShaper::GetNPoints()
{
	return n;
}

void CShaper::GetData(SplinePoint *data)
{
	for(long i=0;i<n;i++)
	{
		data[i].x = p[i].x;
		data[i].y = p[i].y;
	}
}

void CShaper::SetData(SplinePoint *data, long n)
{
	if(n>1)
	{
		this->n = n;
		for(long i=0;i<n;i++)
		{
			p[i].x = data[i].x;
			p[i].y = data[i].y;
		}
	}
	else
	{
		this->n = 2;
		p[0].x = 0;
		p[0].y = 1;
		p[1].x = 1;
		p[1].y = 0;
	}

	if (onDataChanged) onDataChanged();
}

void CShaper::insertPoint(float x, float y)
{
	if (n >= maxn) return;

	long i;
	for(i=1;i<n;i++)
		if(x < p[i].x)
			break;

	for(long j=n;j>i;j--)
		p[j] = p[j-1];

	p[i].x = x;
	p[i].y = y;

	n++;
}

void CShaper::removePoint(int index)
{
	if (index <= 0 || index >= n-1 || n <= 2) return;

	for(long i=index;i<n;i++)
		p[i]=p[i+1];

	n--;
}

void CShaper::updatedata()
{
	FillP2();

	if(n>=degree)
	{
		float WantedX = 0.0;
		float dx = 1.f/(float)table_size;
		float y;

		std::vector<KnoopVector> u(static_cast<size_t>(n + degree));
		compute_intervals(u.data(), n-1, degree);

		data[0] = 0.f;
		WantedX += dx;
		for(long i=1;i<table_size;i++)
		{
			y = GetBSplinePoint(WantedX,u.data());
			data[i] = y;
			WantedX += dx;
		}

	}
	else
	{
		float WantedX = 0.f;
		float dx = 1.f/(float)table_size;
		float y;

		data[0] = 0.f;
		WantedX += dx;

		for(long i=1;i<table_size;i++)
		{
			y = GetSplinePoint(WantedX);
			data[i] = y;
			WantedX += dx;
		}
	}

	data[table_size] = 2*data[table_size - 1] - data[table_size - 2];

	//a and b

	float x1 = p2[n-2].x;
	float y1 = p2[n-2].y;
	float x2 = p2[n-1].x;
	float y2 = p2[n-1].y;

	float temp = x1 - x2;

	if(temp == 0.f)
	{
		if(y2 > y1)
			a = 1.f;
		else
			a = -1.f;
	}
	else
		a = (y1-y2)/temp;

	if(a > 2.f)
		a = 2.f;
	else
		if(a < -2.f)
			a = -2.f;

	b = y2;

	if (onDataChanged) onDataChanged();
}

float CShaper::GetSplinePoint(float wantedX)
{
	SplinePoint P;

	float minparam = 0;
	float maxparam = 1;
	float midparam = (minparam + maxparam)*0.5f;

	float error = 10.f;
	long nIter = 0;
	long maxiter = 300;

	while((error > MAXERROR) && (nIter<maxiter))
	{
		P = RecursiveSpline(p2.data(),0,n-1,midparam);

		if(P.x <= wantedX)
		{
			minparam = midparam;
			error = wantedX - P.x;
		}
		if(P.x >= wantedX)
		{
			maxparam = midparam;
			error = P.x - wantedX;
		}

		midparam = (minparam + maxparam)*0.5f;

		nIter++;
	}

	return P.y;
}

float CShaper::GetBSplinePoint(float wantedX, KnoopVector *u)
{
	SplinePoint P;

	float minparam = 0;
	float maxparam = (float) (n-degree+1);
	float midparam = (minparam + maxparam)*0.5f;

	float error = 10.f;
	long nIter = 0;
	long maxiter = 300;

	while((error > MAXERROR) && (nIter<maxiter))
	{
		compute_point(u, n-1, degree, midparam, p2.data(), &P);

		if(P.x <= wantedX)
		{
			minparam = midparam;
			error = wantedX - P.x;
		}
		if(P.x >= wantedX)
		{
			maxparam = midparam;
			error = P.x - wantedX;
		}

		midparam = (minparam + maxparam)*0.5f;

		nIter++;
	}

	return P.y;

}

void CShaper::reset()
{
	this->n = 2;
	p[0].x = 0;
	p[0].y = 1;
	p[1].x = 1;
	p[1].y = 0;

	if (onDataChanged) onDataChanged();
}

void CShaper::FillP2()
{
	float x,y;

	for(long i=0;i<n;i++)
	{
		x = p[i].x;
		y = 1.f - p[i].y;

		p2[i].x = x*x;
		p2[i].y = y*y;
	}
}

void CShaper::process(float *input, float *output, long nSamp)
{
	float ab = a-b;

	for(long i=0;i<nSamp;i++)
	{
		float absIn = fabsf(input[i]);
		float scaled = absIn * (float)table_size;
		unsigned int index = (unsigned int)scaled;
		float frac = scaled - (float)index;

		if(index < (unsigned int)table_size)
		{
			if(input[i] >= 0.f)
				output[i] = data[index] + frac*(data[index+1] - data[index]);
			else
				output[i] = -(data[index] + frac*(data[index+1] - data[index]));
		}
		else
		{
			if(input[i] >= 0.f)
				output[i] = a*input[i] - ab;
			else
				output[i] = a*input[i] + ab;
		}
	}
}
