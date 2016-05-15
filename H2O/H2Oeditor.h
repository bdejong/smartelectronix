// H2Oeditor.h: interface for the H2Oeditor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_H2OEDITOR_H__736D3461_4B5F_11D4_B567_002018B8E8B7__INCLUDED_)
#define AFX_H2OEDITOR_H__736D3461_4B5F_11D4_B567_002018B8E8B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "vstgui.h"
#include "TextDisplay.h"

enum
{
	kSplash1 = 500,
	kSplash2,
	kSplash3
};

class H2Oeditor : public AEffGUIEditor, public CControlListener
{
public:
	H2Oeditor(AudioEffect *effect);
	virtual ~H2Oeditor();
protected:
	virtual long open (void *ptr);
	virtual void close ();

	virtual void setParameter (long index, float value);
	virtual void valueChanged (CDrawContext* context, CControl* control);

	// Bitmaps
	CBitmap *hBack;
	CBitmap *hKnob;
	CBitmap *hSaturate;
	CBitmap *hSwitch;
	CBitmap *hSplash;

	CAnimKnob *attack, *release, *ingain, *outgain, *amount;
	COnOffButton *saturate;
	CTextDisplay *text;
	CSplashScreen *splash1,*splash2,*splash3;
};

#endif // !defined(AFX_H2OEDITOR_H__736D3461_4B5F_11D4_B567_002018B8E8B7__INCLUDED_)
