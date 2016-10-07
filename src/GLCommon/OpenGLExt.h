#ifndef __OPENGL_EXT_H__
#define __OPENGL_EXT_H__

#include "OpenGL.h"

#if 0
extern BOOL  glLoadExtensions(void);

    // GL_EXT_framebuffer_object
extern PFNGLISRENDERBUFFEREXTPROC              glIsRenderbufferEXT;
extern PFNGLBINDRENDERBUFFEREXTPROC            glBindRenderbufferEXT;
extern PFNGLDELETERENDERBUFFERSEXTPROC         glDeleteRenderbuffersEXT;
extern PFNGLGENRENDERBUFFERSEXTPROC            glGenRenderbuffersEXT;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC         glRenderbufferStorageEXT;
extern PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC  glGetRenderBufferParameterivEXT;
extern PFNGLISFRAMEBUFFEREXTPROC               glIsFramebufferEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC             glBindFramebufferEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC          glDeleteFramebuffersEXT;
extern PFNGLGENFRAMEBUFFERSEXTPROC             glGenFramebuffersEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC      glCheckFramebufferStatusEXT;
extern PFNGLFRAMEBUFFERTEXTURE1DEXTPROC        glFramebufferTexture1DEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC        glFramebufferTexture2DEXT;
extern PFNGLFRAMEBUFFERTEXTURE3DEXTPROC        glFramebufferTexture3DEXT;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC     glFramebufferRenderbufferEXT;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT;
extern PFNGLGENERATEMIPMAPEXTPROC              glGenerateMipmapEXT;


//GL_EXT_timer_query
extern PFNGLGENQUERIESPROC glGenQueries;
extern PFNGLDELETEQUERIESPROC glDeleteQueries;
extern PFNGLBEGINQUERYPROC glBeginQuery;
extern PFNGLENDQUERYPROC glEndQuery;
extern PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64vEXT;
extern PFNGLGETINTEGER64VPROC glGetInteger64v;

    // GL_ARB_vertex_program
extern PFNGLPROGRAMSTRINGARBPROC   glProgramStringARB;
extern PFNGLBINDPROGRAMARBPROC     glBindProgramARB;
extern PFNGLDELETEPROGRAMSARBPROC  glDeleteProgramsARB;
extern PFNGLGENPROGRAMSARBPROC     glGenProgramsARB;

	// GL_ARB_shader_objects
extern PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
extern PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
extern PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
extern PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
extern PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
extern PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
extern PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;
extern PFNGLUNIFORM1IARBPROC glUniform1iARB;
extern PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB;
extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;

    // WGL_gpu_affinity
extern PFNWGLENUMGPUSNVPROC                wglEnumGpusNV;
extern PFNWGLENUMGPUDEVICESNVPROC          wglEnumGpuDevicesNV;
extern PFNWGLCREATEAFFINITYDCNVPROC        wglCreateAffinityDCNV;
extern PFNWGLDELETEDCNVPROC                wglDeleteDCNV;
extern PFNWGLENUMGPUSFROMAFFINITYDCNVPROC  wglEnumGpusFromAffinityDCNV;

    // WGL_ARB_make_current_read
extern PFNWGLMAKECONTEXTCURRENTARBPROC     wglMakeContextCurrentARB;
extern PFNWGLGETCURRENTREADDCARBPROC       wglGetCurrentReadDCARB;

	// WGL_Swap_Interval
extern PFNWGLSWAPINTERVALEXTPROC		   wglSwapIntervalEXT;	

	//WGL_ARB_pixel_format
extern PFNWGLCHOOSEPIXELFORMATARBPROC		wglChoosePixelFormat;
    
// WGL_NV_Copy_image
extern PFNWGLCOPYIMAGESUBDATANVPROC        wglCopyImageSubDataNV;
	
	//GL_ARB_copy_buffer
extern PFNGLCOPYBUFFERSUBDATAPROC          glCopyBufferSubData;

    // GL_ARB_vertex_buffer_object
extern PFNGLBINDBUFFERARBPROC              glBindBuffer;
extern PFNGLBUFFERDATAARBPROC              glBufferData;
extern PFNGLBUFFERSUBDATAARBPROC           glBufferSubData;
extern PFNGLDELETEBUFFERSARBPROC           glDeleteBuffers;
extern PFNGLGENBUFFERSARBPROC              glGenBuffers;
extern PFNGLMAPBUFFERPROC                  glMapBuffer;
extern PFNGLUNMAPBUFFERPROC                glUnmapBuffer;
extern PFNGLMAPBUFFERRANGEPROC             glMapBufferRange;

    // GL_NV_fence
