// MurderEditor.h: interface for the MurderEditor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MURDEREDITOR_H__7E854C92_4DCB_459B_9308_C16BECDBB9EE__INCLUDED_)
#define AFX_MURDEREDITOR_H__7E854C92_4DCB_459B_9308_C16BECDBB9EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "vstgui.h"

class MurderEditor : public AEffGUIEditor, public CControlListener
{
public:
	MurderEditor(AudioEffect *effect);
	virtual ~MurderEditor();

protected:
    virtual long open (void *ptr);
    virtual void close ();

    void setParameter (long index, float value);
    
private:
    void valueChanged (CDrawContext* context, CControl* control);

	CPoint *points;
	COnOffButton **buttons;
	CSplashScreen *splash;
};


#endif // !defined(AFX_MURDEREDITOR_H__7E854C92_4DCB_459B_9308_C16BECDBB9EE__INCLUDED_)
