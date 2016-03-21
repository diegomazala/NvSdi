/**
 ***********************************************************************
 * \class dm::Texture
 * \brief Encapsulate several functionalities from Opengl to handle textures.
 ***********************************************************************
 * \author		Diego Mazala - diegomazala@gmail.com
 * \date		March, 2009
 ***********************************************************************/


#ifndef __TEXTURE_H__
#define __TEXTURE_H__


#ifdef _WIN32#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

#include "OpenGLExt.h"


namespace gl
{

template<typename T>
struct TRgba
{
	T data[4];
	TRgba(){}
	TRgba(T r, T g, T b, T a)//:r(r),g(g),b(b),a(a){}
	{
		data[0] = r; data[1] = g; data[2] = b; data[3] = a;
	}
	TRgba& operator= (const TRgba& c)
	{
		memcpy(this->data, c.data, sizeof(float)*4);
		return *this;
	}
};
typedef TRgba<unsigned char> Rgba;



class Texture
{

//===========================================================	
public:											// Methods

	//
	//! Constructor. Initialize attributes
	Texture(GLenum target=GL_NONE):
		mId(0),
		mTarget(target),
		mValid(false),
		mMipmapped(false)
		{}

	virtual ~Texture()
	{
		Destroy();
	};

	GLenum Type() const
	{
		return mTarget;
	}

	GLuint Id() const
	{
		return mId;
	}

	void SetId(GLuint id)
	{
		mId = id;
	}

	GLvoid Create()		
	{ 
		glGenTextures(1, &mId); 
		mValid = GL_TRUE;
	}

	GLvoid Destroy()	
	{ 
		if(mValid)
			glDeleteTextures(1, &mId); 
		mValid	= false;
		mId		= 0;
	}

	GLvoid Enable() const
	{
		glEnable(mTarget);
	}

	GLvoid Disable() const
	{
		glDisable(mTarget);
	}

	// Register and upload the texture in OpenGL
	GLvoid Bind() const
	{ 
		glBindTexture(mTarget, mId); 
	}

	GLvoid Unbind() const
	{ 
		glBindTexture(mTarget, 0); 
	}

	GLboolean Valid() const 
	{ 
		return mValid; 
	}

	GLvoid SetMipmap(GLboolean mipmap)
	{
		mMipmapped = mipmap;
	}

	GLvoid SetParameter(GLenum paramName, GLint paramValue) const
	{ 
		glTexParameteri(mTarget, paramName, paramValue); 
	}

	GLvoid SetParameter(GLenum paramName, GLfloat paramValue) const
	{ 
		glTexParameterf(mTarget, paramName, paramValue); 
	}

	GLvoid SetParameter(GLenum paramName, GLint * pParamValue) const
	{ 
		glTexParameteriv(mTarget, paramName, pParamValue); 
	}

	GLvoid SetParameter(GLenum paramName, GLfloat * pParamValue) const
	{ 
		glTexParameterfv(mTarget, paramName, pParamValue); 
	}


//===========================================================
protected:										// Attributes


	GLuint			mId;		///< Texture identification
	const GLenum	mTarget;	///< Texture target (1D, 2D, 3D)
	GLboolean		mValid;		///< Validation of texture		
	GLboolean		mMipmapped;	///< Build the texture mipmapped
};




class Texture2D : public Texture
{
public:
	Texture2D():Texture(GL_TEXTURE_2D){}
	virtual ~Texture2D(){};

	template<typename T>
	GLvoid BuildFromImage(const T& image) const
	{
		BuildFromImage(image, mTarget, mMipmapped);
	}

	GLvoid BuildEmpty(int w, int h, const Rgba& pixelColor) const
	{
		const GLenum target = mTarget;
		GLboolean mipmapped = mMipmapped;

		const int size = w*h;
		Rgba* lpData = new Rgba[size];
		for(int i=0; i<size; ++i)
				lpData[i] = pixelColor;

		if(mipmapped)
		{
			gluBuild2DMipmaps(	target, 
								GL_RGBA8, 
								w, 
								h, 
								GL_RGBA,
								GL_UNSIGNED_BYTE, 
								(const void *)lpData);
		}
		else
		{
			glTexImage2D(	target, 
							0, 
							GL_RGBA8, 
							w, 
							h, 
							0,
							GL_RGBA, 
							GL_UNSIGNED_BYTE, 
							(const void *)lpData);
		}
		delete [] lpData;
	}

	GLvoid BuildNull(int w, int h) const
	{
		const GLenum target = mTarget;
		GLboolean mipmapped = mMipmapped;

		if(mipmapped)
		{
			gluBuild2DMipmaps(	target, 
								GL_RGBA8, 
								w, 
								h, 
								GL_RGBA,
								GL_UNSIGNED_BYTE, 
								NULL);
		}
		else
		{
			glTexImage2D(	target, 
							0, 
							GL_RGBA8, 
							w, 
							h, 
							0,
							GL_RGBA, 
							GL_UNSIGNED_BYTE, 
							NULL);
		}
	}

	GLvoid BuildFromData(int w, int h, const Rgba pixelData[]) const
	{
		const GLenum target = mTarget;
		GLboolean mipmapped = mMipmapped;

		if(mipmapped)
		{
			gluBuild2DMipmaps(	target, 
								GL_RGBA8, 
								w, 
								h, 
								GL_RGBA,
								GL_UNSIGNED_BYTE, 
								(const void *)pixelData);
		}
		else
		{
			glTexImage2D(	target, 
							0, 
							GL_RGBA8, 
							w, 
							h, 
							0,
							GL_RGBA, 
							GL_UNSIGNED_BYTE, 
							(const void *)pixelData);
		}
	}
	
