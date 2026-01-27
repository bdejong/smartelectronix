#if !defined(AFX_PREVIEW_H__90B1A060_753A_11D4_9312_9C385F79CE38__INCLUDED_)
#define AFX_PREVIEW_H__90B1A060_753A_11D4_9312_9C385F79CE38__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "vstgui.h"
#include "Shaper.h"

class CShaper;

class CPreview:public CControl
{
public:
	CPreview(CRect& size, CControlListener *listener, long tag, CBitmap *background, CShaper *pShaper);
	virtual ~CPreview();

	void mouse(CDrawContext *pContext, CPoint &where);
	void draw(CDrawContext* pDC);

private:
	void internredraw();

	void fillsine();

	COffscreenContext *OSDC;

	float *sinarray;
	float *tmparray;

	CShaper *pShaper;

	CBitmap *hBackground;
};

#endif
