// TextDisplay.h: interface for the CTextDisplay class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "vstgui.h"

class CTextDisplay : public CParamDisplay
{
public:
	CTextDisplay(CRect &size, CBitmap *ascii, CColor rectColor);
	virtual ~CTextDisplay();

	void setAlignMiddle() { middle = true; };

	virtual void draw (CDrawContext* pContext);
	void setText(char *text);

private:
	char _todisplay[255];	
	CColor _textColor;
	CBitmap *_ascii;

	CColor _rectColor;
	bool middle;
};
