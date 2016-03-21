#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

extern void drawDigit(unsigned int _n, unsigned int _x, unsigned int _y);
extern void draw2Digits(unsigned int _n, unsigned int _x, unsigned int _y);
void drawSemicolon(unsigned int _x, unsigned int _y);
