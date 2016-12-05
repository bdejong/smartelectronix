// CustomSplashScreen.h: interface for the CCustomSplashScreen class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "vstgui.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CCustomSplashScreen : public CControl
{
public:
	CCustomSplashScreen (const CRect &size, IControlListener *listener, const long tag, CBitmap *background, const CRect &toDisplay, const CPoint &offset);
	virtual ~CCustomSplashScreen ();	
  
	virtual void draw(CDrawContext*) override;
    virtual CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
	void unSplash();
    virtual CBaseObject* newCopy() const override { return 0; }


protected:
	CRect    toDisplay;
	CRect    keepSize;
	CPoint   offset;

	float offValue;
	float onValue;
};
