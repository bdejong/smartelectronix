#pragma once

#include "audioeffectx.h"

#include "vstgui.h"

class CMultiStateButton : public CControl {
public:
    CMultiStateButton(const CRect& size, IControlListener* listener, long tag,
        CBitmap* background, long nStates, long heightOfOneState);
    virtual ~CMultiStateButton();

    virtual void draw(CDrawContext*);
    virtual CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons);

    virtual CBaseObject* newCopy() const { return 0; }

private:
    long nStates, state, heightOfOneState;
};
