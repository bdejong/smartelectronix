#pragma once

#include "audioeffectx.h"

#include "vstgui.h"

#include "Defines.h"
#include "SmartelectronixDisplay.hpp"

class CWaveDisplay : public CControl {
public:
    CWaveDisplay(const CRect& size, CSmartelectronixDisplay* effect, CBitmap* back, CBitmap* heads, CBitmap* readout);

    virtual ~CWaveDisplay();

    virtual void draw(CDrawContext* pContext) override;

    virtual CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
    virtual CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons) override;
    virtual CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;

    virtual CBaseObject* newCopy () const override
    {
        return new CWaveDisplay(size, effect, back, heads, readout);
    };

    CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

protected:
    CPoint where;

    CSmartelectronixDisplay* effect;

    CBitmap* back;
    CBitmap* heads;
    CBitmap* readout;
    CVSTGUITimer* timer;

    unsigned char display;
};
