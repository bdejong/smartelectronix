#pragma once

#include "plugin-bindings/aeffguieditor.h"

class CTextDisplay;

enum {
    kSplash1 = 500,
    kSplash2,
    kSplash3
};

class H2Oeditor : public AEffGUIEditor, public IControlListener {
public:
    H2Oeditor(AudioEffect* effect);
    virtual ~H2Oeditor();

protected:
    virtual bool open(void* ptr) override;
    virtual void close() override;

    virtual void setParameter (VstInt32 index, float value) override;
    virtual void valueChanged (CControl* pControl) override;

    // Bitmaps
    CBitmap* hBack;
    CBitmap* hKnob;
    CBitmap* hSaturate;
    CBitmap* hSwitch;
    CBitmap* hSplash;

    CAnimKnob *attack, *release, *ingain, *outgain, *amount;
    COnOffButton* saturate;
    CTextDisplay* text;
    CSplashScreen *splash1, *splash2, *splash3;
};
