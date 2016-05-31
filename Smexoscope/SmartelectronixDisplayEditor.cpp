#include "SmartelectronixDisplayEditor.h"
#include "SmartelectronixDisplay.hpp"
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
    CBitmap* hBackground = new CBitmap("body.png");

    CRect frameSize (rect.left, rect.top, rect.right, rect.bottom);
    CFrame* newFrame = new CFrame (frameSize, this);
    newFrame->open (ptr);
    newFrame->setBackground(hBackground);

    setKnobMode(kLinearMode);

    CBitmap* heads = new CBitmap("heads.png");
    CBitmap* readout = new CBitmap("readout2.png");
    display = new CWaveDisplay(CRect(38, 16, 665, 285), (CSmartelectronixDisplay*)effect, hBackground, heads, readout);
    newFrame->addView(display);
    heads->forget();
    readout->forget();

    // top row
    CBitmap* knob = new CBitmap("blue_knob1_4.png");

    // time windows
    timeWindow = new CAnimKnob(CRect(697, 31, 697 + knob->getWidth(), 31 + knob->getHeight() / 75), this, CSmartelectronixDisplay::kTimeWindow, 75, 33, knob, CPoint(0, 0));
    timeWindow->setDefaultValue(0.5f);
    newFrame->addView(timeWindow);

    ((CSmartelectronixDisplay*)effect)->getDisplay(CSmartelectronixDisplay::kTimeWindow, temp);
    timeLabel = new CLabel(CRect(696, 72, 730, 83), temp);
    newFrame->addView(timeLabel);

    // amp window
    ampWindow = new CAnimKnob(CRect(762, 31, 762 + knob->getWidth(), 31 + knob->getHeight() / 75), this, CSmartelectronixDisplay::kAmpWindow, 75, 33, knob, CPoint(0, 0));
    ampWindow->setDefaultValue(0.5f);
    newFrame->addView(ampWindow);

    ((CSmartelectronixDisplay*)effect)->getDisplay(CSmartelectronixDisplay::kAmpWindow, temp);
    ampLabel = new CLabel(CRect(760, 72, 794, 83), temp);
    newFrame->addView(ampLabel);

    // trigger speed
    triggerSpeed = new CAnimKnob(CRect(700, 134, 700 + knob->getWidth(), 134 + knob->getHeight() / 75), this, CSmartelectronixDisplay::kTriggerSpeed, 75, 33, knob, CPoint(0, 0));
    triggerSpeed->setDefaultValue(0.5f);
    newFrame->addView(triggerSpeed);

    ((CSmartelectronixDisplay*)effect)->getDisplay(CSmartelectronixDisplay::kTriggerSpeed, temp);
    trigLabel = new CLabel(CRect(698, 175, 732, 186), temp);
    newFrame->addView(trigLabel);

    // trigger limit
    triggerLimit = new CAnimKnob(CRect(765, 134, 765 + knob->getWidth(), 134 + knob->getHeight() / 75), this, CSmartelectronixDisplay::kTriggerLimit, 75, 33, knob, CPoint(0, 0));
    triggerLimit->setDefaultValue(0.5f);
    newFrame->addView(triggerLimit);

    ((CSmartelectronixDisplay*)effect)->getDisplay(CSmartelectronixDisplay::kTriggerLimit, temp);
    threshLabel = new CLabel(CRect(765, 175, 799, 186), temp);
    newFrame->addView(threshLabel);

    knob->forget();

    // trigger type
    CBitmap* triggerBitmap = new CBitmap("free_etc.png");
    trigger = new CMultiStateButton(CRect(718, 94, 718 + triggerBitmap->getWidth(), 94 + triggerBitmap->getHeight() / 5), this, CSmartelectronixDisplay::kTriggerType, triggerBitmap, 4, triggerBitmap->getHeight() / 5);
    newFrame->addView(trigger);
    triggerBitmap->forget();

    CBitmap* onOff = new CBitmap("off_on.png");
    CBitmap* channel = new CBitmap("lefr_right.png");

    // sync redraw
    syncDraw = new COnOffButton(CRect(696, 221, 696 + onOff->getWidth(), 221 + onOff->getHeight() / 2), this, CSmartelectronixDisplay::kSyncDraw, onOff);
    newFrame->addView(syncDraw);

    // dc
    dc = new COnOffButton(CRect(690, 259, 690 + onOff->getWidth(), 259 + onOff->getHeight() / 2), this, CSmartelectronixDisplay::kDCKill, onOff);
    newFrame->addView(dc);

    // freeze
    freeze = new COnOffButton(CRect(754, 221, 754 + onOff->getWidth(), 221 + onOff->getHeight() / 2), this, CSmartelectronixDisplay::kFreeze, onOff);
    newFrame->addView(freeze);

    // channel
    channelSelector = new COnOffButton(CRect(748, 259, 748 + onOff->getWidth(), 259 + onOff->getHeight() / 2), this, CSmartelectronixDisplay::kChannel, channel);
    newFrame->addView(channelSelector);

    onOff->forget();
    channel->forget();

    //trigger level slider
    CBitmap* myFaderHandlePixmap = new CBitmap("slider_new.png");

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
    triggerLevel->setDrawTransparentHandle(false);
    newFrame->addView(triggerLevel);

    myFaderHandlePixmap->forget();

    frame = newFrame;
    hBackground->forget();

    for (int i = 0; i < CSmartelectronixDisplay::kNumParams; i++)
        setParameter(i, effect->getParameter(i));

    return true;
}

void CSmartelectronixDisplayEditor::close()
{
    CFrame* oldFrame = frame;
    frame = 0;
    oldFrame->forget ();
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

    switch (index) {
    case CSmartelectronixDisplay::kTriggerLimit:
        ((CSmartelectronixDisplay*)effect)->getDisplay(index, temp);
        threshLabel->setLabel(temp);
        break;
    case CSmartelectronixDisplay::kTriggerSpeed:
        ((CSmartelectronixDisplay*)effect)->getDisplay(index, temp);
        trigLabel->setLabel(temp);
        break;
    case CSmartelectronixDisplay::kTimeWindow:
        ((CSmartelectronixDisplay*)effect)->getDisplay(index, temp);
        timeLabel->setLabel(temp);
        break;
    case CSmartelectronixDisplay::kAmpWindow:
        ((CSmartelectronixDisplay*)effect)->getDisplay(index, temp);
        ampLabel->setLabel(temp);
        break;
    }
}

void CSmartelectronixDisplayEditor::valueChanged(CControl* control)
{
    long tag = control->getTag();

    if (tag >= 0 && tag < CSmartelectronixDisplay::kNumParams) {
        //get display...

        switch (tag) {
        case CSmartelectronixDisplay::kTriggerLimit:
            ((CSmartelectronixDisplay*)effect)->getDisplay(tag, temp);
            threshLabel->setLabel(temp);
            break;
        case CSmartelectronixDisplay::kTriggerSpeed:
            ((CSmartelectronixDisplay*)effect)->getDisplay(tag, temp);
            trigLabel->setLabel(temp);
            break;
        case CSmartelectronixDisplay::kTimeWindow:
            ((CSmartelectronixDisplay*)effect)->getDisplay(tag, temp);
            timeLabel->setLabel(temp);
            break;
        case CSmartelectronixDisplay::kAmpWindow:
            ((CSmartelectronixDisplay*)effect)->getDisplay(tag, temp);
            ampLabel->setLabel(temp);
            break;
        }

        //update control...
        effect->setParameterAutomated(tag, control->getValue());
    }
}
