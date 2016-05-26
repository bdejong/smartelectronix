// CustomVU.cpp: implementation of the CCustomVU class.
//
//////////////////////////////////////////////////////////////////////

#include "WaveDisplay.h"
#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------
// CVuMeter
//------------------------------------------------------------------------
CWaveDisplay::CWaveDisplay(const CRect& size, CSmartelectronixDisplay* effect, CBitmap* back, CBitmap* heads, CBitmap* readout)
    : CControl(size, 0, 0)
    , effect(effect)
    , back(back)
    , heads(heads)
    , readout(readout)
{
    value = oldValue = 0.f;

    srand((unsigned)time(NULL));
    display = rand() % 4;

    if (heads)
        heads->remember();
    if (back)
        back->remember();
    if (readout)
        readout->remember();

    where.x = -1;
    timer = new CVSTGUITimer(this, 1000/60, true);
}

CMessageResult CWaveDisplay::notify (CBaseObject* sender, IdStringPtr message)
{
    if (message == CVSTGUITimer::kMsgTimer)
    {
        invalid();

        return kMessageNotified;
    }
    return kMessageUnknown;
}

//------------------------------------------------------------------------
CWaveDisplay::~CWaveDisplay()
{
    if (timer)
        timer->forget();
    if (heads)
        heads->forget();
    if (back)
        back->forget();
    if (readout)
        readout->forget();
}

CMouseEventResult CWaveDisplay::onMouseDown(CPoint& where, const CButtonState& buttons)
{
    if (buttons.isLeftButton() && getViewSize().pointInside(where))
    {
        this->where = where;
        return CMouseEventResult::kMouseEventHandled;
    }

    return CMouseEventResult::kMouseEventNotHandled;
}

CMouseEventResult CWaveDisplay::onMouseMoved(CPoint& where, const CButtonState& buttons)
{
    if (buttons.isLeftButton() && getViewSize().pointInside(where))
        this->where = where;

    return CMouseEventResult::kMouseEventHandled;
}

CMouseEventResult CWaveDisplay::onMouseUp(CPoint& where, const CButtonState& buttons)
{
    this->where.x = -1;
    return CMouseEventResult::kMouseEventHandled;
}

inline float cf_lin2db(float lin)
{
    if (lin < 9e-51)
        return -1000.f; /* prevent invalid operation */
    return 20.f * log10f(lin);
}

