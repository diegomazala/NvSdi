#ifndef __GL_SDI_OUT_WINDOW_H__
#define __GL_SDI_OUT_WINDOW_H__



// From Timer.cpp
__int64 GetNanoClock();

#include "OpenGL.h"
#include "WinAppGL/GLWindow.h"


class SdiOutWindow : public GLWindow
{
public:
	SdiOutWindow();
	~SdiOutWindow();
	
	void Render();

	void SetOpenGLState();
	
	void Draw(int w, int h);
	
	void OnKeyEvent(const KeyEvent* pEvent);

	

protected:

	bool presenting;

	bool InitGL();

};


#endif // __GL_SDI_OUT_WINDOW_H__
