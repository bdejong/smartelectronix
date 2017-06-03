#include "ASupaEditor.hpp"
#include "AnechoicRoomSim.hpp"
#include <stdio.h>
#include <math.h>

ASupaEditor::ASupaEditor(AudioEffect* effect) : AEffGUIEditor(effect)
{
    sizeKnob = nullptr;
    sizeDisplay = nullptr;
    frame = nullptr;

    // init the size of the plugin
    rect.left = 0;
    rect.top = 0;
    rect.right = 302;
    rect.bottom = 192;
}

bool ASupaEditor::open(void* ptr)
{
    AEffGUIEditor::open(ptr);

    CBitmap* hBackground = new CBitmap("back.png");
    CBitmap* distknob = new CBitmap("knob.png");
    CBitmap* greyText = new CBitmap("type.png");
    CBitmap* splashBitmap = new CBitmap("splash.png");

    //init frame
    CRect size(0, 0, hBackground->getWidth(), hBackground->getHeight());
    frame = new CFrame(size, this);
    frame->open(ptr);
    frame->setBackground(hBackground);
    setKnobMode(kLinearMode);

    CColor grey = { 118,118,118 };
    CColor white = { 255,255,255 };
    CColor red = { 255,0,0 };

    long displayWidth = 30;
    long displayHeight = 6;

    char temp[256];
    (static_cast<AnechoicRoomSim*>(effect))->getDisplay(AnechoicRoomSim::kSize, temp);
    sizeDisplay = new CTextDisplay(CRect(39, 159, 103, 159 + displayHeight), greyText, grey);
    sizeDisplay->setText(temp);
    frame->addView(sizeDisplay);

    // distortion
    sizeKnob = new CAnimKnob(CRect(117, 63, 117 + distknob->getWidth(), 63 + distknob->getHeight() / 111), this, AnechoicRoomSim::kSize, 111, 68, distknob, CPoint(0, 0));
    sizeKnob->setDefaultValue(0.5f);
    sizeKnob->setValue(effect->getParameter(AnechoicRoomSim::kSize));
    frame->addView(sizeKnob);

    splash = new CSplashScreen(CRect(220, 23, 275, 74), this, 1000, splashBitmap, size, CPoint(0, 0));
    frame->addView(splash);

    // forget bitmaps
    hBackground->forget();
    distknob->forget();
    greyText->forget();
    splashBitmap->forget();

    return true;
}

void ASupaEditor::close()
{
    sizeKnob = nullptr;
    sizeDisplay = nullptr;
    splash = nullptr;

    if (!frame)
    {
        CFrame* oldFrame = frame;
        frame = nullptr;
        oldFrame->forget();
    }
}

void ASupaEditor::setParameter(VstInt32 index, float value)
{
    if (!frame)
    {
        return;
    }

    if (index >= 0 && index < AnechoicRoomSim::kNumParams)
    {
        switch (index)
        {
        case AnechoicRoomSim::kSize: if (sizeKnob) sizeKnob->setValue(value); break;
        }

        char temp[256];
        (static_cast<AnechoicRoomSim*>(effect))->getDisplay(index, temp);

        switch (index)
        {
        case AnechoicRoomSim::kSize: if (sizeDisplay) sizeDisplay->setText(temp); break;
        }
    }
}

void ASupaEditor::valueChanged(CControl* control)
{
    long tag = control->getTag();

    if (tag >= 0 && tag < AnechoicRoomSim::kNumParams)
    {
        char temp[256];
        (static_cast<AnechoicRoomSim*>(effect))->getDisplay(tag, temp);

        switch (tag)
        {
        case AnechoicRoomSim::kSize: if (sizeDisplay) sizeDisplay->setText(temp); break;
        }

        //update control...
        effect->setParameterAutomated(tag, control->getValue());
    }
}
