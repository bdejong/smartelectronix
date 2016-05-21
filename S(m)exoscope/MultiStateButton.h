// MultiStateButton.h: interface for the CMultiStateButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MULTISTATEBUTTON_H__9F0F90CB_F0C9_4896_AF7A_AC87E2EE29A1__INCLUDED_)
#define AFX_MULTISTATEBUTTON_H__9F0F90CB_F0C9_4896_AF7A_AC87E2EE29A1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "vstgui.h"

class CMultiStateButton : public CControl
{
public:
	CMultiStateButton (const CRect &size, IControlListener *listener, long tag,
                  CBitmap *background, long nStates, long heightOfOneState);
	virtual ~CMultiStateButton ();

	virtual void draw (CDrawContext*);
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);

	virtual CBaseObject* newCopy () const { return 0; }

private:
	long nStates, state, heightOfOneState;
};

#endif // !defined(AFX_MULTISTATEBUTTON_H__9F0F90CB_F0C9_4896_AF7A_AC87E2EE29A1__INCLUDED_)
