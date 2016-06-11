#pragma once

#define __INF 250

void fillasciitable();

extern int ASCII[256];
extern int DASCII[256]; //dTable

inline int getascii(unsigned char c)
{
    switch (c) {
    case 'A':
        return 0;
    case 'B':
        return 1;
    case 'C':
        return 2;
    case 'D':
        return 3;
    case 'E':
        return 4;
    case 'F':
        return 5;
    case 'G':
        return 6;
    case 'H':
        return 7;
    case 'I':
        return 8;
    case 'J':
        return 9;
    case 'K':
        return 10;
    case 'L':
        return 11;
    case 'M':
        return 12;
    case 'N':
        return 13;
    case 'O':
        return 14;
    case 'P':
        return 15;
    case 'Q':
        return 16;
    case 'R':
        return 17;
    case 'S':
        return 18;
    case 'T':
        return 19;
    case 'U':
        return 20;
    case 'V':
        return 21;
    case 'W':
        return 22;
    case 'X':
        return 23;
    case 'Y':
        return 24;
    case 'Z':
        return 25;

    case 'a':
        return 26;
    case 'b':
        return 27;
    case 'c':
        return 28;
    case 'd':
        return 29;
    case 'e':
        return 30;
    case 'f':
        return 31;
    case 'g':
        return 32;
    case 'h':
        return 33;
    case 'i':
        return 34;
    case 'j':
        return 35;
    case 'k':
        return 36;
    case 'l':
        return 37;
    case 'm':
        return 38;
    case 'n':
        return 39;
    case 'o':
        return 40;
    case 'p':
        return 41;
    case 'q':
        return 42;
    case 'r':
        return 43;
    case 's':
        return 44;
    case 't':
        return 45;
    case 'u':
        return 46;
    case 'v':
        return 47;
    case 'w':
        return 48;
    case 'x':
        return 49;
    case 'y':
        return 50;
    case 'z':
        return 51;

    case '0':
        return 52;
    case '1':
        return 53;
    case '2':
        return 54;
    case '3':
        return 55;
    case '4':
        return 56;
    case '5':
        return 57;
    case '6':
        return 58;
    case '7':
        return 59;
    case '8':
        return 60;
    case '9':
        return 61;

    case '.':
        return 62;
    case ',':
        return 63;
    case ';':
        return 64;
    case ':':
        return 65;
    case __INF:
        return 66;
    case '/':
        return 67;
    case '-':
        return 68;
    case '+':
        return 69;
    case '=':
        return 70;
    case '?':
        return 71;

    case ' ':
        return 72;
    }
    return -1;
};

inline void PlaceAndWidth(unsigned char c, int& place, int& width)
{
    int x = getascii(c);

    if (x != -1) {
        place = ASCII[x];
        width = DASCII[x];
    } else {
        place = -1;
        width = -1;
    }
}
