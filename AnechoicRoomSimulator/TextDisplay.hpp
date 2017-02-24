#pragma once

#include "vstgui.h"

class CTextDisplay : public CParamDisplay
{
public:
    CTextDisplay(CRect& size, CBitmap* ascii, CColor rectColor);
    virtual ~CTextDisplay();

    void setAlignMiddle() { middle = true; };

    virtual void draw(CDrawContext* pContext);
    void setText(char* text);

private:
    char todisplay_[255];
    CColor textColor_;
    CBitmap* ascii_;

    CColor rectColor_;
    bool middle;
};