	void Plot(int win_width, int win_height, int tex_width, int tex_height)
	{

		int tx, ty;

		if(this->Type() == GL_TEXTURE_2D)
		{
			tx = ty = 1;
		}
		else
		{
			tx = tex_width;
			ty = tex_height;
		}

		glPushAttrib(GL_ALL_ATTRIB_BITS);
		{

			//glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, win_width, win_height);

			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			
			// Projection settings
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluOrtho2D( 0.0, 1.0, 0.0, 1.0 ); 

			// View settings
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			// Draw image 
			this->Draw(0, 0, 1, 1, 0, 0, tx, ty);
		}	
		glPopAttrib();
	}

	void Draw(float vx0, float vy0, float vx1, float vy1, float tx0, float ty0, float tx1, float ty1)
	{
		this->Enable();
		this->Bind();

		glBegin(GL_QUADS);
		{
			glTexCoord2f(tx0, ty0);	glVertex2f(vx0, vy0); 
			glTexCoord2f(tx0, ty1);	glVertex2f(vx0, vy1); 
			glTexCoord2f(tx1, ty1);	glVertex2f(vx1, vy1); 
			glTexCoord2f(tx1, ty0);	glVertex2f(vx1, vy0); 
		}
		glEnd();

		this->Unbind();
		this->Disable();
	}


	GLboolean CopyFrom(GLuint srcTexId, GLenum srcTexType, HGLRC hSrcRC, HGLRC hDstRC, GLsizei width, GLsizei height) 
	{
		if(mWglCopyImageSubDataNV==NULL)
			mWglCopyImageSubDataNV = (PFNWGLCOPYIMAGESUBDATANVPROC)wglGetProcAddress("wglCopyImageSubDataNV");

		return mWglCopyImageSubDataNV(hSrcRC, srcTexId, srcTexType, 0, 0, 0, 0,
									hDstRC, this->Id(), this->Type(), 0, 0, 0, 0,
									width, height, 1);
	}

	GLboolean CopyFrom(const Texture2D& srcTex, HGLRC hSrcRC, HGLRC hDstRC, GLsizei width, GLsizei height) 
	{
		if(mWglCopyImageSubDataNV==NULL)
			mWglCopyImageSubDataNV = (PFNWGLCOPYIMAGESUBDATANVPROC)wglGetProcAddress("wglCopyImageSubDataNV");

		return mWglCopyImageSubDataNV(hSrcRC, srcTex.Id(), srcTex.Type(), 0, 0, 0, 0,
									hDstRC, this->Id(), this->Type(), 0, 0, 0, 0,
									width, height, 1);
	}

	GLboolean CopyTo(Texture2D& dstTex, HGLRC hDstRC, HGLRC hSrcRC, GLsizei width, GLsizei height) 
	{
		if(mWglCopyImageSubDataNV==NULL)
			mWglCopyImageSubDataNV = (PFNWGLCOPYIMAGESUBDATANVPROC)wglGetProcAddress("wglCopyImageSubDataNV");

		return mWglCopyImageSubDataNV(hSrcRC, this->Id(), this->Type(), 0, 0, 0, 0,
									hDstRC, dstTex.Id(), dstTex.Type(), 0, 0, 0, 0,
									width, height, 1);
	}
	
	GLint GetWidth() const
	{
		GLint w;
		glGetTexLevelParameteriv(mTarget, 0, GL_TEXTURE_WIDTH, &w);
		return w;
	}

	GLint GetHeight() const
	{
		GLint h;
		glGetTexLevelParameteriv(mTarget, 0, GL_TEXTURE_HEIGHT, &h);
		return h;
	}


	GLvoid GetData(unsigned char* &pImageData, GLenum pixelType=GL_BGRA)
	{
		glGetTexImage(	mTarget, 
						0, 
						pixelType, 
						GL_UNSIGNED_BYTE, 
						pImageData);
	}

protected:

	template<typename T>
	GLvoid BuildFromImage(const T& image, GLenum target, GLboolean mipmapped) const
	{
		if(mipmapped)
		{
			gluBuild2DMipmaps(	target, 
								GL_RGBA8, 
								image.Width(), 
								image.Height(), 
								GL_RGBA,
								GL_UNSIGNED_BYTE, 
								(const void *)image.Data());
		}
		else
		{
			glTexImage2D(	target, 
							0, 
							GL_RGBA, 
							image.Width(), 
							image.Height(), 
							0,
							GL_RGBA, 
							GL_UNSIGNED_BYTE, 
							(const void *)image.Data());
		}
	}

	
protected:
	Texture2D(GLenum target):Texture(target),mWglCopyImageSubDataNV(NULL){}

	PFNWGLCOPYIMAGESUBDATANVPROC        mWglCopyImageSubDataNV;
};


class TextureRectARB : public Texture2D
{
public:
	TextureRectARB():Texture2D(GL_TEXTURE_RECTANGLE_ARB){}
	virtual ~TextureRectARB(){};

};

class TextureRectNV : public Texture2D
{
public:
	TextureRectNV():Texture2D(GL_TEXTURE_RECTANGLE_NV){}
	virtual ~TextureRectNV(){};

};


} // end namespace 

#endif //__TEXTURE_H__
