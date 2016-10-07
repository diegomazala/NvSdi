
#ifndef _SDI_IN_ASYNC_WINDOW_H_
#define _SDI_IN_ASYNC_WINDOW_H_

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

#include "DVP.h"
#include "Texture.h"

class SdiInAsyncWindow : public GLWindow
{
public:

	SdiInAsyncWindow();
	~SdiInAsyncWindow();

	void Render();
	void OnKeyEvent(const KeyEvent* pEvent);
	

protected:

	bool InitGL();
	

	bool showStatistics;
	gl::Quad quad;
};


#endif // _SDI_IN_ASYNC_WINDOW_H_
