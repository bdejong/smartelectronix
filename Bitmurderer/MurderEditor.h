// MurderEditor.h: interface for the MurderEditor class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "plugin-bindings/aeffguieditor.h"

class MurderEditor : public AEffGUIEditor, public IControlListener
{
public:
    MurderEditor(AudioEffect* effect);
    virtual ~MurderEditor();

protected:
    virtual bool open(void* ptr) override;
    virtual void close() override;

    virtual void setParameter(VstInt32 index, float value) override;

private:
    virtual void valueChanged(CControl* control) override;

    CPoint *points;
    COnOffButton **buttons;
    CSplashScreen *splash;
};
