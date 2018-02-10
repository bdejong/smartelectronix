#include "TextDisplay.hpp"
#include "asciitable.hpp"

CTextDisplay::CTextDisplay(const CRect& size, CBitmap* ascii, CColor rectColor)
    : CParamDisplay(size)
{
    ascii_ = ascii;
    ascii_->remember();
    rectColor_ = rectColor;
    fillasciitable();
    middle = false;
}

CTextDisplay::~CTextDisplay()
{
    ascii_->forget();
}

void CTextDisplay::draw(CDrawContext* pContext)
{
    //this should be done off-screen, but I didn't feel like it.
    //I'll fix it in the next version (+ Mat's aplha-strip...)

    CRect tmprect = getViewSize();
    tmprect.offset(3, -1);

#ifdef _DEBUG
    pContext->setFillColor(kRedCColor);
#else
    pContext->setFillColor(rectColor_);
#endif

    pContext->setFrameColor(rectColor_);

    pContext->drawRect(getViewSize());
    //pContext->fillRect(size);

    CRect sourcerect;
    CPoint bitmapoffset;

    int left = getViewSize().left; //our staring point!
    int top = getViewSize().top; //our staring point!
    int bottom = getViewSize().top + ascii_->getHeight(); //our staring point!
    int place;
    int width;

    long totalWidth = 0;

    long spaceSize = 3;

    for (int i = 0; i < 256; ++i)
    {
        if (todisplay_[i] == 0)
            break;

        PlaceAndWidth(todisplay_[i], place, width);

        if (todisplay_[i] == ' ')
        {
            totalWidth += spaceSize;
        }

        if (place != -1)
        {
            totalWidth += width;
        }
    }

    if (middle)
        left += (getViewSize().getWidth() - totalWidth) / 2;
    else
        left += getViewSize().getWidth() - totalWidth;

    for (int i = 0; i < 256; ++i)
    {
        if (todisplay_[i] == 0)
            break;

        PlaceAndWidth(todisplay_[i], place, width);

        if (todisplay_[i] == ' ')
        {
            left += spaceSize;
        }


        if (place != -1)
        {
            sourcerect(left, top, left + width, bottom);

            //draw
            bitmapoffset(place, 0);
            ascii_->draw(pContext, sourcerect, bitmapoffset);

            left += width;
        }
    }

    setDirty(false);
}

void CTextDisplay::setText(char* text)
{
    strcpy(todisplay_, text);
    setDirty(true);
}
