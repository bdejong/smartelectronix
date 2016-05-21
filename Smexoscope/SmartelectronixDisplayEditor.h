// AmbienceEditor.h: interface for the AmbienceEditor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AMBIENCEEDITOR_H__29C4B114_17DD_4725_AA1C_87CD3429C239__INCLUDED_)
#define AFX_AMBIENCEEDITOR_H__29C4B114_17DD_4725_AA1C_87CD3429C239__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "aeffguieditor.h"

#include "Defines.h"
#include "WaveDisplay.h"
#include "MultiStateButton.h"
#include "Label.h"

class CSmartelectronixDisplayEditor : public AEffGUIEditor, public IControlListener
{
public:

	CSmartelectronixDisplayEditor(AudioEffect *effect);
	virtual ~CSmartelectronixDisplayEditor();

	virtual CBaseObject* newCopy () const { return 0; }

protected:
	virtual bool open(void *ptr);
	virtual void close();
	virtual void idle();

	virtual void setParameter(long index, float value);
	virtual void valueChanged(CControl* control);

	CWaveDisplay *display;

	CAnimKnob *triggerSpeed;
	CAnimKnob *timeWindow;
	CAnimKnob *ampWindow;
	CAnimKnob *triggerLimit;

	CSlider *triggerLevel;

	COnOffButton *syncDraw;
	COnOffButton *freeze;
	COnOffButton *channelSelector;
	COnOffButton *dc;

	CMultiStateButton *trigger;

	CLabel *ampLabel;
	CLabel *timeLabel;
	CLabel *trigLabel;
	CLabel *threshLabel;

	char temp[256];
};

#endif // !defined(AFX_AMBIENCEEDITOR_H__29C4B114_17DD_4725_AA1C_87CD3429C239__INCLUDED_)
