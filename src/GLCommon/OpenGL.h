#ifndef __OPENGL_H__
#define __OPENGL_H__


#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

#include <assert.h>
#include <ctime>
#include <iostream>
#include <sstream>
#include <exception>

//#include <GL/glew.h>
//#include <GL/wglew.h>

//#include <GL/gl.h> 
#include <GL/glu.h> 
#include <GL/glext.h>
#include <GL/wglext.h>



#define glWarning std::cerr << "GL_Warning: "
#define glError std::cerr << "GL_Error: "

static const char* glGetErrorStr(GLenum errCode)
{
	switch(errCode)
	{
		case GL_INVALID_ENUM:		return " <GL_INVALID_ENUM> ";
		case GL_INVALID_VALUE:		return " <GL_INVALID_VALUE> ";
		case GL_INVALID_OPERATION:	return " <GL_INVALID_OPERATION> ";
		case GL_STACK_OVERFLOW:		return " <GL_STACK_OVERFLOW> ";
		case GL_STACK_UNDERFLOW:	return " <GL_STACK_UNDERFLOW> ";
		case GL_OUT_OF_MEMORY:		return " <GL_OUT_OF_MEMORY> ";
		case GL_TABLE_TOO_LARGE:	return " <GL_TABLE_TOO_LARGE> ";
		case GL_NO_ERROR:			return " <GL_NO_ERROR> ";
		default:					return " <GL_UNKNOWN_ERROR> ";
	}
}


namespace gl
{
	struct exception : public std::exception
	{
		explicit exception(const char* msg): std::exception((std::string("GL_Error: ")+msg).c_str()) {}
	};
}	


#define assert_msg(_Expression, _Msg) (void)( (!!(_Expression)) || (_wassert(_CRT_WIDE(_Msg), _CRT_WIDE(__FILE__), __LINE__), 0) )

#ifdef _DEBUG
#define CheckGLError(msg) {							\
	assert_msg(glGetError() == GL_NO_ERROR, msg);	\
}
#else
#define CheckGLError(msg) {									\
	const GLenum _err = glGetError();						\
	if(_err!=GL_NO_ERROR)									\
	{														\
		std::stringstream _gl_err_msg;						\
		_gl_err_msg << msg << ' ' << glGetErrorStr(_err);	\
		throw(gl::exception(_gl_err_msg.str().c_str()));	\
	}														\
}
#endif



static void glDrawCube(float size, float tx=1.f, float ty=1.f)
{
	size /= 2.f;
	glBegin(GL_QUADS);

	// face v0-v1-v2-v3
	glNormal3f(0,0,1);
	glTexCoord2f(tx, ty);  glVertex3f(size,size,size);
	glTexCoord2f(0, ty);  glVertex3f(-size,size,size);
	glTexCoord2f(0, 0);  glVertex3f(-size,-size,size);
	glTexCoord2f(tx, 0);  glVertex3f(size,-size,size);

	// face v0-v3-v4-v5
	glNormal3f(1,0,0);
	glTexCoord2f(0, ty);  glVertex3f(size,size,size);
	glTexCoord2f(0, 0);  glVertex3f(size,-size,size);
	glTexCoord2f(tx, 0);  glVertex3f(size,-size,-size);
	glTexCoord2f(tx, ty);  glVertex3f(size,size,-size);

	// face v0-v5-v6-v1
	glNormal3f(0,1,0);
	glTexCoord2f(tx, 0);  glVertex3f(size,size,size);
	glTexCoord2f(tx, ty);  glVertex3f(size,size,-size);
	glTexCoord2f(0, ty);  glVertex3f(-size,size,-size);
	glTexCoord2f(0, 0);  glVertex3f(-size,size,size);

	// face  v1-v6-v7-v2
	glNormal3f(-1,0,0);
	glTexCoord2f(tx, ty);  glVertex3f(-size,size,size);
	glTexCoord2f(0, ty);  glVertex3f(-size,size,-size);
	glTexCoord2f(0, 0);  glVertex3f(-size,-size,-size);
	glTexCoord2f(tx, 0);  glVertex3f(-size,-size,size);

	// face v7-v4-v3-v2
	glNormal3f(0,-1,0);
	glTexCoord2f(0, 0);  glVertex3f(-size,-size,-size);
	glTexCoord2f(tx, 0);  glVertex3f(size,-size,-size);
	glTexCoord2f(tx, ty);  glVertex3f(size,-size,size);
	glTexCoord2f(0, ty);  glVertex3f(-size,-size,size);

	// face v4-v7-v6-v5
	glNormal3f(0,0,-1);
	glTexCoord2f(0, 0);  glVertex3f(size,-size,-size);
	glTexCoord2f(tx, 0);  glVertex3f(-size,-size,-size);
	glTexCoord2f(tx, ty);  glVertex3f(-size,size,-size);
	glTexCoord2f(0, ty);  glVertex3f(size,size,-size);

	glEnd();
}


static bool glCheckTexture(GLuint texId, GLenum texType, int& w, int& h)
{
	int lInternalFormat;
	bool is_tex = false;
	glPushAttrib(GL_TEXTURE_BIT);
	{
		glEnable(texType);
		glBindTexture(texType, texId);
		is_tex = glIsTexture(texId);
		glGetTexLevelParameteriv(texType, 0, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(texType, 0, GL_TEXTURE_HEIGHT, &h);
		glGetTexLevelParameteriv(texType, 0, GL_TEXTURE_INTERNAL_FORMAT, &lInternalFormat);
		glDisable(texType);
		glBindTexture(texType, 0);
	}
	glPopAttrib();

	return is_tex;
}


static void glDrawQuad(float vx0, float vy0, float vx1, float vy1, bool wired)
{
	if (wired)
		glBegin(GL_LINE_LOOP);		
	else 
		glBegin(GL_QUADS);

	glVertex2f(vx0, vy0); 
	glVertex2f(vx0, vy1); 
	glVertex2f(vx1, vy1); 
	glVertex2f(vx1, vy0); 
	glEnd();
}

#endif	// __OPENGL_H__
