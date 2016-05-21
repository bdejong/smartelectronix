#include "SmartelectronixDisplayEditor.h"
#include "SmartelectronixDisplay.hpp"
#include "resource.h"
#include "stdlib.h"

CSmartelectronixDisplayEditor::CSmartelectronixDisplayEditor(AudioEffect* effect)
    : AEffGUIEditor(effect)
{
    // init the size of the plugin
    rect.left = 0;
    rect.top = 0;
    rect.right = BACK_WIDTH;
    rect.bottom = BACK_HEIGHT;

    display = 0;
}

CSmartelectronixDisplayEditor::~CSmartelectronixDisplayEditor()
{
}

bool CSmartelectronixDisplayEditor::open(void* ptr)
{
    // !!! always call this !!!
    AEffGUIEditor::open(ptr);

    CBitmap* hBackground = new CBitmap(IDB_BITMAPBACKGROUND);
    CBitmap* heads = new CBitmap(IDB_BITMAPHEADS);
    CBitmap* readout = new CBitmap(IDB_BITMAPREADOUT);

    //init frame
    CRect size(0, 0, hBackground->getWidth(), hBackground->getHeight());
    frame = new CFrame(size, this);
    frame->setBackground(hBackground);
    setKnobMode(kLinearMode);

    //display = new CWaveDisplay(CRect(38,16,663,285),(CSmartelectronixDisplay*)effect,hBackground,heads,readout);
    display = new CWaveDisplay(CRect(38, 16, 665, 285), (CSmartelectronixDisplay*)effect, hBackground, heads, readout);
    frame->addView(display);
    heads->forget();
    readout->forget();

    // top row
    CBitmap* knob = new CBitmap(IDB_BITMAPKNOB);

    // time windows
    timeWindow = new CAnimKnob(CRect(697, 31, 697 + knob->getWidth(), 31 + knob->getHeight() / 75), this, CSmartelectronixDisplay::kTimeWindow, 75, 33, knob, CPoint(0, 0));
    timeWindow->setDefaultValue(0.5f);
    timeWindow->setValue(effect->getParameter(CSmartelectronixDisplay::kTimeWindow));
    frame->addView(timeWindow);

    ((CSmartelectronixDisplay*)effect)->getDisplay(CSmartelectronixDisplay::kTimeWindow, temp);
    timeLabel = new CLabel(CRect(696, 72, 730, 83), temp);
    frame->addView(timeLabel);

    // amp window
    ampWindow = new CAnimKnob(CRect(762, 31, 762 + knob->getWidth(), 31 + knob->getHeight() / 75), this, CSmartelectronixDisplay::kAmpWindow, 75, 33, knob, CPoint(0, 0));
    ampWindow->setDefaultValue(0.5f);
    ampWindow->setValue(effect->getParameter(CSmartelectronixDisplay::kAmpWindow));
    frame->addView(ampWindow);

    ((CSmartelectronixDisplay*)effect)->getDisplay(CSmartelectronixDisplay::kAmpWindow, temp);
    ampLabel = new CLabel(CRect(760, 72, 794, 83), temp);
    frame->addView(ampLabel);

    // trigger speed
    triggerSpeed = new CAnimKnob(CRect(700, 134, 700 + knob->getWidth(), 134 + knob->getHeight() / 75), this, CSmartelectronixDisplay::kTriggerSpeed, 75, 33, knob, CPoint(0, 0));
    triggerSpeed->setDefaultValue(0.5f);
    triggerSpeed->setValue(effect->getParameter(CSmartelectronixDisplay::kTriggerSpeed));
    frame->addView(triggerSpeed);

    ((CSmartelectronixDisplay*)effect)->getDisplay(CSmartelectronixDisplay::kTriggerSpeed, temp);
    trigLabel = new CLabel(CRect(698, 175, 732, 186), temp);
    frame->addView(trigLabel);

    // trigger limit
    triggerLimit = new CAnimKnob(CRect(765, 134, 765 + knob->getWidth(), 134 + knob->getHeight() / 75), this, CSmartelectronixDisplay::kTriggerLimit, 75, 33, knob, CPoint(0, 0));
    triggerLimit->setDefaultValue(0.5f);
    triggerLimit->setValue(effect->getParameter(CSmartelectronixDisplay::kTriggerLimit));
    frame->addView(triggerLimit);

    ((CSmartelectronixDisplay*)effect)->getDisplay(CSmartelectronixDisplay::kTriggerLimit, temp);
    threshLabel = new CLabel(CRect(765, 175, 799, 186), temp);
    frame->addView(threshLabel);

    knob->forget();

    // trigger type
    CBitmap* triggerBitmap = new CBitmap(IDB_BITMAPTRIGGER);
#if SIMPLE_VERSION
    trigger = new CMultiStateButton(CRect(718, 94, 718 + triggerBitmap->getWidth(), 94 + triggerBitmap->getHeight() / 5), this, CSmartelectronixDisplay::kTriggerType, triggerBitmap, 4, triggerBitmap->getHeight() / 5);
#else
    trigger = new CMultiStateButton(CRect(718, 94, 718 + triggerBitmap->getWidth(), 94 + triggerBitmap->getHeight() / 5), this, CSmartelectronixDisplay::kTriggerType, triggerBitmap, 5, triggerBitmap->getHeight() / 5);
#endif
    trigger->setValue(effect->getParameter(CSmartelectronixDisplay::kTriggerType));
    frame->addView(trigger);
    triggerBitmap->forget();

    CBitmap* onOff = new CBitmap(IDB_BITMAPONOFF);
    CBitmap* channel = new CBitmap(IDB_BITMAPCHANNEL);

    // sync redraw
    syncDraw = new COnOffButton(CRect(696, 221, 696 + onOff->getWidth(), 221 + onOff->getHeight() / 2), this, CSmartelectronixDisplay::kSyncDraw, onOff);
    syncDraw->setValue(effect->getParameter(CSmartelectronixDisplay::kSyncDraw));
    frame->addView(syncDraw);

    // dc
    dc = new COnOffButton(CRect(690, 259, 690 + onOff->getWidth(), 259 + onOff->getHeight() / 2), this, CSmartelectronixDisplay::kDCKill, onOff);
    dc->setValue(effect->getParameter(CSmartelectronixDisplay::kDCKill));
    frame->addView(dc);

    // freeze
    freeze = new COnOffButton(CRect(754, 221, 754 + onOff->getWidth(), 221 + onOff->getHeight() / 2), this, CSmartelectronixDisplay::kFreeze, onOff);
    freeze->setValue(effect->getParameter(CSmartelectronixDisplay::kFreeze));
    frame->addView(freeze);

    // channel
    channelSelector = new COnOffButton(CRect(748, 259, 748 + onOff->getWidth(), 259 + onOff->getHeight() / 2), this, CSmartelectronixDisplay::kChannel, channel);
    channelSelector->setValue(effect->getParameter(CSmartelectronixDisplay::kChannel));
    frame->addView(channelSelector);

    onOff->forget();
    channel->forget();

    //trigger level slider
    CBitmap* myFaderHandlePixmap = new CBitmap(IDB_BITMAPSLIDERHANDLE);

    int sliderWidth = myFaderHandlePixmap->getWidth();
    int sliderHeight = 277;
    int sliderTop = 13;
    int sliderLeft = 11;
    int minPos = sliderTop;
    int maxPos = sliderTop + sliderHeight - myFaderHandlePixmap->getHeight() - 1;
    triggerLevel = new CSlider(
        CRect(sliderLeft, sliderTop, sliderLeft + sliderWidth, sliderTop + sliderHeight),
        this,
        CSmartelectronixDisplay::kTriggerLevel,
        minPos,
        maxPos,
        myFaderHandlePixmap,
        hBackground,
        CPoint(sliderLeft, sliderTop),
        kVertical | kBottom);
    triggerLevel->setDefaultValue(0.5f);
    triggerLevel->setValue(effect->getParameter(CSmartelectronixDisplay::kTriggerLevel));
    triggerLevel->setDrawTransparentHandle(false);
    frame->addView(triggerLevel);

    myFaderHandlePixmap->forget();
    hBackground->forget();

    return true;
}

