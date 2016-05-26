// MultiStateButton.cpp: implementation of the CMultiStateButton class.
//
//////////////////////////////////////////////////////////////////////

#include "MultiStateButton.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMultiStateButton::CMultiStateButton(const CRect& size, IControlListener* listener, long tag,
    CBitmap* background, long nStates, long heightOfOneState)
    : CControl(size, listener, tag, background)
    , nStates(nStates)
    , heightOfOneState(heightOfOneState)
{
    state = 0;
}

//------------------------------------------------------------------------
CMultiStateButton::~CMultiStateButton()
{
}

//------------------------------------------------------------------------
void CMultiStateButton::draw(CDrawContext* pContext)
{
    long off;

    if (getBackground())
        off = (long)(nStates * heightOfOneState * value);
    else
        off = 0;

    if (getBackground()) {
        if (getTransparency())
            getBackground()->draw(pContext, size, CPoint(0, off), 0.f);
        else
            getBackground()->draw(pContext, size, CPoint(0, off));
    } else {
        if (value)
            pContext->setFillColor(kRedCColor);
        else
            pContext->setFillColor(kGreenCColor);

        pContext->drawRect(size);

        pContext->setFrameColor(kWhiteCColor);

        if (value)
            pContext->drawString("on", size);
        else
            pContext->drawString("off", size);
    }
    setDirty(false);
}

//------------------------------------------------------------------------
CMouseEventResult CMultiStateButton::onMouseDown(CPoint& where, const CButtonState& buttons)
{
    if (!getMouseEnabled())
        return CMouseEventResult::kMouseEventNotHandled;

    if (!(buttons.isLeftButton()))
        return CMouseEventResult::kMouseEventNotHandled;

    value = (float)(value + 1.0 / (double)(nStates));
    if (value >= 1.0)
        value = 0.0;

    if (listener)
        listener->valueChanged(this);

    return CMouseEventResult::kMouseEventHandled;
}
