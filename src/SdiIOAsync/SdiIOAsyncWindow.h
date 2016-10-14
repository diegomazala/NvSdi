
#ifndef _SDI_IO_ASYNC_WINDOW_H_
#define _SDI_IO_ASYNC_WINDOW_H_

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

class SdiIOAsyncWindow : public GLWindow
{
public:

	SdiIOAsyncWindow();
	~SdiIOAsyncWindow();

	void Render();
	void OnKeyEvent(const KeyEvent* pEvent);
	

protected:

	bool InitGL();
	

	bool showStatistics;
	gl::TextureBlit texBlit;
};


#endif // _SDI_IO_ASYNC_WINDOW_H_