extern PFNGLGENFENCESNVPROC                glGenFencesNV;
extern PFNGLDELETEFENCESNVPROC             glDeleteFencesNV;
extern PFNGLSETFENCENVPROC                 glSetFenceNV;
extern PFNGLTESTFENCENVPROC                glTestFenceNV;

// Definitions for extension constants
#define GL_PIXEL_UNPACK_BUFFER	0x88EC
#define GL_PIXEL_PACK_BUFFER	0x88EB

// Options for glMapBufferRange
#define MAP_READ_BIT		    0x0001
#define MAP_WRITE_BIT		    0x0002
#define MAP_INVALIDATE_RANGE_BIT    0x0004
#define MAP_INVALIDATE_BUFFER_BIT   0x0008
#define MAP_FLUSH_EXPLICIT_BIT	    0x0010
#define MAP_UNSYNCHRONIZED_BIT	    0x0020

// GL fence constants
#define ALL_COMPLETED_NV                   0x84F2



namespace glExt
{
	static HWND		hWnd	= NULL;
	static HGLRC	hGLRC	= NULL;

	static void CreateDummyGLWindow()
	{
		HINSTANCE hInstance = ::GetModuleHandle(NULL);	// Need a handle to this process instance    		
		// Create the window class
		WNDCLASSEX	wc;						// Windows Class Structure
		wc.cbSize			= sizeof(WNDCLASSEX); 
		wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
		wc.lpfnWndProc		= DefWindowProc;			    // WndProc Handles Messages, we'll pass a static version, which will receive a 'this' pointer
		wc.cbClsExtra		= 0;									// No Extra Window Data
		wc.cbWndExtra		= 0;									// No Extra Window Data
		wc.hInstance		= hInstance;							// Set The Instance
		wc.hIcon			= NULL;									// Load The Default Icon
		wc.hIconSm		    = NULL; 
		wc.hCursor			= LoadCursor(hInstance, IDC_ARROW);		// Load The Arrow Pointer
		wc.hbrBackground	= NULL;									// No Background Required For GL
		wc.lpszMenuName		= NULL;									// We Don't Want A Menu
		wc.lpszClassName	= "Dummy";						// Set The Class Name

		// register the window class
		::RegisterClassEx(&wc);
	    
 			// Call the windows function to create the window.  The 
		hWnd = ::CreateWindowEx(	NULL, "Dummy", NULL, NULL, 0, 0, 1, 1,	NULL, NULL, NULL, NULL);								
            	  
		// Get the windows device context
		HDC hDC=::GetDC(hWnd);	
      	
		///// STEP 2: Now we need to create an OpenGL context for this window

		// We need a pixel format descriptor.  A PFD tells how OpenGL draws
		static	PIXELFORMATDESCRIPTOR pfd=				
		{
			sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
			1,											// Version Number
			PFD_DRAW_TO_WINDOW |						// Format Must Support Window
			PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
			PFD_DOUBLEBUFFER,							// Must Support Double Buffering
			PFD_TYPE_RGBA,								// Request An RGBA Format
			8,										    // Select Our Color Depth
			0, 0, 0, 0, 0, 0,							// Color Bits Ignored
			0,											// No Alpha Buffer
			0,											// Shift Bit Ignored
			0,											// No Accumulation Buffer
			0, 0, 0, 0,									// Accumulation Bits Ignored
			16,											// 16Bit Z-Buffer (Depth Buffer)  
			0,											// No Stencil Buffer
			0,											// No Auxiliary Buffer
			PFD_MAIN_PLANE,								// Main Drawing Layer
			0,											// Reserved
			0, 0, 0										// Layer Masks Ignored
		};
	
		GLuint PixelFormat;			
		// create the pixel pixel format descriptor
		PixelFormat=::ChoosePixelFormat(hDC,&pfd);
		//  set the pixel format descriptor
		::SetPixelFormat(hDC,PixelFormat,&pfd);	    
		// Create a wGL rendering context
		hGLRC=wglCreateContext(hDC);	    
		//  Activate the rendering context
		wglMakeCurrent(hDC,hGLRC);	

		CheckGLError("<glExt::CreateDummyGLWindow>");
	}

	static void DestroyDummyGLWindow()
	{
		// We can kill the dummy window now
		HDC hDC=::GetDC(hWnd);
		wglMakeCurrent(NULL,NULL);	
		wglDeleteContext(hGLRC);
		::ReleaseDC(hWnd,hDC);
		::DestroyWindow(hWnd);
		hWnd = NULL;
		hGLRC = NULL;
	}
}
#endif

#endif // __OPENGL_EXT_H__
