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

	void SetType(GLenum target)
	{
		mTarget = target;
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
	GLenum			mTarget;	///< Texture target (1D, 2D, 3D)
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




//////////////
// TYPEDEFS //
//////////////
typedef BOOL(WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC(WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int *attribList);
typedef BOOL(WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);
typedef void (APIENTRY * PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRY * PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (APIENTRY * PFNGLBUFFERDATAPROC) (GLenum target, ptrdiff_t size, const GLvoid *data, GLenum usage);
typedef void (APIENTRY * PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef GLuint(APIENTRY * PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint(APIENTRY * PFNGLCREATESHADERPROC) (GLenum type);
typedef void (APIENTRY * PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void (APIENTRY * PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (APIENTRY * PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint *arrays);
typedef void (APIENTRY * PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRY * PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (APIENTRY * PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY * PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
typedef GLint(APIENTRY * PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const char *name);
typedef void (APIENTRY * PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, char *infoLog);
typedef void (APIENTRY * PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, char *infoLog);
typedef void (APIENTRY * PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (APIENTRY * PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const char* *string, const GLint *length);
typedef void (APIENTRY * PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (APIENTRY * PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const char *name);
typedef GLint(APIENTRY * PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const char *name);
typedef void (APIENTRY * PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY * PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (APIENTRY * PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void (APIENTRY * PFNGLGENERATEMIPMAPPROC) (GLenum target);
typedef void (APIENTRY * PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (APIENTRY * PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY * PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY * PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);



class TextureBlit
{

public:

	TextureBlit() :isCreated(false){}

	void Blit(int window_width, int window_height)
	{
		glBindVertexArray(vao);
		if (windowSizeLocation > -1)
		{
			const float windowSize[] = { window_width, window_height };
			glUniform2fv(windowSizeLocation, 1, windowSize);
		}
		glDrawArrays(GL_QUADS, 0, 4);
	}

	void Blit(int shader_program, int window_width, int window_height)
	{
		glUseProgram(shader_program);
		Blit(window_width, window_height);
		glUseProgram(0);
	}

	void BlitDefault(int window_width, int window_height)
	{
		glUseProgram(program);
		Blit(window_width, window_height);
		glUseProgram(0);
	}


	void CreateVbo(float scale = 1.0f)
	{
		LoadExtensionList();

		const float v = 1.f * scale;

		const float vertex_data[] = {
			-v, -v, 0.f,
			-v, v, 0.f,
			v, v, 0.f,
			v, -v, 0.f
		};

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		SetupDefaultShaderProgram();

		isCreated = (glGetError() == GL_NO_ERROR);
	}

	void DestroyVbo()
	{
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}

	bool SetupDefaultShaderProgram()
	{
		const char* vertex_shader =
			"#version 400\n"
			"layout(location = 0) in vec3 inVertex;"
			"void main() {"
			"  gl_Position = vec4(inVertex, 1.0);"
			"}";

		const char* fragment_shader =
			"#version 400\n"
			"uniform sampler2D inTex;"
			"uniform vec2 windowSize;"
			"out vec4 color;"
			"void main() {"
			"  color = texture(inTex, gl_FragCoord.xy/windowSize);"
			"}";

		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &vertex_shader, NULL);
		glCompileShader(vs);
		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, &fragment_shader, NULL);
		glCompileShader(fs);

		program = glCreateProgram();
		glAttachShader(program, fs);
		glAttachShader(program, vs);
		glLinkProgram(program);

		windowSizeLocation = glGetUniformLocation(program, "windowSize");

		return (glGetError() == GL_NO_ERROR) && (windowSizeLocation > -1);
	}

	GLuint SetupShaderProgram(const char* vertex_shader, const char* fragment_shader)
	{
		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &vertex_shader, NULL);
		glCompileShader(vs);
		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, &fragment_shader, NULL);
		glCompileShader(fs);

		GLuint shader_program = glCreateProgram();
		glAttachShader(shader_program, fs);
		glAttachShader(shader_program, vs);
		glLinkProgram(shader_program);

		windowSizeLocation = glGetUniformLocation(program, "windowSize");
	}


private:

	bool LoadExtensionList()
	{
		glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
		if (!glAttachShader)
		{
			return false;
		}

		glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
		if (!glBindBuffer)
		{
			return false;
		}

		glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
		if (!glBindVertexArray)
		{
			return false;
		}

		glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
		if (!glBufferData)
		{
			return false;
		}

		glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
		if (!glCompileShader)
		{
			return false;
		}

		glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
		if (!glCreateProgram)
		{
			return false;
		}

		glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
		if (!glCreateShader)
		{
			return false;
		}

		glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
		if (!glDeleteBuffers)
		{
			return false;
		}

		glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
		if (!glDeleteProgram)
		{
			return false;
		}

		glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
		if (!glDeleteShader)
		{
			return false;
		}

		glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
		if (!glDeleteVertexArrays)
		{
			return false;
		}

		glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");
		if (!glDetachShader)
		{
			return false;
		}

		glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
		if (!glEnableVertexAttribArray)
		{
			return false;
		}

		glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
		if (!glGenBuffers)
		{
			return false;
		}

		glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
		if (!glGenVertexArrays)
		{
			return false;
		}

		glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
		if (!glGetAttribLocation)
		{
			return false;
		}

		glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
		if (!glGetProgramInfoLog)
		{
			return false;
		}

		glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
		if (!glGetProgramiv)
		{
			return false;
		}

		glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
		if (!glGetShaderInfoLog)
		{
			return false;
		}

		glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
		if (!glGetShaderiv)
		{
			return false;
		}

		glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
		if (!glLinkProgram)
		{
			return false;
		}

		glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
		if (!glShaderSource)
		{
			return false;
		}

		glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
		if (!glUseProgram)
		{
			return false;
		}

		glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
		if (!glVertexAttribPointer)
		{
			return false;
		}

		glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)wglGetProcAddress("glBindAttribLocation");
		if (!glBindAttribLocation)
		{
			return false;
		}

		glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
		if (!glGetUniformLocation)
		{
			return false;
		}

		glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
		if (!glUniformMatrix4fv)
		{
			return false;
		}

		glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
		if (!glActiveTexture)
		{
			return false;
		}

		glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
		if (!glUniform1i)
		{
			return false;
		}

		glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
		if (!glGenerateMipmap)
		{
			return false;
		}

		glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
		if (!glDisableVertexAttribArray)
		{
			return false;
		}

		glUniform2fv = (PFNGLUNIFORM2FVPROC)wglGetProcAddress("glUniform2fv");
		if (!glUniform2fv)
		{
			return false;
		}

		glUniform3fv = (PFNGLUNIFORM3FVPROC)wglGetProcAddress("glUniform3fv");
		if (!glUniform3fv)
		{
			return false;
		}

		glUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");
		if (!glUniform4fv)
		{
			return false;
		}

		return true;
	}



	GLuint program;
	GLuint vao;
	GLuint vbo;
	GLint windowSizeLocation;
	GLboolean isCreated;

	
	PFNGLATTACHSHADERPROC glAttachShader;
	PFNGLBINDBUFFERPROC glBindBuffer;
	PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
	PFNGLBUFFERDATAPROC glBufferData;
	PFNGLCOMPILESHADERPROC glCompileShader;
	PFNGLCREATEPROGRAMPROC glCreateProgram;
	PFNGLCREATESHADERPROC glCreateShader;
	PFNGLDELETEBUFFERSPROC glDeleteBuffers;
	PFNGLDELETEPROGRAMPROC glDeleteProgram;
	PFNGLDELETESHADERPROC glDeleteShader;
	PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
	PFNGLDETACHSHADERPROC glDetachShader;
	PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
	PFNGLGENBUFFERSPROC glGenBuffers;
	PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
	PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
	PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
	PFNGLGETPROGRAMIVPROC glGetProgramiv;
	PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
	PFNGLGETSHADERIVPROC glGetShaderiv;
	PFNGLLINKPROGRAMPROC glLinkProgram;
	PFNGLSHADERSOURCEPROC glShaderSource;
	PFNGLUSEPROGRAMPROC glUseProgram;
	PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
	PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
	PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
	PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
	PFNGLACTIVETEXTUREPROC glActiveTexture;
	PFNGLUNIFORM1IPROC glUniform1i;
	PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
	PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
	PFNGLUNIFORM2FVPROC glUniform2fv;
	PFNGLUNIFORM3FVPROC glUniform3fv;
	PFNGLUNIFORM4FVPROC glUniform4fv;

};

} // end namespace 

#endif //__TEXTURE_H__
