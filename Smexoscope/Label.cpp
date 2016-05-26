#include "Label.h"

CLabel::CLabel(const CRect& R, const std::string& text)
    : CParamDisplay(R), _label(text)
{
    CColor grey = { 118, 118, 118 };

    setFont(kNormalFontSmall);
    setFontColor(kWhiteCColor);
    setBackColor(grey);
}

void CLabel::draw(CDrawContext* pContext)
{
    pContext->setFrameColor(backColor);
    pContext->setFillColor(backColor);
    pContext->drawRect(size);

    pContext->setFrameColor(fontColor);
    pContext->setFont(fontID);
    pContext->setFontColor(fontColor);
    pContext->drawString(_label.c_str(), size, kCenterText, true);

    setDirty(false);
}

void CLabel::setLabel(const std::string& text)
{
    if (_label != text)
    {
        _label = text;
        setDirty(true);
    }
}
