// TextDisplay.h: interface for the CTextDisplay class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXTDISPLAY_H__05B21240_4B75_11D4_B567_002018B8E8B7__INCLUDED_)
#define AFX_TEXTDISPLAY_H__05B21240_4B75_11D4_B567_002018B8E8B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "vstgui.h"

class CTextDisplay  : public CControl
{
public:
	CTextDisplay(CRect &size, CControlListener *listener, int tag);
	virtual ~CTextDisplay();
	virtual void draw (CDrawContext* pContext);
	void setText(char *text);

private:
	char todisplay[255];	
	CColor textColor;
	CBitmap *ascii;
};

#endif // !defined(AFX_TEXTDISPLAY_H__05B21240_4B75_11D4_B567_002018B8E8B7__INCLUDED_)
