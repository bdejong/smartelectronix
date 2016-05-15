// TextDisplay.h: interface for the CTextDisplay class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXTDISPLAY_H__05B21240_4B75_11D4_B567_002018B8E8B7__INCLUDED_)
#define AFX_TEXTDISPLAY_H__05B21240_4B75_11D4_B567_002018B8E8B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

#endif // !defined(AFX_TEXTDISPLAY_H__05B21240_4B75_11D4_B567_002018B8E8B7__INCLUDED_)
