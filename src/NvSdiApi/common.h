#ifndef _COMMON_H_
#define _COMMON_H_

//
// common.h
//
// Header file for common NVGVOSDK functions. 
//

#include <windows.h> 
#include <tchar.h>
#include <assert.h>

#include <ctime>

#include <list>
#include <iostream>

#include<sstream>

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <SYS\stat.h>

#include <GL/gl.h> 
#include <GL/glu.h> 
#include <GL/glext.h>
#include <GL/wglext.h>
//#include <gl/glaux.h>

#include "nvapi.h"

#include "NvGvoApi.h"
#include "NvApiError.h"

#include "pbuffer.h"
#include "fbo.h"

//
// Common enums
//

//
// Common struction definitions
//

//
// Function definitions
//

// Timer.cpp
__int64 GetNanoClock();
void  PrintFPS(void);
float CalcFPS(void);

// Console.cpp
void SetupConsole();
void SetupLog();
void CloseLog();

HWND MyCreateWindow(HINSTANCE hInstance, int x, int y, int width, int height, bool fullscreen = false);
LONG WINAPI MainWndProc (HWND, UINT, WPARAM, LPARAM);
GLvoid DrawYCrCbBars(GLfloat _width, GLfloat _height);
GLvoid DrawRGBBars100(GLfloat _width, GLfloat _height);
GLvoid DrawRGBBars75(GLfloat _width, GLfloat _height);
GLvoid DrawSMPTEBars(GLfloat _width, GLfloat _height, int _bitDepth, bool _100PercentWhite, bool _b16by9);

//
// Macro definitions
//
#define NVGVOCHECK(in) \
	{ NVRESULT nvResult = in; \
	if ( nvResult != NV_OK ) { \
		if ( nvResult == NV_ACCESSDENIED ) { \
		MessageBox(NULL, "NvGvo:Unable to access video device.", "Error", MB_OK); \
			return E_FAIL; \
		} else if (nvResult == NV_RUNNING) { \
			NvGvoStop(g_hGVO); \
		} else if (nvResult == NV_BADPARAMETER) { \
			MessageBox(NULL, "NvGvo: bad parameter.", "Error", MB_OK); \
			return E_FAIL; \
		} else if (nvResult == NV_INTERNALERROR) { \
			MessageBox(NULL, "NvGvo: internal error.", "Error", MB_OK); \
			return E_FAIL; \
		} else { \
			MessageBox(NULL, "NvGvo: unspecified error.", "Error", MB_OK); \
			return E_FAIL; \
		} \
	} }

#endif