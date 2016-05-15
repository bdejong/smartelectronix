#if !defined(AFX_SHAPER_H__E8BC8F40_056E_11D4_B566_00AA00419C92__INCLUDED_)
#define AFX_SHAPER_H__E8BC8F40_056E_11D4_B566_00AA00419C92__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Spline.h"
#include "ShaperView.h"
#include "Preview.h"
#include "Defines.h"

class CShaperView;
class CPreview;

class CShaper  
{
	friend class CShaperView;

public:
	void process(float *input, float *output, long nSamp);
	
	CShaper();
	virtual ~CShaper();

	void SetData(SplinePoint *data, long n);
	void GetData(SplinePoint *data);
	long GetNPoints();

	void updatedata();
	
	void SetPointerToView(CShaperView *pView);
	void SetPointerToPreview(CPreview *pPre);
	
	void reset();

private:

	SplinePoint p[maxn];
	long n;
	
	void FillP2();
	float GetSplinePoint(float wantedX);
	float GetBSplinePoint(float wantedX, KnoopVector *u);
	float guess;
	
	CShaperView *pView;
	CPreview *pPreview;

	float data[table_size + 1];
	float a, b;
	long degree;
	
	SplinePoint p2[maxn];
};

#endif
