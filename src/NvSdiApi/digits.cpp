#include "digits.h"

//
// Draws segment 1 of 7 segment digit
//
void
drawSegment1(unsigned int _x, unsigned int _y)
{
	glBegin(GL_TRIANGLE_STRIP);	
	glVertex2i(_x + 1, _y +  9);
	glVertex2i(_x + 2, _y + 10);
	glVertex2i(_x + 2, _y +  8);
	glVertex2i(_x + 4, _y + 10);
	glVertex2i(_x + 4, _y +  8);
	glVertex2i(_x + 5, _y +  9);
	glEnd();
}

//
// Draws segment 2 of 7 segment digit
//
void
drawSegment2(unsigned int _x, unsigned int _y)
{
	glBegin(GL_TRIANGLE_STRIP);
	glVertex2i(_x + 1, _y +  5);
	glVertex2i(_x + 0, _y +  6);
	glVertex2i(_x + 2, _y +  6);
	glVertex2i(_x + 0, _y +  8);
	glVertex2i(_x + 2, _y +  8);
	glVertex2i(_x + 1, _y +  9);
	glEnd();
}

//
// Draws segment 3 of 7 segment digit
//
void
drawSegment3(unsigned int _x, unsigned int _y)
{
	glBegin(GL_TRIANGLE_STRIP);
	glVertex2i(_x + 5, _y +  5);
	glVertex2i(_x + 4, _y +  6);
	glVertex2i(_x + 6, _y +  6);
	glVertex2i(_x + 4, _y +  8);
	glVertex2i(_x + 6, _y +  8);
	glVertex2i(_x + 5, _y +  9);
	glEnd();
}

//
// Draws segment 4 of 7 segment digit
//
void
drawSegment4(unsigned int _x, unsigned int _y)
{
	glBegin(GL_TRIANGLE_STRIP);	
	glVertex2i(_x + 1, _y +  5);
	glVertex2i(_x + 2, _y +  6);
	glVertex2i(_x + 2, _y +  4);
	glVertex2i(_x + 4, _y +  6);
	glVertex2i(_x + 4, _y +  4);
	glVertex2i(_x + 5, _y +  5);
	glEnd();
}

//
// Draws segment 5 of 7 segment digit
//
void
drawSegment5(unsigned int _x, unsigned int _y)
{
	glBegin(GL_TRIANGLE_STRIP);
	glVertex2i(_x + 1, _y +  1);
	glVertex2i(_x + 0, _y +  2);
	glVertex2i(_x + 2, _y +  2);
	glVertex2i(_x + 0, _y +  4);
	glVertex2i(_x + 2, _y +  4);
	glVertex2i(_x + 1, _y +  5);
	glEnd();
}

//
// Draws segment 6 of 7 segment digit
//
void
drawSegment6(unsigned int _x, unsigned int _y)
{
	glBegin(GL_TRIANGLE_STRIP);
	glVertex2i(_x + 5, _y +  1);
	glVertex2i(_x + 4, _y +  2);
	glVertex2i(_x + 6, _y +  2);
	glVertex2i(_x + 4, _y +  4);
	glVertex2i(_x + 6, _y +  4);
	glVertex2i(_x + 5, _y +  5);
	glEnd();
}

//
// Draws segment 7 of 7 segment digit
//
void
drawSegment7(unsigned int _x, unsigned int _y)
{
	glBegin(GL_TRIANGLE_STRIP);	
	glVertex2i(_x + 1, _y +  1);
	glVertex2i(_x + 2, _y +  2);
	glVertex2i(_x + 2, _y +  0);
	glVertex2i(_x + 4, _y +  2);
	glVertex2i(_x + 4, _y +  0);
	glVertex2i(_x + 5, _y +  1);
	glEnd();
}

//
// Draw specified 7 segment digit
//
void
drawDigit(unsigned int _n, unsigned int _x, unsigned int _y)
{
	switch(_n) {
		case 0:
			drawSegment1(_x, _y);
			drawSegment2(_x, _y);
			drawSegment3(_x, _y);
			drawSegment5(_x, _y);
			drawSegment6(_x, _y);
			drawSegment7(_x, _y);
			break;

		case 1:
			drawSegment3(_x, _y);
			drawSegment6(_x, _y);
			break;

		case 2:
			drawSegment1(_x, _y);
			drawSegment3(_x, _y);
			drawSegment4(_x, _y);
			drawSegment5(_x, _y);
			drawSegment7(_x, _y);
			break;

		case 3:
			drawSegment1(_x, _y);
			drawSegment3(_x, _y);
			drawSegment4(_x, _y);
			drawSegment6(_x, _y);
			drawSegment7(_x, _y);
			break;

		case 4:
			drawSegment2(_x, _y);
			drawSegment3(_x, _y);
			drawSegment4(_x, _y);
			drawSegment6(_x, _y);
			break;

		case 5:
			drawSegment1(_x, _y);
			drawSegment2(_x, _y);
			drawSegment4(_x, _y);
			drawSegment6(_x, _y);
			drawSegment7(_x, _y);
			break;

		case 6:
			drawSegment2(_x, _y);
			drawSegment4(_x, _y);
			drawSegment5(_x, _y);
			drawSegment6(_x, _y);
			drawSegment7(_x, _y);
			break;

		case 7:
			drawSegment1(_x, _y);
			drawSegment3(_x, _y);
			drawSegment6(_x, _y);
			break;

		case 8:
			drawSegment1(_x, _y);
			drawSegment2(_x, _y);
			drawSegment3(_x, _y);
			drawSegment4(_x, _y);
			drawSegment5(_x, _y);
			drawSegment6(_x, _y);
			drawSegment7(_x, _y);
			break;

		case 9:
			drawSegment1(_x, _y);
			drawSegment2(_x, _y);
			drawSegment3(_x, _y);
			drawSegment4(_x, _y);
			drawSegment6(_x, _y);
			break;
	}
}

//
// Draw 2 digital number
void
draw2Digits(unsigned int _n, unsigned int _x, unsigned int _y)
{
	drawDigit(_n / 10, _x, _y);
	drawDigit(_n % 10, _x + 8, _y);
}

//
// Draw semicolon
//
void
drawSemicolon(unsigned int _x, unsigned int _y)
{
	glBegin(GL_QUADS);
	glVertex2i(_x + 1, _y + 2);
	glVertex2i(_x + 1, _y + 3);
	glVertex2i(_x + 2, _y + 3);
	glVertex2i(_x + 2, _y + 2);
	glVertex2i(_x + 1, _y + 6);
	glVertex2i(_x + 1, _y + 7);
	glVertex2i(_x + 2, _y + 7);
	glVertex2i(_x + 2, _y + 6);
	glEnd();
}
