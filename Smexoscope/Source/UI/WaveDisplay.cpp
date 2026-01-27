#include "WaveDisplay.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

static float cf_lin2db(float lin)
{
    if (lin < 1e-44f)
        return -1000.f;
    return 20.f * std::log10(lin);
}

WaveDisplay::WaveDisplay(SmexoscopeProcessor& proc,
                         const juce::Image& bgImage,
                         const juce::Image& heads,
                         const juce::Image& readout)
    : processor(proc),
      backgroundImage(bgImage),
      headsImage(heads),
      readoutImage(readout)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    headsFrame = static_cast<unsigned char>(std::rand() % 4);

    startTimerHz(60);
}

WaveDisplay::~WaveDisplay()
{
    stopTimer();
}

void WaveDisplay::timerCallback()
{
    repaint();
}

void WaveDisplay::mouseDown(const juce::MouseEvent& event)
{
    mousePos = event.getPosition();
    repaint();
}

void WaveDisplay::mouseDrag(const juce::MouseEvent& event)
{
    mousePos = event.getPosition();
    repaint();
}

void WaveDisplay::mouseUp(const juce::MouseEvent&)
{
    mousePos.x = -1;
    repaint();
}

void WaveDisplay::paint(juce::Graphics& g)
{
    int w = getWidth();
    int h = getHeight();

    // The component is placed at (38,16) in the editor, and we draw the
    // background from the same offset in the body image.
    int offsetX = 38;
    int offsetY = 16;

    // Draw background section from body.png
    g.drawImage(backgroundImage,
                0, 0, w, h,
                offsetX, offsetY, w, h);

    // Draw heads image at (615-38, 240-16) relative to this component
    int headsX = 615 - offsetX;
    int headsY = 240 - offsetY;
    int headsFrameH = headsImage.getHeight() / 4;
    g.drawImage(headsImage,
                headsX, headsY, headsImage.getWidth(), headsFrameH,
                0, headsFrame * headsFrameH, headsImage.getWidth(), headsFrameH);

    g.setColour(juce::Colour(229, 229, 229)); // anti-aliased lines
    // No setAntiAliased in JUCE Graphics, but drawLine uses AA by default with float coords

    // Trigger line
    float trigTypeVal = processor.apvts.getRawParameterValue(SmexoscopeProcessor::PARAM_TRIGGER_TYPE)->load();
    long trigType = (long)(trigTypeVal * SmexoscopeProcessor::kNumTriggerTypes + 0.0001);

    if (trigType == SmexoscopeProcessor::kTriggerRising || trigType == SmexoscopeProcessor::kTriggerFalling)
    {
        float trigLevel = processor.apvts.getRawParameterValue(SmexoscopeProcessor::PARAM_TRIGGER_LEVEL)->load();
        int y = 1 + (int)((1.f - trigLevel) * (h - 2));

        g.setColour(juce::Colour(229, 229, 229));
        g.drawLine(0.0f, (float)y, (float)(w - 1), (float)y);
    }

    // Zero line
    g.setColour(juce::Colour(179, 111, 56));
    float midY = h * 0.5f - 1.0f;
    g.drawLine(0.0f, midY, (float)(w - 1), midY);

    // Waveform
    float syncDraw = processor.apvts.getRawParameterValue(SmexoscopeProcessor::PARAM_SYNC_DRAW)->load();
    const std::vector<CPoint>& points = (syncDraw > 0.5f) ? processor.getCopy() : processor.getPeaks();

    float timeWindowVal = processor.apvts.getRawParameterValue(SmexoscopeProcessor::PARAM_TIME_WINDOW)->load();
    double counterSpeedInverse;
    if (trigType == SmexoscopeProcessor::kTriggerTempo)
        counterSpeedInverse = SmexoscopeProcessor::tempoBeatsForKnob(timeWindowVal);
    else
        counterSpeedInverse = SmexoscopeProcessor::timeKnobToBeats(4.0 * timeWindowVal) / 4.0;

    if (counterSpeedInverse < 1.0)
    {
        // Draw interpolated lines
        g.setColour(juce::Colour(64, 148, 172));

        double phase = counterSpeedInverse;
        double dphase = counterSpeedInverse;

        double prevxi = points[0].x;
        double prevyi = points[0].y;

        for (int i = 1; i < w - 1; i++)
        {
            long idx = (long)phase;
            double alpha = phase - (double)idx;

            if (static_cast<size_t>(idx * 2 + 2) < points.size())
            {
                double xi = i;
                double yi = (1.0 - alpha) * points[static_cast<size_t>(idx * 2)].y + alpha * points[static_cast<size_t>((idx + 1) * 2)].y;

                g.drawLine((float)prevxi, (float)prevyi, (float)xi, (float)yi);
                prevxi = xi;
                prevyi = yi;
            }

            phase += dphase;
        }
    }
    else
    {
        // Draw peak lines
        g.setColour(juce::Colour(118, 118, 118));

        for (size_t i = 0; i + 1 < points.size(); i++)
        {
            g.drawLine((float)points[i].x, (float)points[i].y,
                       (float)points[i + 1].x, (float)points[i + 1].y);
        }
    }

    // XY crosshair readout
    if (mousePos.x != -1)
    {
        g.setColour(juce::Colour(118, 118, 118));

        // Crosshair
        g.drawLine(0.0f, (float)mousePos.y, (float)(w - 1), (float)mousePos.y);
        g.drawLine((float)mousePos.x, 0.0f, (float)mousePos.x, (float)(h - 1));

        float ampVal = processor.apvts.getRawParameterValue(SmexoscopeProcessor::PARAM_AMP_WINDOW)->load();
        float gain = std::pow(10.f, ampVal * 6.f - 3.f);
        float y = (-2.f * ((float)mousePos.y + 1.f) / (float)OSC_HEIGHT + 1.f) / gain;
        float x = (float)mousePos.x * (float)counterSpeedInverse;

        // Draw readout background
        // Original draws at CRect(508, 8, ...).offset(offset) = (546, 24) in frame coords
        // In local coords: (546-38, 24-16) = (508, 8)
        int readoutX = 508;
        int readoutY = 8;
        g.drawImage(readoutImage,
                    readoutX, readoutY, readoutImage.getWidth(), readoutImage.getHeight(),
                    0, 0, readoutImage.getWidth(), readoutImage.getHeight());

        // Draw text
        g.setColour(juce::Colour(179, 111, 56));
        g.setFont(juce::FontOptions(10.0f));

        int textX = 512;
        int textY = 10;
        int lineSize = 10;

        char text[256];

        std::snprintf(text, sizeof(text), "y = %.5f", y);
        g.drawText(text, textX, textY, 140, lineSize, juce::Justification::left, false);
        textY += lineSize;

        std::snprintf(text, sizeof(text), "y = %.5f dB", cf_lin2db(std::fabs(y)));
        g.drawText(text, textX, textY, 140, lineSize, juce::Justification::left, false);
        textY += lineSize * 2;

        std::snprintf(text, sizeof(text), "x = %.2f samples", x);
        g.drawText(text, textX, textY, 140, lineSize, juce::Justification::left, false);
        textY += lineSize;

        double sr = processor.getSampleRate();
        std::snprintf(text, sizeof(text), "x = %.5f seconds", x / sr);
        g.drawText(text, textX, textY, 140, lineSize, juce::Justification::left, false);
        textY += lineSize;

        std::snprintf(text, sizeof(text), "x = %.5f ms", 1000.0 * x / sr);
        g.drawText(text, textX, textY, 140, lineSize, juce::Justification::left, false);
        textY += lineSize;

        if (x == 0.0f)
            std::snprintf(text, sizeof(text), "x = infinite Hz");
        else
            std::snprintf(text, sizeof(text), "x = %.3f Hz", sr / x);
        g.drawText(text, textX, textY, 140, lineSize, juce::Justification::left, false);
    }


}
