// TextDisplay.cpp: implementation of the CTextDisplay class.
//
//////////////////////////////////////////////////////////////////////

#include "TextDisplay.h"
#include "asciitable.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTextDisplay::CTextDisplay(const CRect &size, CBitmap *ascii, CColor rectColor)
	: CParamDisplay (size)
{
	_ascii = ascii;

	_ascii->remember();

	_rectColor = rectColor;

	fillasciitable();

	middle = false;
}

CTextDisplay::~CTextDisplay()
{
	_ascii->forget();
}

void CTextDisplay::draw (CDrawContext* pContext)
{
	//this should be done off-screen, but I didn't feel like it.
	//I'll fix it in the next version (+ Mat's aplha-strip...)
	
	CRect tmprect = getViewSize();
	tmprect.offset(3,-1);

#ifdef _DEBUG
	pContext->setFillColor(kRedCColor);
#else
	pContext->setFillColor(_rectColor);
#endif

	pContext->setFrameColor(_rectColor);
	
	pContext->drawRect(getViewSize());
	//pContext->fillRect(size);
	
	CRect sourcerect;
	CPoint bitmapoffset;

	int left = getViewSize().left; //our staring point!
	int top = getViewSize().top; //our staring point!
	int bottom = getViewSize().top + _ascii->getHeight(); //our staring point!
	int place;
	int width;

	long totalWidth = 0;

	for(int i=0;i<256;i++)
	{
		if(_todisplay[i] == 0)
			break;
			
		PlaceAndWidth(_todisplay[i],place,width);

		if(place != -1)
		{
			totalWidth += width;
		}
	}

	if(middle)
		left += (getViewSize().getWidth() - totalWidth) / 2;
	else
		left += getViewSize().getWidth() - totalWidth;

	for(int i=0;i<256;i++)
	{
		if(_todisplay[i] == 0)
			break;
			
		PlaceAndWidth(_todisplay[i],place,width);

		if(place != -1)
		{
			sourcerect(left,top,left+width,bottom);

			//draw
			bitmapoffset(place,0);
			_ascii->draw(pContext,sourcerect,bitmapoffset);

			left += width;
		}
	}
	
	setDirty(false);
}

void CTextDisplay::setText(char *text)
{
	strcpy(_todisplay,text);
	setDirty(true);
}
