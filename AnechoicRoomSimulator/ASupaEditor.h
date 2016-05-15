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

	CAnimKnob *sizeKnob;

	CTextDisplay *sizeDisplay;

	CSplashScreen *splash;
};

#endif // !defined(AFX_ASUPAEDITOR_H__B92F8BC1_D048_11D3_9312_C56A22663A38__INCLUDED_)
