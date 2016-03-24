
#ifndef _SDI_IN_WINDOW_H_
#define _SDI_IN_WINDOW_H_

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <assert.h>
#include <stdio.h>

#include "OpenGL.h"

#include "nvSDIin.h"


#include "timecode.h"

#include "ANCapi.h"


#include "WinAppGL/GLWindow.h"



class SdiInScreenshot : public GLWindow
{
public:

	SdiInScreenshot();
	~SdiInScreenshot();

	void DisplayVideo();


	void Render();
	void OnKeyEvent(const KeyEvent* pEvent);


protected:

	bool InitGL();
	

	int mVideoIndex;
};


//
// Function prototypes
//

// From Console.cpp
void SetupConsole();

#endif _SDI_IN_WINDOW_H_