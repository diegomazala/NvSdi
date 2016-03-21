#ifndef __GL_WINDOW_H__
#define __GL_WINDOW_H__


#include "BasicWindow.h"
#include "GL/gl.h"
#include "GL/glu.h"
#include <string>


class WINAPP_API GLWindow : public BasicWindow
{

public:

	GLWindow();

	virtual ~GLWindow();

	virtual bool Create(const CreationParameters& params, WinApp* _app);

	virtual void Render();

	virtual void Destroy();
	
	virtual void MakeCurrent() const;

	virtual void VSync(int interval=1);

	HGLRC GetGLRC(){ return hRC; };

protected:

	virtual bool InitGL();

	HGLRC		hRC;				// Permanent Rendering Context

};


#endif	// __GL_WINDOW_H__