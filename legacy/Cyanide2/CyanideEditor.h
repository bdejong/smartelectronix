#if !defined(AFX_CYANIDEEDITOR_H__4542A201_F9E8_11D3_A03E_00AA00419C92__INCLUDED_)
#define AFX_CYANIDEEDITOR_H__4542A201_F9E8_11D3_A03E_00AA00419C92__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "vstgui.h"
#include "ShaperView.h"
#include "Preview.h"
#include "Shaper.h"
#include "Label.h"

enum
{
	kSplash = 100,
	kPreview,
	kGuides,
	kReset
};

class CyanideEditor : public AEffGUIEditor, public CControlListener  
{
public:
	CyanideEditor(AudioEffect *effect, CShaper *pShaper);
	virtual ~CyanideEditor();
protected:
	virtual long open (void *ptr);
	virtual void close ();

	virtual void setParameter (long index, float value);
	virtual void valueChanged (CDrawContext* context, CControl* control);

	void setDisplayText(long index);

	CKnob *hPreGain;
	CKnob *hPreFilter;
	CKnob *hPostGain;
	CKnob *hPostFilter;
	CKnob *hPreType;
	CKnob *hPostType;
	CKnob *hDryWet;

	CSplashScreen *hSplashScreen;

	COnOffButton *hGuides;
	CKickButton  *hReset;

	COnOffButton *hOversample;

	CShaperView *hShaper;
	CPreview *hPreview;

	CShaper *pShaper;

	CLabel *hLabel;

	long opened;
};

#endif

