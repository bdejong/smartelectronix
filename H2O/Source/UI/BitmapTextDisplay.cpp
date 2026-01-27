#include "BitmapTextDisplay.h"

// Character code for infinity symbol (from original asciitable.h: __INF = 250)
static constexpr char INF_CHAR = (char)250;

BitmapTextDisplay::BitmapTextDisplay(const juce::Image& image)
    : fontImage(image)
{
}

BitmapTextDisplay::~BitmapTextDisplay()
{
}

int BitmapTextDisplay::getCharIndex(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return 26 + (c - 'a');
    if (c >= '0' && c <= '9') return 52 + (c - '0');

    switch (c)
    {
        case '.': return 62;
        case ',': return 63;
        case ';': return 64;
        case ':': return 65;
        case INF_CHAR: return 66;  // Infinity symbol
        case '/': return 67;
        case '-': return 68;
        case '+': return 69;
        case '=': return 70;
        case '?': return 71;
        case ' ': return 72;
        default: return -1;
    }
}

int BitmapTextDisplay::getCharWidth(int index)
{
    if (index < 0 || index >= 73)
        return 0;
    return charWidths[index];
}

int BitmapTextDisplay::getCharPosition(int index)
{
    if (index < 0 || index >= 73)
        return 0;

    int pos = 0;
    for (int i = 0; i < index; ++i)
        pos += charWidths[i];
    return pos;
}

void BitmapTextDisplay::paint(juce::Graphics& g)
{
    if (fontImage.isNull() || text.isEmpty())
        return;

    int xPos = 0;
    int fontHeight = fontImage.getHeight();

    for (int i = 0; i < text.length(); ++i)
    {
        char c = static_cast<char>(text[i]);
        int charIndex = getCharIndex(c);

        if (charIndex >= 0)
        {
            int charX = getCharPosition(charIndex);
            int charWidth = getCharWidth(charIndex);

            // Draw the character from the font strip
            g.drawImage(fontImage,
                        xPos, 0, charWidth, getHeight(),
                        charX, 0, charWidth, fontHeight);

            xPos += charWidth;
        }
    }
}

void BitmapTextDisplay::setText(const juce::String& newText)
{
    if (text != newText)
    {
        text = newText;
        repaint();
    }
}
