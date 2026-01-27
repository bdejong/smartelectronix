#pragma once

#include "plugin-bindings/aeffguieditor.h"

#include "TextDisplay.hpp"

class ASupaEditor :public AEffGUIEditor, public IControlListener
{
public:
    ASupaEditor(AudioEffect* effect);
    virtual ~ASupaEditor() = default;

    virtual bool open(void* ptr) override;
    virtual void close() override;

    virtual void setParameter(VstInt32 index, float value) override;
    virtual void valueChanged(CControl* control) override;

protected:
    CAnimKnob* sizeKnob;
    CTextDisplay* sizeDisplay;
    CSplashScreen* splash;
};
