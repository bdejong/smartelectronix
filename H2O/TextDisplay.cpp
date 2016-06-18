#include "TextDisplay.h"
#include "asciitable.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTextDisplay::CTextDisplay(const CRect& size, IControlListener* listener, int tag)
    : CControl(size, listener, tag)
{
    ascii = 0;
    ascii = new CBitmap("images/ascii.png");

    fillasciitable();
}

CTextDisplay::~CTextDisplay()
{
}

void CTextDisplay::draw(CDrawContext* pContext)
{
    //this should be done off-screen, but I didn't feel like it.
    //I'll fix it in the next version (+ Mat's aplha-strip...)

    CColor rectColor = { 0, 0, 0, 0 };
    CColor fontColor = { 255, 255, 255, 0 };
    CColor frameColor = { 0, 0, 0, 0 };

    pContext->setFontColor(fontColor);
    pContext->setFont(kSystemFont, 10);

    CRect tmprect = size;
    tmprect.offset(3, -1);
    pContext->setFillColor(rectColor);
    pContext->setFrameColor(frameColor);

    pContext->drawRect(size);
    // pContext->fillRect(size);
    // pContext->drawString(todisplay,tmprect,false,kLeftText);

    CRect sourcerect;
    CPoint bitmapoffset;

    int left = size.left + 3; //our staring point!
    int top = size.top; //our staring point!
    int bottom = size.top + ascii->getHeight(); //our staring point!
    int place;
    int width;

    for (int i = 0; i < 256; i++) {
        if (todisplay[i] == 0)
            break;

        PlaceAndWidth(todisplay[i], place, width);
        if (place != -1) {
            sourcerect(left, top, left + width, bottom);

            //draw
            bitmapoffset(place, 0);
            ascii->draw(pContext, sourcerect, bitmapoffset);

            left += width;
        }
    }

    setDirty(false);
}

void CTextDisplay::setText(const char* text)
{
    strcpy(todisplay, text);
    setDirty(true);
}
