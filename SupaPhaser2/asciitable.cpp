#include "asciitable.h"

int ASCII[256];
int DASCII[256]; //dTable

void fillasciitable()
{
	for(int i=0;i<256;i++)
		ASCII[i] = DASCII[i] = 0;

	//DASCII = width
	DASCII[getascii('A')] = 6;
	DASCII[getascii('B')] = 6;
	DASCII[getascii('C')] = 6;
	DASCII[getascii('D')] = 6;
	DASCII[getascii('E')] = 5;
	DASCII[getascii('F')] = 5;
	DASCII[getascii('G')] = 6;
	DASCII[getascii('H')] = 6;
	DASCII[getascii('I')] = 2;
	DASCII[getascii('J')] = 6;
	DASCII[getascii('K')] = 6;
	DASCII[getascii('L')] = 5;
	DASCII[getascii('M')] = 6;
	DASCII[getascii('N')] = 6;
	DASCII[getascii('O')] = 7; //7
	DASCII[getascii('P')] = 6;
	DASCII[getascii('Q')] = 6;
	DASCII[getascii('R')] = 6;
	DASCII[getascii('S')] = 6;
	DASCII[getascii('T')] = 6;
	DASCII[getascii('U')] = 6;
	DASCII[getascii('V')] = 6;
	DASCII[getascii('W')] = 8;
	DASCII[getascii('X')] = 6;
	DASCII[getascii('Y')] = 6;
	DASCII[getascii('Z')] = 6;
	DASCII[getascii('?')] = 5;
	DASCII[getascii('!')] = 2;
	DASCII[getascii('.')] = 2;
	DASCII[getascii(',')] = 3;
	DASCII[getascii('/')] = 4;
	DASCII[getascii(':')] = 2;
	DASCII[getascii('1')] = 3;
	DASCII[getascii('2')] = 6;
	DASCII[getascii('3')] = 6;
	DASCII[getascii('4')] = 6;
	DASCII[getascii('5')] = 6;
	DASCII[getascii('6')] = 6;
	DASCII[getascii('7')] = 5;
	DASCII[getascii('8')] = 6;
	DASCII[getascii('9')] = 6;
	DASCII[getascii('0')] = 6;
	DASCII[getascii('#')] = 6;
	DASCII[getascii('%')] = 6;
	DASCII[getascii('&')] = 6;
	DASCII[getascii('(')] = 3;
	DASCII[getascii(')')] = 3;
	DASCII[getascii('*')] = 6;
	DASCII[getascii('=')] = 5;
	DASCII[getascii('+')] = 6;
	DASCII[getascii('-')] = 5;
	DASCII[getascii('[')] = 3;
	DASCII[getascii(']')] = 3;
	DASCII[getascii('@')] = 6;
	//ASCII = place

	ASCII[0] = 0;
	for(int i=1;i<256;i++)
		ASCII[i] = ASCII[i-1] + DASCII[i-1];
}
