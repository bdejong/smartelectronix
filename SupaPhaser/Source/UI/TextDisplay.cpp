#include "TextDisplay.h"

static const int charWidths[54] = {
    6, 6, 6, 6, 5, 5, 6, 6, 2, 6, 6, 5, 6, 6, 7, 6, 6, 6, 6, 6,  // A-T
    6, 6, 8, 6, 6, 6,  // U-Z
    5, 2, 2, 3, 4, 2,  // ? ! . , / :
    3, 6, 6, 6, 6, 6, 5, 6, 6, 6,  // 1-9, 0
    6, 6, 6, 3, 3, 6, 5, 6, 5, 3, 3, 6  // # % & ( ) * = + - [ ] @
};

static int getCharIndex(char c)
{
    switch (c)
    {
        case 'A': return 0;  case 'B': return 1;  case 'C': return 2;
        case 'D': return 3;  case 'E': return 4;  case 'F': return 5;
        case 'G': return 6;  case 'H': return 7;  case 'I': return 8;
        case 'J': return 9;  case 'K': return 10; case 'L': return 11;
        case 'M': return 12; case 'N': return 13; case 'O': return 14;
        case 'P': return 15; case 'Q': return 16; case 'R': return 17;
        case 'S': return 18; case 'T': return 19; case 'U': return 20;
        case 'V': return 21; case 'W': return 22; case 'X': return 23;
        case 'Y': return 24; case 'Z': return 25;
        case '?': return 26; case '!': return 27; case '.': return 28;
        case ',': return 29; case '/': return 30; case ':': return 31;
        case '1': return 32; case '2': return 33; case '3': return 34;
        case '4': return 35; case '5': return 36; case '6': return 37;
        case '7': return 38; case '8': return 39; case '9': return 40;
        case '0': return 41;
        case '#': return 42; case '%': return 43; case '&': return 44;
        case '(': return 45; case ')': return 46; case '*': return 47;
        case '=': return 48; case '+': return 49; case '-': return 50;
        case '[': return 51; case ']': return 52; case '@': return 53;
        default: return -1;
    }
}

static int getCharPosition(int index)
{
    if (index < 0 || index >= 54)
        return -1;

    int pos = 0;
    for (int i = 0; i < index; ++i)
        pos += charWidths[i];
    return pos;
}

static void placeAndWidth(char c, int& place, int& width)
{
    int index = getCharIndex(c);
    if (index != -1)
    {
        place = getCharPosition(index);
        width = charWidths[index];
    }
    else
    {
        place = -1;
        width = -1;
    }
}

TextDisplay::TextDisplay(const juce::Image& fontImg, juce::Colour bgColor)
    : fontImage(fontImg), backgroundColor(bgColor)
{
}

void TextDisplay::paint(juce::Graphics& g)
{
    g.setColour(backgroundColor);
    g.fillRect(getLocalBounds());

    if (text.isEmpty() || fontImage.isNull())
        return;

    int fontHeight = fontImage.getHeight();
    int spaceSize = 3;

    int totalWidth = 0;
    for (int i = 0; i < text.length(); ++i)
    {
        char c = static_cast<char>(text[i]);
        if (c == ' ')
        {
            totalWidth += spaceSize;
        }
        else
        {
            int place, width;
            placeAndWidth(c, place, width);
            if (place != -1)
                totalWidth += width;
        }
    }

    int x;
    if (middleAligned)
        x = (getWidth() - totalWidth) / 2;
    else if (rightAligned)
        x = getWidth() - totalWidth;
    else
        x = 0;

    for (int i = 0; i < text.length(); ++i)
    {
        char c = static_cast<char>(text[i]);
        if (c == ' ')
        {
            x += spaceSize;
            continue;
        }

        int place, width;
        placeAndWidth(c, place, width);
        if (place != -1)
        {
            g.drawImage(fontImage,
                        x, 0, width, fontHeight,
                        place, 0, width, fontHeight);
            x += width;
        }
    }
}

void TextDisplay::setText(const juce::String& newText)
{
    if (text != newText)
    {
        text = newText;
        repaint();
    }
}
