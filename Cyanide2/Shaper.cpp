#include "Shaper.h"

#include <math.h>

CShaper::CShaper()
{
	a = b = 0.f;
	n = 2;
	p[0].x = 0;
	p[0].y = 1;
	p[1].x = 1;
	p[1].y = 0;
	degree = 4;
	//degree = 2; = lowest
	pView = 0;
	pPreview = 0;
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

	if(pView)
		pView->setDirty();
}

void CShaper::updatedata()
{
	FillP2();

	if(n>=degree)
	{
		float WantedX = 0.0;
		float dx = 1.f/(float)table_size;
		float y;
		
		KnoopVector *u;
		u = new KnoopVector[n+degree];
		compute_intervals(u, n-1, degree);
		
		data[0] = 0.f;
		WantedX += dx;
		for(long i=1;i<table_size;i++)
		{
			y = GetBSplinePoint(WantedX,u);
			data[i] = y;
			WantedX += dx;
		}

		delete u;
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

	if(pPreview)
		pPreview->setDirty();
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
		P = RecursiveSpline(p2,0,n-1,midparam);
		
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
		compute_point(u, n-1, degree, midparam, p2, &P);
		
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

	if(pView)
		pView->setDirty();
}

void CShaper::SetPointerToPreview(CPreview *pPre)
{
	pPreview = pPre;
}

void CShaper::SetPointerToView(CShaperView *pView)
{
	this->pView = pView;
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

#ifdef MAC
	#define iman_	1
#else
	#define iman_	0
#endif

void CShaper::process(float *input, float *output, long nSamp)
{
	float ab = a-b;

	for(long i=0;i<nSamp;i++)
	{
		double val = fabs(input[i]) + (68719476736.0*1.5) / (1<<extra_bits);
		unsigned long A = ((unsigned long *)&val)[iman_];
		unsigned long index = (A >> frac_bits);
		
		if(index < table_size)
		{
			float frac = (A & (frac_size - 1))*(1.f/(float)frac_size);
			
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

//MUCH slower!!
/*void CShaper::process(float *input, float *output, long nSamp)
{
	float alpha, out;
	long index;

	for(long i=0;i<nSamp;i++)
	{
		float in = input[i] + offset;
		
		unsigned long sign = *((unsigned long*)&in) & ((unsigned long)1 << 31);

		in = fabsf(in);
		
		float tmp = in*nShape_1;
		
		index = fastcast(tmp);
		alpha = tmp - (float)index;

		if(index >= nShape)				//we could protect against overflow here...
			out = a*(in - 1.f) + b;
		else
			out = data[index] + alpha*(data[index+1] - data[index]);

		output[i] = sign ? -out : out;
	}
}
*/
