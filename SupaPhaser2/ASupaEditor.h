// ASupaEditor.h: interface for the ASupaEditor class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "vstgui.h"
#include "plugin-bindings/aeffguieditor.h"

#include "TextDisplay.h"
#include "MultiStateButton.h"
#include "CustomSplashScreen.h"

class ASupaEditor:public AEffGUIEditor, public IControlListener
{
public:
	ASupaEditor(AudioEffect *effect);
	virtual ~ASupaEditor();

	virtual bool open(void *ptr) override;
	virtual void close() override;

	virtual void setParameter(VstInt32 index, float value) override;
	virtual void valueChanged(CControl* control) override;

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
