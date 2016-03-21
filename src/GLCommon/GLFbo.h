////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class	GLFbo
///
/// \brief	
///
///	This class has two frame buffers objects (FBO) and use them to provide support for
/// render to texture. It also supports multisampled (antialised) rendering. 
///
/// \author	Diego
/// \date	April 2011
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __GL_FBO_H__
#define __GL_FBO_H__


#include <windows.h>
#include <iostream>
#include <exception>


#ifdef GLNVSDI_EXPORTS
#define GLNVSDI_API __declspec(dllexport)
#else
#define GLNVSDI_API __declspec(dllimport)
#endif

#include "OpenGL.h"
#include "Texture.h"
#include "OpenGLExt.h"
#include "Fbo.h"



class GLNVSDI_API GLFbo
{
public:

	GLFbo();
	~GLFbo();

	void Initialize(int w, int h, 
					int sizeBits,           // bits per component
					int num_samples,		// number of samples
					GLboolean alpha,        // alpha
					GLboolean depth,		// depth
					gl::Texture* pTex_0,	// texture object - field 0
					gl::Texture* pTex_1=NULL);// texture object - field 1

	void Uninitialize();
	
	bool IsValid(int field=0) const;

	void BeginRender(int field=0);
	void EndRender(int field=0);

	Fbo* GetFbo(int field=0) const;
	gl::Texture* GetTexture(int field=0) const;

	int Width() const;
	int Height() const;

private:

	bool mMultisampling;

	Fbo* mpFbo[2];
	Fbo* mpFboMulti[2];

	int mWidth;
	int mHeight;

	gl::Texture* mpTex[2];
};

#endif // __GL_NV_GVO_FBO_H__