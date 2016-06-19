// CustomSplashScreen.h: interface for the CCustomSplashScreen class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "vstgui.h"

long launch_url(const char *urlstring);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CCustomSplashScreen : public CControl
{
public:
	CCustomSplashScreen (const CRect &size, IControlListener *listener, long tag, CBitmap *background, CRect &toDisplay, CPoint &offset);
	virtual ~CCustomSplashScreen ();	
  
	virtual void draw(CDrawContext*) override;
    virtual CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons);
	void unSplash();

protected:
	CRect    toDisplay;
	CRect    keepSize;
	CPoint   offset;

	float offValue;
	float onValue;
};
