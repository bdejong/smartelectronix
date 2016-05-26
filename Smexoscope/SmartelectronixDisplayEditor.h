#pragma once

#include "audioeffectx.h"

#include "aeffguieditor.h"

#include "Defines.h"
#include "Label.h"
#include "MultiStateButton.h"
#include "WaveDisplay.h"

class CSmartelectronixDisplayEditor : public AEffGUIEditor, public IControlListener {
public:
    CSmartelectronixDisplayEditor(AudioEffect* effect);
    virtual ~CSmartelectronixDisplayEditor();

protected:
    virtual bool open(void* ptr);
    virtual void close();
    virtual void setParameter(long index, float value);
    virtual void valueChanged(CControl* control);

    CWaveDisplay* display;

    CAnimKnob* triggerSpeed;
    CAnimKnob* timeWindow;
    CAnimKnob* ampWindow;
    CAnimKnob* triggerLimit;

    CSlider* triggerLevel;

    COnOffButton* syncDraw;
    COnOffButton* freeze;
    COnOffButton* channelSelector;
    COnOffButton* dc;

    CMultiStateButton* trigger;

    CLabel* ampLabel;
    CLabel* timeLabel;
    CLabel* trigLabel;
    CLabel* threshLabel;

    char temp[256];
};
