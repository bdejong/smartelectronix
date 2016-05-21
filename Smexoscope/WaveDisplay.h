#pragma once

#include "audioeffectx.h"

#include "vstgui.h"

#include "Defines.h"
#include "SmartelectronixDisplay.hpp"

class CWaveDisplay : public CControl
{
public:
	CWaveDisplay (const CRect& size, CSmartelectronixDisplay* effect, CBitmap *back, CBitmap *heads, CBitmap *readout);
	virtual ~CWaveDisplay();

	virtual void draw (CDrawContext *pContext);
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);

	virtual void setDirty (const bool val = true);

	virtual CBaseObject* newCopy () const { return 0; }

protected:
	COffscreenContext *OSDC;

	CPoint where;
	bool down;

	CSmartelectronixDisplay* effect;

	CBitmap *back;
	CBitmap *heads;
	CBitmap *readout;

	unsigned char display;
};
