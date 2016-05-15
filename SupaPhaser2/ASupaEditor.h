// ASupaEditor.h: interface for the ASupaEditor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ASUPAEDITOR_H__B92F8BC1_D048_11D3_9312_C56A22663A38__INCLUDED_)
#define AFX_ASUPAEDITOR_H__B92F8BC1_D048_11D3_9312_C56A22663A38__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "vstgui.h"

#include "TextDisplay.h"
#include "MultiStateButton.h"
#include "CustomSplashScreen.h"

class ASupaEditor:public AEffGUIEditor, public CControlListener  
{
public:
	ASupaEditor(AudioEffect *effect);
	virtual ~ASupaEditor();

	virtual long open (void *ptr);
	virtual void close ();

	virtual void setParameter (long index, float value);
	virtual void valueChanged (CDrawContext* context, CControl* control);

protected:

	CAnimKnob *attack, *release, *envMin, *envMax;
	CAnimKnob *freq, *stereo, *lfoMin, *lfoMax, *feedback, *stages, *distort, *mixture;

	CTextDisplay *attackDisplay, *releaseDisplay, *envMinDisplay, *envMaxDisplay;
	CTextDisplay *freqDisplay, *stereoDisplay, *lfoMinDisplay, *lfoMaxDisplay;
	CTextDisplay *feedbackDisplay, *stagesDisplay;

	CTextDisplay *mixtureDisplay1, *mixtureDisplay2;

	CTextDisplay *dryWetDisplay, *gainDisplay;

	CVerticalSlider *dryWet, *gain;

	COnOffButton *extend;

	CMultiStateButton *headButton;

	CCustomSplashScreen *splashScreen;
	
	char temp[256];
};

#endif // !defined(AFX_ASUPAEDITOR_H__B92F8BC1_D048_11D3_9312_C56A22663A38__INCLUDED_)
