#include "asciitable.h"

int ASCII[256];
int DASCII[256]; //dTable

void fillasciitable()
{
	for(int i=0;i<256;i++)
		ASCII[i] = DASCII[i] = 0;

	//DASCII = width
	DASCII[getascii('A')] = 10;
	DASCII[getascii('B')] = 8;
	DASCII[getascii('C')] = 10;
	DASCII[getascii('D')] = 10;
	DASCII[getascii('E')] = 8;
	DASCII[getascii('F')] = 7;
	DASCII[getascii('G')] = 10;
	DASCII[getascii('H')] = 9;
	DASCII[getascii('I')] = 4;
	DASCII[getascii('J')] = 7;
	DASCII[getascii('K')] = 9;
	DASCII[getascii('L')] = 8;
	DASCII[getascii('M')] = 11;
	DASCII[getascii('N')] = 9;
	DASCII[getascii('O')] = 11;
	DASCII[getascii('P')] = 8;
	DASCII[getascii('Q')] = 12;
	DASCII[getascii('R')] = 9;
	DASCII[getascii('S')] = 8;
	DASCII[getascii('T')] = 10;
	DASCII[getascii('U')] = 9;
	DASCII[getascii('V')] = 10;
	DASCII[getascii('W')] = 13;
	DASCII[getascii('X')] = 9;
	DASCII[getascii('Y')] = 9;
	DASCII[getascii('Z')] = 9;

	DASCII[getascii('a')] = 9;
	DASCII[getascii('b')] = 8;
	DASCII[getascii('c')] = 8;
	DASCII[getascii('d')] = 9;
	DASCII[getascii('e')] = 8;
	DASCII[getascii('f')] = 7;
	DASCII[getascii('g')] = 9;
	DASCII[getascii('h')] = 8;
	DASCII[getascii('i')] = 4;
	DASCII[getascii('j')] = 7;
	DASCII[getascii('k')] = 8;
	DASCII[getascii('l')] = 4;
	DASCII[getascii('m')] = 12;
	DASCII[getascii('n')] = 8;
	DASCII[getascii('o')] = 9;
	DASCII[getascii('p')] = 8;
	DASCII[getascii('q')] = 9;
	DASCII[getascii('r')] = 6;
	DASCII[getascii('s')] = 7;
	DASCII[getascii('t')] = 6;
	DASCII[getascii('u')] = 8;
	DASCII[getascii('v')] = 8;
	DASCII[getascii('w')] = 12;
	DASCII[getascii('x')] = 9;
	DASCII[getascii('y')] = 8;
	DASCII[getascii('z')] = 9;

	DASCII[getascii('0')] = 9;
	DASCII[getascii('1')] = 9;
	DASCII[getascii('2')] = 9;
	DASCII[getascii('3')] = 9;
	DASCII[getascii('4')] = 9;
	DASCII[getascii('5')] = 9;
	DASCII[getascii('6')] = 9;
	DASCII[getascii('7')] = 9;
	DASCII[getascii('8')] = 9;
	DASCII[getascii('9')] = 9;

	DASCII[getascii('.')] = 5;
	DASCII[getascii(',')] = 5;
	DASCII[getascii(';')] = 6;
	DASCII[getascii(':')] = 6;
	DASCII[getascii(__INF)] = 14;
	DASCII[getascii('/')] = 8;
	DASCII[getascii('-')] = 7;
	DASCII[getascii('+')] = 10;
	DASCII[getascii('=')] = 10;
	DASCII[getascii('?')] = 8;
	DASCII[getascii(' ')] = 5;

	//ASCII = place

	ASCII[0] = 0;
	for(i=1;i<256;i++)
		ASCII[i] = ASCII[i-1] + DASCII[i-1];
}
