
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



class SdiInWindow : public GLWindow
{
public:

	SdiInWindow();
	~SdiInWindow();

	void DisplayVideo(int vid_w, int vid_h);

	void CalcWindowSize(int vid_w, int vid_h, int stream_count, NVVIOSIGNALFORMAT signal_format, int& win_w, int& win_h);


	void Render();
	void OnKeyEvent(const KeyEvent* pEvent);


	int DroppedFrames;

protected:

	bool InitGL();
	
	
};


//
// Function prototypes
//

// From Console.cpp
void SetupConsole();

#endif _SDI_IN_WINDOW_H_