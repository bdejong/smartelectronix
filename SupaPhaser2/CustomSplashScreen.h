// CustomSplashScreen.h: interface for the CCustomSplashScreen class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CUSTOMSPLASHSCREEN_H__B9850EA1_935B_4D55_B77E_5E7E51E0A46B__INCLUDED_)
#define AFX_CUSTOMSPLASHSCREEN_H__B9850EA1_935B_4D55_B77E_5E7E51E0A46B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "vstgui.h"


long launch_url(const char *urlstring);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CCustomSplashScreen : public CControl
{
public:
	CCustomSplashScreen (const CRect &size, CControlListener *listener, long tag, CBitmap *background, CRect &toDisplay, CPoint &offset);
	virtual ~CCustomSplashScreen ();	
  
	virtual void draw (CDrawContext*);
	virtual void mouse (CDrawContext *pContext, CPoint &where);
	virtual void unSplash ();

protected:
	CRect    toDisplay;
	CRect    keepSize;
	CPoint   offset;

	float offValue;
	float onValue;
};

#endif // !defined(AFX_CUSTOMSPLASHSCREEN_H__B9850EA1_935B_4D55_B77E_5E7E51E0A46B__INCLUDED_)
