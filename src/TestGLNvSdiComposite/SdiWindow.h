
#ifndef _SDI_WINDOW_H_
#define _SDI_WINDOW_H_

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <assert.h>
#include <stdio.h>

#include "OpenGL.h"

#include "nvSDIin.h"
#include "nvSDIout.h"

#include "GLFbo.h"

#include "timecode.h"

#include "ANCapi.h"


#include "WinAppGL/GLWindow.h"



class SdiWindow : public GLWindow
{
public:

	SdiWindow();
	~SdiWindow();

	void RenderToSdi(int vid_w, int vid_h);
	void DisplayVideo(int vid_w, int vid_h);

	void CalcWindowSize(int vid_w, int vid_h, int stream_count, NVVIOSIGNALFORMAT signal_format, int& win_w, int& win_h);


	void Render();
	void OnKeyEvent(const KeyEvent* pEvent);


	bool InitFbo();

	int DroppedFrames;
protected:

	bool InitGL();

	GLFbo	fbo;

	bool m_ProccessingEnabled;


	gl::Texture2D	mOutputTex;
	int	m_CurrentInputIndex;
};


//
// Function prototypes
//

// From Console.cpp
void SetupConsole();

#endif _SDI_WINDOW_H_
