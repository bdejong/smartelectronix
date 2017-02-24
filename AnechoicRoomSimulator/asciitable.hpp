#pragma once

#define __INF 250

void fillasciitable();

extern int ASCII[256];
extern int DASCII[256]; //dTable

inline int getascii(unsigned char c)
{
    switch (c)
    {
    case 'A': return 0;
    case 'B': return 1;
    case 'C': return 2;
    case 'D': return 3;
    case 'E': return 4;
    case 'F': return 5;
    case 'G': return 6;
    case 'H': return 7;
    case 'I': return 8;
    case 'J': return 9;
    case 'K': return 10;
    case 'L': return 11;
    case 'M': return 12;
    case 'N': return 13;
    case 'O': return 14;
    case 'P': return 15;
    case 'Q': return 16;
    case 'R': return 17;
    case 'S': return 18;
    case 'T': return 19;
    case 'U': return 20;
    case 'V': return 21;
    case 'W': return 22;
    case 'X': return 23;
    case 'Y': return 24;
    case 'Z': return 25;
    case '?': return 26;
    case '!': return 27;
    case '.': return 28;
    case ',': return 29;
    case '/': return 30;
    case ':': return 31;
    case '1': return 32;
    case '2': return 33;
    case '3': return 34;
    case '4': return 35;
    case '5': return 36;
    case '6': return 37;
    case '7': return 38;
    case '8': return 39;
    case '9': return 40;
    case '0': return 41;
    case '#': return 42;
    case '%': return 43;
    case '&': return 44;
    case '(': return 45;
    case ')': return 46;
    case '*': return 47;
    case '=': return 48;
    case '+': return 49;
    case '-': return 50;
    case '[': return 51;
    case ']': return 52;
    case '@': return 53;
    }
    return -1;
};

inline void PlaceAndWidth(unsigned char c, int& place, int& width)
{
    int x = getascii(c);

    if (x != -1)
    {
        place = ASCII[x];
        width = DASCII[x];
    }
    else
    {
        place = -1;
        width = -1;
    }
}