//------------------------------------------------------------------------
void CWaveDisplay::draw(CDrawContext* pContext)
{
    CPoint offset(38, 16);

    CRect R(0, 0, size.getWidth(), size.getHeight());
    back->draw(pContext, R.offset(offset), offset);

    R(615 - size.left, 240 - size.top, 615 + heads->getWidth() - size.left, 240 + heads->getHeight() / 4 - size.top);
    heads->draw(pContext, R.offset(offset), CPoint(0, (display * heads->getHeight()) / 4));

    pContext->setDrawMode(CDrawMode(kAntiAliasing));

    // trig-line
    long triggerType = (long)(effect->getParameter(CSmartelectronixDisplay::kTriggerType) * CSmartelectronixDisplay::kNumTriggerTypes + 0.0001);

    if (triggerType == CSmartelectronixDisplay::kTriggerRising || triggerType == CSmartelectronixDisplay::kTriggerFalling) {
        long y = 1 + (long)((1.f - effect->getParameter(CSmartelectronixDisplay::kTriggerLevel)) * (size.getHeight() - 2));

        CColor grey(229, 229, 229);
        pContext->setFrameColor(grey);
        pContext->drawLine(CPoint(0, y).offset(offset), CPoint(size.getWidth() - 1, y).offset(offset));
    }

    // zero-line
    CColor orange(179, 111, 56);
    pContext->setFrameColor(orange);
    pContext->drawLine(CPoint(0, size.getHeight() * 0.5 - 1).offset(offset), CPoint(size.getWidth() - 1, size.getHeight() * 0.5 - 1).offset(offset));

    // waveform
    const std::vector<CPoint>& points = (effect->getParameter(CSmartelectronixDisplay::kSyncDraw) > 0.5f) ? effect->getCopy() : effect->getPeaks();
    double counterSpeedInverse = pow(10.f, effect->getParameter(CSmartelectronixDisplay::kTimeWindow) * 5.f - 1.5);

    if (counterSpeedInverse < 1.0) //draw interpolated lines!
    {
        CColor blue(64, 148, 172);
        pContext->setFrameColor(blue);

        double phase = counterSpeedInverse;
        double dphase = counterSpeedInverse;

        double prevxi = points[0].x;
        double prevyi = points[0].y;

        for (long i = 1; i < size.getWidth() - 1; i++) {
            long index = (long)phase;
            double alpha = phase - (double)index;

            double xi = i;
            double yi = (1.0 - alpha) * points[index * 2].y + alpha * points[(index + 1) * 2].y;

            pContext->drawLine(CPoint(prevxi, prevyi).offset(offset), CPoint(xi, yi).offset(offset));
            prevxi = xi;
            prevyi = yi;

            phase += dphase;
        }
    } else {
        CColor grey(118, 118, 118);
        pContext->setFrameColor(grey);

        CPoint p1, p2;
        for (unsigned int i=0; i<points.size()-1; i++)
        {
            p1 = points[i];
            p2 = points[i+1];
            pContext->drawLine(p1.offset(offset), p2.offset(offset));
        }
    }

    //TODO clean this mess up...
    if (where.x != -1) {
        CPoint whereOffset = where;
        whereOffset.offsetInverse(offset);

        pContext->drawLine(CPoint(0, whereOffset.y).offset(offset), CPoint(size.getWidth() - 1, whereOffset.y).offset(offset));
        pContext->drawLine(CPoint(whereOffset.x, 0).offset(offset), CPoint(whereOffset.x, size.getHeight() - 1).offset(offset));

        float gain = powf(10.f, effect->getParameter(CSmartelectronixDisplay::kAmpWindow) * 6.f - 3.f);
        float y = (-2.f * ((float)whereOffset.y + 1.f) / (float)OSC_HEIGHT + 1.f) / gain;
        float x = (float)whereOffset.x * (float)counterSpeedInverse;
        char text[256];

        long lineSize = 10;

        //CColor color = { 169, 101, 46, 0 };
        //CColor color = { 0, 0, 0, 0 };
        CColor color(179, 111, 56);

        pContext->setFontColor(color);
        pContext->setFont(kNormalFontSmaller);

        readout->draw(pContext, CRect(508, 8, 508 + readout->getWidth(), 8 + readout->getHeight()).offset(offset), CPoint(0, 0));

        CRect textRect(512, 10, 652, 10 + lineSize);
        textRect.offset(offset);

        sprintf(text, "y = %.5f", y);
        pContext->drawString(text, textRect, kLeftText, true);
        textRect.offset(0, lineSize);

        sprintf(text, "y = %.5f dB", cf_lin2db(fabsf(y)));
        pContext->drawString(text, textRect, kLeftText, true);
        textRect.offset(0, lineSize * 2);

        sprintf(text, "x = %.2f samples", x);
        pContext->drawString(text, textRect, kLeftText, true);
        textRect.offset(0, lineSize);

        sprintf(text, "x = %.5f seconds", x / effect->getSampleRate());
        pContext->drawString(text, textRect, kLeftText, true);
        textRect.offset(0, lineSize);

        sprintf(text, "x = %.5f ms", 1000.f * x / effect->getSampleRate());
        pContext->drawString(text, textRect, kLeftText, true);
        textRect.offset(0, lineSize);

        if (x == 0)
            sprintf(text, "x = infinite Hz");
        else
            sprintf(text, "x = %.3f Hz", effect->getSampleRate() / x);

        pContext->drawString(text, textRect, kLeftText, true);
    }
}
