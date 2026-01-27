#include "Spline.h"
#include <vector>

SplinePoint RecursiveSpline(SplinePoint *data, long i, long j, float t)
{
	if(j==1)
	{
		SplinePoint a;

		a.x = data[i].x + t*(data[i+1].x - data[i].x);
		a.y = data[i].y + t*(data[i+1].y - data[i].y);

		return a;
	}
	else
	{
		SplinePoint a = RecursiveSpline(data,i,j-1,t);
		SplinePoint b = RecursiveSpline(data,i+1,j-1,t);

		a.x += t*(b.x-a.x);
		a.y += t*(b.y-a.y);

		return a;
	}
}

//////////////////////////////////////////////////////////////////////
// B-Splines
//////////////////////////////////////////////////////////////////////

void bspline(long n, long t, SplinePoint *control, SplinePoint *output, long num_output)
{
	SplinePoint calcxyz;
	long output_index;

	std::vector<KnoopVector> u(static_cast<size_t>(n + t + 1));
	compute_intervals(u.data(), n, t);

	float increment=(float) (n-t+2)/(num_output-1);
	float interval=0;

	for (output_index=0;output_index < num_output-1;output_index++)
	{
		compute_point(u.data(), n, t, interval, control, &calcxyz);

		output[output_index].x = calcxyz.x;
		output[output_index].y = calcxyz.y;

		interval += increment;
	}

	output[num_output-1].x=control[n].x;
	output[num_output-1].y=control[n].y;

}

float blend4(long i, float u)
{
	float tmp = u - (float)i;

	if(u < i || u > i+4)
		return 0.f;

	if(u <= i+1)
	{
		return tmp*tmp*tmp*0.1666667f;
	}
	if(u <= i+2)
	{
		return 0.66667f + ((2.f - 0.5f * tmp) * tmp - 2.f) * tmp;
	}
	if(u <= i+3)
	{
		return -7.3333f + (10.f + (0.5f*tmp - 4.f) * tmp) * tmp;
	}
	if(u <= i+4)
	{
		return 10.66667f + ((2.f - 0.166667f * tmp) * tmp - 8.f)*tmp;
	}
	else
		return 0.f;
}

void getpoint(SplinePoint &P, SplinePoint *Control, long n, float u)
{
	float tmp;

	P.x = 0.f;
	P.y = 0.f;

	for(long i=0;i<n;i++)
	{
		tmp = blend4(i,u);
		P.x += Control[i].x*tmp;
		P.y += Control[i].y*tmp;
	}
}

float blend1(long k, long t, KnoopVector *u, float v)
{
	float value;

	if (t==1)
	{
		if ((u[k].i<=v) && (v<u[k+1].i))
			value = 1;
		else
			value = 0;
	}
	else
	{
		if ((u[k+t-1].I==u[k].I) && (u[k+t].I==u[k+1].I))
		{
			value = 0;
		}
		else
		{
			if(u[k+t-1].I==u[k].I)
			{
				value = (u[k+t].i - v) / (u[k+t].i - u[k+1].i)*blend1(k+1, t-1, u, v);
			}
			else
			{
				if(u[k+t].I==u[k+1].I)
					value = (v - u[k].i) / (u[k+t-1].i - u[k].i)*blend1(k, t-1, u, v);
				else
					value = (v - u[k].i) / (u[k+t-1].i - u[k].i)*blend1(k, t-1, u, v) +
							(u[k+t].i - v) / (u[k+t].i - u[k+1].i)*blend1(k+1, t-1, u, v);
			}
		}
	}
	return value;
}

void compute_intervals(KnoopVector *u, long n, long t)
{
	for (long j=0; j<=n+t; j++)
	{
		if (j<t)
		{
			u[j].I=0;
			u[j].i=(float)u[j].I;
		}
		else
			if ((t<=j) && (j<=n))
			{
				u[j].I=j-t+1;
				u[j].i=(float)u[j].I;
			}
			else
			{
				if (j>n)
				{
					u[j].I=n-t+2;
					u[j].i=(float)u[j].I;
				}
			}
	}
}

void compute_point(KnoopVector *u, long n, long t, float v, SplinePoint *control, SplinePoint *output)
{
	float temp;

	output->x=0;
	output->y=0;

	long index = (long) v;

	for(long k=index; k<index+t; k++)
	{
		temp = blend1(k,t,u,v);

		output->x += (control[k]).x * temp;
		output->y += (control[k]).y * temp;
	}
}
