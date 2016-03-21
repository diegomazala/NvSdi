#include "common.h"

//
// Draw YCrCb Colorbar Pattern
//
GLvoid DrawYCrCbBars(GLfloat _width, GLfloat _height) 
{
	int lBarWidth = _width / 8;
	int lBarHeight = _height;

	int startx = 0;
	int stopx = lBarWidth;

	// FORMAT TO OPENGL IS SPECIFIED CrYCb

	// Black
	glColor4f(0x200/1024.0f, 0x040/1024.0f, 0x200/1024.0f, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Blue
	glColor4f(0x1D7/1024.0f, 0x07f/1024.0f, 0x3c0/1024.0f, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Red
	glColor4f(0x3c0/1024.0f, 0x0fa/1024.0f, 0x199/1024.0f, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Magenta
	glColor4f(0x397/1024.0f, 0x13a/1024.0f, 0x359/1024.0f, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Green
	glColor4f(0x069/1024.0f, 0x2b2/1024.0f , 0x0a7/1024.0f, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Cyan
	glColor4f(0x040/1024.0f, 0x2f2/1024.0f, 0x267/1024.0f, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;
 
	// Yellow
	glColor4f(0x229/1024.0f, 0x36d/1024.0f, 0x040/1024.0f, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);	

	startx = stopx;
	stopx += lBarWidth;
		
	// White
	glColor4f(0x200/1024.0f, 0x3ac/1024.0f, 0x200/1024.0f, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);	
}

//
// Draw 100% RGB Colorbar Pattern
//
GLvoid DrawRGBBars100(GLfloat _width, GLfloat _height) 
{
	int lBarWidth = _width / 8;
	int lBarHeight = _height;

	int startx = 0;
	int stopx = lBarWidth;

	// Black
	glColor4f(0.0, 0.0, 0.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Blue	
	glColor4f(0.0, 0.0, 1.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);


	startx = stopx;
	stopx += lBarWidth;

	// Red
	glColor4f(1.0, 0.0, 0.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Magenta
	glColor4f(1.0, 0.0, 1.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Green
	glColor4f(0.0, 1.0, 0.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Cyan
	glColor4f(0.0, 1.0, 1.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;
 		
	// Yellow
	glColor4f(1.0, 1.0, 0.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// White
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);	
}

//
// Draw 75% RGB Colorbar Pattern
//
GLvoid DrawRGBBars75(GLfloat _width, GLfloat _height) 
{
	int lBarWidth = _width / 8;
	int lBarHeight = _height;

	int startx = 0;
	int stopx = lBarWidth;

	// Black
	glColor4f(0.0, 0.0, 0.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;
		
	// Blue
	glColor4f(0.0, 0.0, 1.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Red
	glColor4f(0.75, 0.0, 0.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Magenta
	glColor4f(0.75, 0.0, 0.75, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Green
	glColor4f(0.0, 0.75, 0.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Cyan
	glColor4f(0.0, 0.75, 0.75, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;
 		
	// Yellow
	glColor4f(0.75, 0.75, 0.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;
		
	// White
	glColor4f(0.75, 0.75, 0.75, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);
}

//
// Draw SMPTE color bar pattern
//
GLvoid DrawSMPTEBars(GLfloat _width, GLfloat _height, int _bitDepth, bool _100PercentWhite, bool _b16by9) 
{  
	int x, y, w, h;

	// SMPTE RP219-2002 Pattern 1.

	x = 0;
	y = _height;
	h = (int)((7.0/12.0) * (float)_height) + 1;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 240 : 0;

	// FORMAT TO OPENGL IS SPECIFIED CrYCb

	// 40% Gray
	if (_bitDepth == 8 ) {
		glColor3ub(128, 104, 128);
	} else {
		glColor3f(0.5, 0.404296875, 0.5);	
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 205 : 102;

	// 75%/100% White
	if (_100PercentWhite) {
		if (_bitDepth == 8) {
			glColor3ub(128, 235, 128);
		} else {
			glColor3f(1.0, 0.91796875, 1.0);
		}
	} else {
		if (_bitDepth == 8) {
			glColor3ub(128, 180, 128);
		} else {
			glColor3f(1.0, 0.704101562, 1.0);	
		}
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 206 : 103;

	// Yellow
	if (_bitDepth == 8) {
		glColor3ub(136, 168, 44);
	} else {
		glColor3f(0.658203125, 0.530273437, 0.171875);
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 206 : 103;

	// Cyan
	if (_bitDepth == 8) {
		glColor3ub(44, 145, 147);
	} else {
		glColor3f( 0.171875, 0.567382812, 0.575195312);
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 206 : 103;

	// Green
	if (_bitDepth == 8) {
		glColor3ub(52, 133, 63);
	} else {
		glColor3f(0.202148437, 0.521484375, 0.247070312);
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 206 : 103;

	// Magenta
	if (_bitDepth == 8) {
		glColor3ub(204, 63, 193);
	} else {
		glColor3f(0.797851562, 0.245117187, 0.752929687);
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 206 : 103;

	// Red
	if (_bitDepth == 8) {
		glColor3ub(212, 51, 109);
	} else {
		glColor3f(0.828125, 0.19921875, 0.424804687);
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 205 : 102;

	// Blue
	if (_bitDepth == 8) {
		glColor3ub(120, 28, 212);
	} else {
		glColor3f(0.469726562, 0.108398437, 0.828125);
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 240 : 0;

	// 40% Gray
	if (_bitDepth == 8) {
		glColor3ub(128, 104, 128);
	} else {
		glColor3f(0.5, 0.404296875, 0.5);
	}
	glRecti(x, y, x+w, y-h);

	// SMPTE RP 219-2002 Pattern 2
		
	x = 0;
	y -= h;
	h = (int)((1.0/12.0) * (float)_height) + 1;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 240 : 0;		

	// Cyan
	if (_bitDepth == 8) {
		glColor3ub(16, 188, 154);
	} else {
		glColor3f(0.0625, 0.736328125, 0.600585937);	
	}
	glRecti(x, y, x+w, y-h);

	// *2
	// 75%/100% white
	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 205 : 102;

	if (_100PercentWhite) {
		if (_bitDepth == 8) {
			glColor3ub(128, 235, 128);
		} else {
			glColor3f(0.5, 0.91796875, 0.5);		
		}
	} else {
		if (_bitDepth == 8) {
			glColor3ub(128, 180, 128);
		} else {
			glColor3f(0.5, 0.704101562, 0.5);	
		}
	}	
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 1235 : 617;

	// 75%/100% White
	if (_100PercentWhite) {
		if (_bitDepth == 8) {
			glColor3ub(128, 235, 128);
		} else {
			glColor3f(0.5, 0.91796875, 0.5);		
		}
	} else {
		if (_bitDepth == 8) {
			glColor3ub(128, 180, 128);
		} else {
			glColor3f(0.5, 0.704101562, 0.5);	
		}
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 240 : 0;

	// 100% Blue
	if (_bitDepth == 8) {
		glColor3ub(118, 32, 240);
	} else {
		glColor3f(0.459960937, 0.124023437, 0.9375);
	}
	glRecti(x, y, x+w, y-h);

	// SMPTE RP 219-2002 Pattern 3
	// (black signal selected)

	x = 0;
	y -= h;
	h = (int)((1.0/12.0) * (float)_height) + 1;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 240 : 0;		

	// 100% Yellow
	if (_bitDepth == 8) {
		glColor3ub(138, 219, 16);
	} else {
		glColor3f(0.540039062, 0.856445312, 0.0625);	
	}
	glRecti(x, y, x+w, y-h);

	// *3
	// Set to black.
	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 205 : 102;

	if (_bitDepth == 8) {
		glColor3ub(128, 16, 128);
	} else {
		glColor3i(0.5, 0.0625, 0.5);
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 1235 : 617; 

	// Y Ramp	
	glBegin(GL_QUADS);
	if (_bitDepth == 8) {
		glColor3ub(128, 16, 128);
	} else {
		glColor3f(0.5, 0.0625, 0.5);
	}
	glVertex2d(x, y-h);
	glVertex2d(x, y);
	if (_bitDepth == 8) {
		glColor3ub(128, 235, 128);
	} else {
		glColor3f(0.5, 0.91796875, 0.5);
	}
	glVertex2d(x+w, y);
	glVertex2d(x+w, y-h);
	glEnd();

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 240 : 0;

	// 100% Red
	if (_bitDepth == 8) {
		glColor3ub(240, 63, 102);
	} else {
		glColor3f(0.9375, 0.244140625, 0.399414062);
	}
	glRecti(x, y, x+w, y-h);

	// SMPTE RP 219-2002 Pattern 4

	x = 0;
	y -= h;
	h = (int)((3.0/12.0) * (float)_height) + 1;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 240 : 0;

	// 15% Gray
	if (_bitDepth == 8) {
		glColor3ub(128, 49, 128);
	} else {
		glColor3f(0.5, 0.190429687, 0.5);	
	}
	glRecti(x, y, x+w, y-h);  // SHOULD BE 270 HIGH

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 309 : 154;

	// 0% Black
	if (_bitDepth == 8) {
		glColor3ub(128, 16, 128);
	} else {
		glColor3f(0.5, 0.0625, 0.5); 	
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 411 : 205;

	// 100% White
	if (_bitDepth == 8) {
		glColor3ub(128, 235, 128);
	} else {
		glColor3f(0.5, 0.91796875, 0.5);
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 171 : 85;

	// 0% Black
	if (_bitDepth == 8) {
		glColor3ub(128, 16, 128);
	} else {
		glColor3f(0.5, 0.0625, 0.5); 	
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 69 : 34;

	// -2%
	if (_bitDepth == 8) {
		glColor3ub(128, 12, 128);
	} else {
		glColor3f(0.5, 0.044921875, 0.5);
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 68 : 34;

	// 0%
	if (_bitDepth == 8) {
		glColor3ub(128, 16, 128);
	} else {
		glColor3f(0.5, 0.0625, 0.5); 	
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 69 : 34;

	// +2%
	if (_bitDepth == 8) {
		glColor3ub(128, 20, 128);
	} else {	
		glColor3f(0.5, 0.080078125, 0.5);
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 68 : 34;

	// 0%
	if (_bitDepth == 8) {
		glColor3ub(128, 16, 128);
	} else {
		glColor3f(0.5, 0.0625, 0.5); 	
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 69 : 34;

	// 4%
	if (_bitDepth == 8) {
		glColor3ub(128, 25, 128);
	} else {
		glColor3f(0.5, 0.096679687, 0.5);
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 206 : 103;

	// 0% Black
	if (_bitDepth == 8) {
		glColor3ub(128, 16, 128);
	} else {
		glColor3f(0.5, 0.0625, 0.5); 	
	}
	glRecti(x, y, x+w, y-h);

	x += w;
	w = (int) _b16by9 ? ((float)_width / 1920.0) * 240 : 0;

	// 15% Gray
	if (_bitDepth == 8) {
		glColor3ub(128, 49, 128);
	} else {
		glColor3f(0.5, 0.190429687, 0.5);	
	}
	glRecti(x, y, x+w, y-h);
} 
