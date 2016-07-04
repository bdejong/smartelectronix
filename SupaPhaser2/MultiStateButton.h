// MultiStateButton.h: interface for the CMultiStateButton class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "vstgui.h"

class CMultiStateButton : public CControl
{
public:
	CMultiStateButton (const CRect &size, IControlListener *listener, long tag,
                  CBitmap *background, long nStates, long heightOfOneState);
	virtual ~CMultiStateButton();

	virtual void draw(CDrawContext*) override;
    virtual CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons);

    virtual CBaseObject* newCopy() const { return 0; }

private:
	long nStates, state, heightOfOneState;
};
