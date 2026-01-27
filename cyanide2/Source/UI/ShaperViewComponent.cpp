#include "ShaperViewComponent.h"
#include <array>
#include <cmath>

ShaperViewComponent::ShaperViewComponent(CShaper& s, const juce::Image& bg,
                                         const juce::Image& handle)
    : shaper(s), bgImage(bg), handleImg(handle)
{
    long border = 2;
    float hw = handleImg.getWidth() * 0.5f + 1.0f;
    float hh = handleImg.getHeight() * 0.5f + 1.0f;
    // These will be recalculated in paint based on actual bounds
    minx = hw + (float)border;
    miny = hh + (float)border;
    maxx = 222.0f - (float)border - hw - 1.0f; // default; updated in resized
    maxy = 222.0f - (float)border - hh - 1.0f;
}

ShaperViewComponent::~ShaperViewComponent()
{
}

void ShaperViewComponent::convertToScreen(SplinePoint p, float& sx, float& sy)
{
    sx = p.x * (maxx - minx) + minx;
    sy = p.y * (maxy - miny) + miny;
}

void ShaperViewComponent::convertFromScreen(float sx, float sy, SplinePoint& p)
{
    p.x = (sx - minx) / (maxx - minx);
    p.y = (sy - miny) / (maxy - miny);
}

void ShaperViewComponent::paint(juce::Graphics& g)
{
    // Update coordinate system based on actual bounds
    float border = 2.0f;
    float hw = handleImg.getWidth() * 0.5f + 1.0f;
    float hh = handleImg.getHeight() * 0.5f + 1.0f;
    minx = hw + border;
    miny = hh + border;
    maxx = (float)getWidth() - border - hw - 1.0f;
    maxy = (float)getHeight() - border - hh - 1.0f;

    // Draw background region from main bg image
    if (!bgImage.isNull())
    {
        // The background image is the full plugin background; draw the region for this component
        g.drawImage(bgImage, 0, 0, getWidth(), getHeight(),
                    80, 45, getWidth(), getHeight());
    }

    long n = shaper.GetNPoints();

    // Draw guides (white lines connecting control points)
    if (guides && n > 0)
    {
        g.setColour(juce::Colours::white);
        float px, py;
        convertToScreen(shaper.getPoint(0), px, py);
        juce::Path guidePath;
        guidePath.startNewSubPath(px, py);
        for (long i = 1; i < n; i++)
        {
            convertToScreen(shaper.getPoint(i), px, py);
            guidePath.lineTo(px, py);
        }
        g.strokePath(guidePath, juce::PathStrokeType(1.0f));
    }

    // Draw spline curve (black)
    g.setColour(juce::Colours::black);
    if (n >= 4) // degree
    {
        // B-spline
        std::array<SplinePoint, maxn> pts {};
        for (long i = 0; i < n; i++)
            pts[i] = shaper.getPoint(i);
        bspline(n - 1, 4, pts.data(), lines.data(), N_LINES);

        juce::Path curvePath;
        float sx, sy;
        convertToScreen(lines[0], sx, sy);
        curvePath.startNewSubPath(sx, sy);
        for (long i = 1; i < N_LINES; i++)
        {
            convertToScreen(lines[i], sx, sy);
            curvePath.lineTo(sx, sy);
        }
        g.strokePath(curvePath, juce::PathStrokeType(1.0f));
    }
    else if (n >= 2)
    {
        // Bezier
        std::array<SplinePoint, maxn> pts {};
        for (long i = 0; i < n; i++)
            pts[i] = shaper.getPoint(i);

        juce::Path curvePath;
        SplinePoint p0 = RecursiveSpline(pts.data(), 0, n - 1, 0.0f);
        float sx, sy;
        convertToScreen(p0, sx, sy);
        curvePath.startNewSubPath(sx, sy);

        float dt = 1.0f / (float)N_LINES;
        for (float t = dt; t <= 1.0f; t += dt)
        {
            SplinePoint pt = RecursiveSpline(pts.data(), 0, n - 1, t);
            convertToScreen(pt, sx, sy);
            curvePath.lineTo(sx, sy);
        }
        SplinePoint pEnd = RecursiveSpline(pts.data(), 0, n - 1, 1.0f);
        convertToScreen(pEnd, sx, sy);
        curvePath.lineTo(sx, sy);

        g.strokePath(curvePath, juce::PathStrokeType(1.0f));
    }

    // Draw control point handles
    float dx = handleImg.getWidth() * 0.5f;
    float dy = handleImg.getHeight() * 0.5f;
    for (long i = 0; i < n; i++)
    {
        float hx, hy;
        convertToScreen(shaper.getPoint(i), hx, hy);
        g.drawImage(handleImg,
                    (int)(hx - dx), (int)(hy - dy),
                    handleImg.getWidth(), handleImg.getHeight(),
                    0, 0, handleImg.getWidth(), handleImg.getHeight());
    }
}

