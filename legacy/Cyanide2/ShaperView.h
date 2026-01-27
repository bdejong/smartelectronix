// ShaperView.h: interface for the CShaperView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHAPERVIEW_H__19B98E40_F9EE_11D3_A03E_00AA00419C92__INCLUDED_)
#define AFX_SHAPERVIEW_H__19B98E40_F9EE_11D3_A03E_00AA00419C92__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "shaper.h"
#include "vstgui.h"
#include "spline.h"
#include "Preview.h"

class CShaper;

#define DISTANCE (0.002f)
#define N_LINES (200)

class CShaperView : public CControl
{
public:
	CShaperView(CRect& size, CControlListener *listener, long tag, CBitmap *background, CBitmap *handle, CShaper *pShaper);
	virtual ~CShaperView();
	
	void mouse(CDrawContext *pContext, CPoint &where);
	void draw(CDrawContext* pDC);
	
	float getValue();

	bool getSelectedXY(float &x, float &y);
	void reset();
	void setGuides(bool g);
	
private:
	void InternRedraw();

	void DrawSpline();
	void DrawBSpline();

	bool onMouseMove(SplinePoint point);
	bool OnLButtonUp(SplinePoint point);
	bool onLButtonDown(SplinePoint point);
	bool onRButtonDown(SplinePoint point);
	void ConvertToScreen(SplinePoint p, CPoint &P);
	void ConvertFromScreen(CPoint P, SplinePoint &p);
	void InsertPoint(float x, float y);

	CShaper *pShaper;
	
	CBitmap *hBackground;
	CBitmap *hHandle;

	SplinePoint lines[N_LINES];
	
	float minx, miny, maxx, maxy;
	
	long selectedIndex;

	COffscreenContext *OSDC;

	bool guides;
};

#endif // !defined(AFX_SHAPERVIEW_H__19B98E40_F9EE_11D3_A03E_00AA00419C92__INCLUDED_)