void CSmartelectronixDisplayEditor::close()
{
    /*
	if(frame != 0)
	{
		delete frame;
		frame = 0;
	}
	*/
}

void CSmartelectronixDisplayEditor::setParameter(long index, float value)
{
    if (!frame)
        return;

    switch (index) {
    case CSmartelectronixDisplay::kDCKill:
        dc->setValue(value);
        break;
    case CSmartelectronixDisplay::kChannel:
        channelSelector->setValue(value);
        break;
    case CSmartelectronixDisplay::kFreeze:
        freeze->setValue(value);
        break;
    case CSmartelectronixDisplay::kTriggerLimit:
        triggerLimit->setValue(value);
        break;
    case CSmartelectronixDisplay::kTriggerSpeed:
        triggerSpeed->setValue(value);
        break;
    case CSmartelectronixDisplay::kTimeWindow:
        timeWindow->setValue(value);
        break;
    case CSmartelectronixDisplay::kAmpWindow:
        ampWindow->setValue(value);
        break;
    case CSmartelectronixDisplay::kSyncDraw:
        syncDraw->setValue(value);
        break;
    case CSmartelectronixDisplay::kTriggerType:
        trigger->setValue(value);
        break;
    case CSmartelectronixDisplay::kTriggerLevel:
        triggerLevel->setValue(value);
        break;
    default:
        break;
    }

    ((CSmartelectronixDisplay*)effect)->getDisplay(index, temp);

    switch (index) {
    case CSmartelectronixDisplay::kTriggerLimit:
        threshLabel->setLabel(temp);
        break;
    case CSmartelectronixDisplay::kTriggerSpeed:
        trigLabel->setLabel(temp);
        break;
    case CSmartelectronixDisplay::kTimeWindow:
        timeLabel->setLabel(temp);
        break;
    case CSmartelectronixDisplay::kAmpWindow:
        ampLabel->setLabel(temp);
        break;
    }

    //postUpdate();
}

void CSmartelectronixDisplayEditor::valueChanged(CControl* control)
{
    long tag = control->getTag();

    if (tag >= 0 && tag < CSmartelectronixDisplay::kNumParams) {
        //get display...
        ((CSmartelectronixDisplay*)effect)->getDisplay(tag, temp);

        switch (tag) {
        case CSmartelectronixDisplay::kTriggerLimit:
            threshLabel->setLabel(temp);
            break;
        case CSmartelectronixDisplay::kTriggerSpeed:
            trigLabel->setLabel(temp);
            break;
        case CSmartelectronixDisplay::kTimeWindow:
            timeLabel->setLabel(temp);
            break;
        case CSmartelectronixDisplay::kAmpWindow:
            ampLabel->setLabel(temp);
            break;
        }

        //update control...
        effect->setParameterAutomated(tag, control->getValue());
    }

    //if(context != 0)
    //	control->update(context);
}

void CSmartelectronixDisplayEditor::idle()
{
    AEffGUIEditor::idle();

    //make the display redraw itself!
    display->setDirty(0.f);
}