void ShaperViewComponent::mouseDown(const juce::MouseEvent& event)
{
    SplinePoint p;
    convertFromScreen((float)event.x, (float)event.y, p);

    // Right-click or ctrl+click = remove point
    if (event.mods.isRightButtonDown() || (event.mods.isCtrlDown() && event.mods.isLeftButtonDown()))
    {
        selectedIndex = -1;
        float dist;
        long n = shaper.GetNPoints();
        for (long i = 1; i < n - 1; i++)
        {
            float dx = p.x - shaper.getPoint(i).x;
            float dy = p.y - shaper.getPoint(i).y;
            dist = dx * dx + dy * dy;
            if (dist < DISTANCE)
            {
                selectedIndex = i;
                break;
            }
        }
        if (selectedIndex != -1 && n > 2)
        {
            shaper.removePoint((int)selectedIndex);
            selectedIndex = -1;
            shaper.updatedata();
            repaint();
            if (onChanged) onChanged();
            if (onSelectionChanged) onSelectionChanged();
        }
        return;
    }

    // Left-click: select existing or insert new point
    if (event.mods.isLeftButtonDown())
    {
        selectedIndex = -1;
        long n = shaper.GetNPoints();
        for (long i = 0; i < n; i++)
        {
            float dx = p.x - shaper.getPoint(i).x;
            float dy = p.y - shaper.getPoint(i).y;
            float dist = dx * dx + dy * dy;
            if (dist < DISTANCE)
            {
                selectedIndex = i;
                if (onSelectionChanged) onSelectionChanged();
                return;
            }
        }

        // Insert new point if within bounds
        if (p.x >= 0.0f && p.x <= 1.0f && p.y >= 0.0f && p.y <= 1.0f && n < maxn)
        {
            shaper.insertPoint(p.x, p.y);
            // Find the inserted point to select it
            n = shaper.GetNPoints();
            for (long i = 0; i < n; i++)
            {
                if (fabsf(shaper.getPoint(i).x - p.x) < 0.001f &&
                    fabsf(shaper.getPoint(i).y - p.y) < 0.001f)
                {
                    selectedIndex = i;
                    break;
                }
            }
            shaper.updatedata();
            repaint();
            if (onChanged) onChanged();
            if (onSelectionChanged) onSelectionChanged();
        }
    }
}

void ShaperViewComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (selectedIndex < 0) return;

    SplinePoint p;
    convertFromScreen((float)event.x, (float)event.y, p);

    long n = shaper.GetNPoints();
    float x = p.x;
    float y = juce::jlimit(0.0f, 1.0f, p.y);

    if (selectedIndex == 0 || selectedIndex == n - 1)
    {
        // Endpoints: y only
        shaper.getPoint((int)selectedIndex).y = y;
    }
    else
    {
        // Interior points: constrain x between neighbors
        float leftX = shaper.getPoint((int)selectedIndex - 1).x + 0.00001f;
        float rightX = shaper.getPoint((int)selectedIndex + 1).x - 0.00001f;
        x = juce::jlimit(leftX, rightX, x);
        shaper.getPoint((int)selectedIndex).x = x;
        shaper.getPoint((int)selectedIndex).y = y;
    }

    shaper.updatedata();
    repaint();
    if (onChanged) onChanged();
    if (onSelectionChanged) onSelectionChanged();
}

void ShaperViewComponent::mouseUp(const juce::MouseEvent&)
{
    selectedIndex = -1;
    if (onSelectionChanged) onSelectionChanged();
}

void ShaperViewComponent::setGuides(bool g)
{
    if (guides != g)
    {
        guides = g;
        repaint();
    }
}

void ShaperViewComponent::reset()
{
    shaper.reset();
    shaper.updatedata();
    selectedIndex = -1;
    repaint();
    if (onChanged) onChanged();
}

bool ShaperViewComponent::getSelectedXY(float& x, float& y) const
{
    if (selectedIndex >= 0 && selectedIndex < shaper.GetNPoints())
    {
        x = shaper.getPoint((int)selectedIndex).x;
        y = 1.0f - shaper.getPoint((int)selectedIndex).y;
        return true;
    }
    return false;
}
