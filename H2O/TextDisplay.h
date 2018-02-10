#pragma once

#include "vstgui.h"

class CTextDisplay : public CControl {
public:
    CTextDisplay(const CRect& size, IControlListener* listener, int tag);
    virtual ~CTextDisplay();
    virtual void draw(CDrawContext* pContext) override;
    void setText(const char* text);

    CBaseObject* newCopy () const override
    {
        return new CTextDisplay(getViewSize(), listener, tag);
    }

private:
    char todisplay[255];
    CColor textColor;
    CBitmap* ascii;
};
