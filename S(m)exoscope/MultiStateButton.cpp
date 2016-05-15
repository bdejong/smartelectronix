// MultiStateButton.cpp: implementation of the CMultiStateButton class.
//
//////////////////////////////////////////////////////////////////////

#include "MultiStateButton.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMultiStateButton::CMultiStateButton (const CRect &size, CControlListener *listener, long tag,
                            CBitmap *background, long nStates, long heightOfOneState)
:	CControl (size, listener, tag, background), nStates(nStates), heightOfOneState(heightOfOneState)
{
	state = 0;
}

//------------------------------------------------------------------------
CMultiStateButton::~CMultiStateButton ()
{}

//------------------------------------------------------------------------
void CMultiStateButton::draw (CDrawContext *pContext)
{
	long off;

	if (pBackground)
		off = (long)(nStates * heightOfOneState * value);
	else
		off = 0;

	if (pBackground)
	{
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, size, CPoint (0, off));
		else
			pBackground->draw (pContext, size, CPoint (0, off));
	}
	else
	{
		if(value)
			pContext->setFillColor(kRedCColor);
		else
			pContext->setFillColor(kGreenCColor);
		
		pContext->fillRect(size);

		pContext->setFrameColor(kWhiteCColor);

		if(value)
			pContext->drawString("on",size);
		else
			pContext->drawString("off",size);
	}
	setDirty (false);
}

//------------------------------------------------------------------------
void CMultiStateButton::mouse (CDrawContext *pContext, CPoint &where)
{
	if (!bMouseEnabled)
		return;

 	long button = pContext->getMouseButtons ();
	if (!(button & kLButton))
		return;

	value = (float)(value + 1.0 / (double)(nStates));
	if(value >= 1.0) value = 0.0;

	//draw (pContext);
	doIdleStuff ();
	if (listener)
		listener->valueChanged (pContext, this);
}
