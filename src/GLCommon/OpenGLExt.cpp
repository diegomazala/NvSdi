
#include "OpenGLExt.h"


#if 0


    // GL_EXT_framebuffer_object
PFNGLISRENDERBUFFEREXTPROC              glIsRenderbufferEXT = NULL;
PFNGLBINDRENDERBUFFEREXTPROC            glBindRenderbufferEXT = NULL;
PFNGLDELETERENDERBUFFERSEXTPROC         glDeleteRenderbuffersEXT = NULL;
PFNGLGENRENDERBUFFERSEXTPROC            glGenRenderbuffersEXT = NULL;
PFNGLRENDERBUFFERSTORAGEEXTPROC         glRenderbufferStorageEXT = NULL;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC  glGetRenderbufferParameterivEXT = NULL;
PFNGLISFRAMEBUFFEREXTPROC               glIsFramebufferEXT = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC             glBindFramebufferEXT = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC          glDeleteFramebuffersEXT = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC             glGenFramebuffersEXT = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC      glCheckFramebufferStatusEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC        glFramebufferTexture1DEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC        glFramebufferTexture2DEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC        glFramebufferTexture3DEXT = NULL;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC     glFramebufferRenderbufferEXT = NULL;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT = NULL;
PFNGLGENERATEMIPMAPEXTPROC              glGenerateMipmapEXT = NULL;

//GL_EXT_timer_query
PFNGLGENQUERIESPROC glGenQueries = NULL;
PFNGLDELETEQUERIESPROC glDeleteQueries = NULL;
PFNGLBEGINQUERYPROC glBeginQuery = NULL;
PFNGLENDQUERYPROC glEndQuery = NULL;
PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64vEXT = NULL;
PFNGLGETINTEGER64VPROC glGetInteger64v = NULL;

    // GL_ARB_vertex_program
PFNGLPROGRAMSTRINGARBPROC   glProgramStringARB = NULL;
PFNGLBINDPROGRAMARBPROC     glBindProgramARB = NULL;
PFNGLDELETEPROGRAMSARBPROC  glDeleteProgramsARB = NULL;
PFNGLGENPROGRAMSARBPROC     glGenProgramsARB = NULL;

	// GL_ARB_shader_objects
PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB = NULL;
PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB = NULL;
PFNGLSHADERSOURCEARBPROC glShaderSourceARB = NULL;
PFNGLCOMPILESHADERARBPROC glCompileShaderARB = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB = NULL;
PFNGLATTACHOBJECTARBPROC glAttachObjectARB = NULL;
PFNGLDELETEOBJECTARBPROC glDeleteObjectARB = NULL;
PFNGLGETINFOLOGARBPROC glGetInfoLogARB = NULL;
PFNGLLINKPROGRAMARBPROC glLinkProgramARB = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB = NULL;
PFNGLUNIFORM1IARBPROC glUniform1iARB = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB = NULL;
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = NULL;

    // WGL_gpu_affinity
PFNWGLENUMGPUSNVPROC                wglEnumGpusNV = NULL;
PFNWGLENUMGPUDEVICESNVPROC          wglEnumGpuDevicesNV = NULL;
PFNWGLCREATEAFFINITYDCNVPROC        wglCreateAffinityDCNV = NULL;
PFNWGLDELETEDCNVPROC                wglDeleteDCNV = NULL;
PFNWGLENUMGPUSFROMAFFINITYDCNVPROC  wglEnumGpusFromAffinityDCNV = NULL;

    // WGL_ARB_make_current_read
PFNWGLMAKECONTEXTCURRENTARBPROC     wglMakeContextCurrentARB = NULL;
PFNWGLGETCURRENTREADDCARBPROC       wglGetCurrentReadDCARB = NULL;

	// WGL_Swap_Interval
PFNWGLSWAPINTERVALEXTPROC		    wglSwapIntervalEXT = NULL;	

//WGL_ARB_pixel_format
PFNWGLCHOOSEPIXELFORMATARBPROC		wglChoosePixelFormat = NULL;

   // WGL_NV_Copy_image
PFNWGLCOPYIMAGESUBDATANVPROC        wglCopyImageSubDataNV = NULL;

	//GL_ARB_copy_buffer
PFNGLCOPYBUFFERSUBDATAPROC          glCopyBufferSubData = NULL;

    // GL_ARB_vertex_buffer_object
PFNGLBINDBUFFERARBPROC              glBindBuffer = NULL;
PFNGLBUFFERDATAARBPROC              glBufferData = NULL;
PFNGLBUFFERSUBDATAARBPROC           glBufferSubData = NULL;
PFNGLDELETEBUFFERSARBPROC           glDeleteBuffers = NULL;
PFNGLGENBUFFERSARBPROC              glGenBuffers = NULL;
PFNGLMAPBUFFERPROC                  glMapBuffer = NULL;
PFNGLUNMAPBUFFERPROC                glUnmapBuffer = NULL;
PFNGLMAPBUFFERRANGEPROC             glMapBufferRange = NULL;

    // GL_NV_fence
PFNGLGENFENCESNVPROC                glGenFencesNV;
PFNGLDELETEFENCESNVPROC             glDeleteFencesNV;
PFNGLSETFENCENVPROC                 glSetFenceNV;
PFNGLTESTFENCENVPROC                glTestFenceNV;





BOOL  glLoadExtensions(void)
{
    BOOL fboOK, gpuAffinityOK, vboOK, copyObjectOK, glFenceOK, timerQueryOK, glslOK;

    glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC)
                wglGetProcAddress("glIsRenderbufferEXT");
    glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)
                wglGetProcAddress("glBindRenderbufferEXT");
    glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)
                wglGetProcAddress("glDeleteRenderbuffersEXT");
    glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)
                wglGetProcAddress("glGenRenderbuffersEXT");
    glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)
                wglGetProcAddress("glRenderbufferStorageEXT");
    glGetRenderbufferParameterivEXT = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)
                wglGetProcAddress("glGetRenderbufferParameterivEXT");
    glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC)
                wglGetProcAddress("glIsFramebufferEXT");
    glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)
                wglGetProcAddress("glBindFramebufferEXT");
    glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)
                wglGetProcAddress("glDeleteFramebuffersEXT");
    glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)
                wglGetProcAddress("glGenFramebuffersEXT");
    glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)
                wglGetProcAddress("glCheckFramebufferStatusEXT");
    glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)
                wglGetProcAddress("glFramebufferTexture1DEXT");
    glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)
                wglGetProcAddress("glFramebufferTexture2DEXT");
    glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)
                wglGetProcAddress("glFramebufferTexture3DEXT");
    glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)
                wglGetProcAddress("glFramebufferRenderbufferEXT");
    glGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)
                wglGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
    glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)
                wglGetProcAddress("glGenerateMipmapEXT");

    fboOK =
            (glIsRenderbufferEXT != NULL) &&
            (glBindRenderbufferEXT != NULL) &&
            (glDeleteRenderbuffersEXT != NULL) &&
            (glGenRenderbuffersEXT != NULL) &&
            (glRenderbufferStorageEXT != NULL) &&
            (glGetRenderbufferParameterivEXT != NULL) &&
            (glIsFramebufferEXT != NULL) &&
            (glBindFramebufferEXT != NULL) &&
            (glDeleteFramebuffersEXT != NULL) &&
            (glGenFramebuffersEXT != NULL) &&
            (glCheckFramebufferStatusEXT != NULL) &&
            (glFramebufferTexture1DEXT != NULL) &&
            (glFramebufferTexture2DEXT != NULL) &&
            (glFramebufferTexture3DEXT != NULL) &&
            (glFramebufferRenderbufferEXT != NULL) &&
            (glGetFramebufferAttachmentParameterivEXT != NULL) &&
            (glGenerateMipmapEXT != NULL);


    wglEnumGpusNV = (PFNWGLENUMGPUSNVPROC)                wglGetProcAddress("wglEnumGpusNV");
    wglEnumGpuDevicesNV = (PFNWGLENUMGPUDEVICESNVPROC)                wglGetProcAddress("wglEnumGpuDevicesNV");
    wglCreateAffinityDCNV = (PFNWGLCREATEAFFINITYDCNVPROC)                wglGetProcAddress("wglCreateAffinityDCNV");
    wglDeleteDCNV = (PFNWGLDELETEDCNVPROC)                wglGetProcAddress("wglDeleteDCNV");
    wglEnumGpusFromAffinityDCNV = (PFNWGLENUMGPUSFROMAFFINITYDCNVPROC)                wglGetProcAddress("wglEnumGpusFromAffinityDCNV");


    gpuAffinityOK = 
            (wglEnumGpusNV != NULL) &&
            (wglEnumGpuDevicesNV != NULL) &&
            (wglCreateAffinityDCNV != NULL) &&
            (wglDeleteDCNV != NULL) &&
            (wglEnumGpusFromAffinityDCNV != NULL);

	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    
	wglChoosePixelFormat = (PFNWGLCHOOSEPIXELFORMATARBPROC)
		wglGetProcAddress("wglChoosePixelFormatARB");

    wglCopyImageSubDataNV = (PFNWGLCOPYIMAGESUBDATANVPROC)         wglGetProcAddress("wglCopyImageSubDataNV");
    glCopyBufferSubData = (PFNGLCOPYBUFFERSUBDATAPROC)		wglGetProcAddress("glCopyBufferSubData"); 
    copyObjectOK = (wglCopyImageSubDataNV != NULL) && (glCopyBufferSubData != NULL);

    glGenFencesNV = (PFNGLGENFENCESNVPROC)
        wglGetProcAddress("glGenFencesNV");
    glDeleteFencesNV = (PFNGLDELETEFENCESNVPROC)
        wglGetProcAddress("glDeleteFencesNV");
    glSetFenceNV = (PFNGLSETFENCENVPROC)
        wglGetProcAddress("glSetFenceNV");
    glTestFenceNV = (PFNGLTESTFENCENVPROC)
        wglGetProcAddress("glTestFenceNV");
    
    glFenceOK = 
        (glGenFencesNV != NULL) &&
        (glDeleteFencesNV != NULL) &&
        (glSetFenceNV != NULL) &&
        (glTestFenceNV != NULL);


    glGenQueries = (PFNGLGENQUERIESPROC)
		wglGetProcAddress("glGenQueries");
    glDeleteQueries = (PFNGLDELETEQUERIESPROC)
		wglGetProcAddress("glDeleteQueries");
	glBeginQuery = (PFNGLBEGINQUERYPROC)
		wglGetProcAddress("glBeginQuery");
	glEndQuery = (PFNGLENDQUERYPROC)
		wglGetProcAddress("glEndQuery");
	glGetQueryObjectui64vEXT = (PFNGLGETQUERYOBJECTUI64VEXTPROC)
		wglGetProcAddress("glGetQueryObjectui64vEXT");
	glGetInteger64v = (PFNGLGETINTEGER64VPROC)wglGetProcAddress("glGetInteger64v");		
	timerQueryOK =    
	(glGenQueries != NULL) && 
    (glDeleteQueries != NULL) &&
	(glBeginQuery != NULL) &&
	(glEndQuery != NULL) &&
	(glGetQueryObjectui64vEXT != NULL) &&
	(glGetInteger64v != NULL);

	// Initialize GLSL function entry points
	glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgramObjectARB");
	glCreateShaderObjectARB =(PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
	glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress("glShaderSourceARB");
	glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShaderARB");
	glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
	glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress("glAttachObjectARB");
	glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress("glDeleteObjectARB");
	glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");
	glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress("glLinkProgramARB");
	glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress("glUseProgramObjectARB");
	glUniform1iARB = (PFNGLUNIFORM1IARBPROC)wglGetProcAddress("glUniform1iARB");
	glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)wglGetProcAddress("glGetUniformLocationARB");

	glslOK = 
	(glCreateProgramObjectARB != NULL) && 
	(glCreateShaderObjectARB != NULL) &&
	(glShaderSourceARB != NULL) &&
	(glCompileShaderARB != NULL) && 
	(glGetObjectParameterivARB != NULL) && 
	(glAttachObjectARB != NULL) && 
	(glDeleteObjectARB != NULL) && 
	(glGetInfoLogARB != NULL) && 
	(glLinkProgramARB != NULL) && 
	(glUseProgramObjectARB != NULL) && 
	(glUniform1iARB != NULL) && 
	(glGetUniformLocationARB != NULL); 

#if 0
    // Not used here...

    glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)
                wglGetProcAddress("glProgramStringARB");
    glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)
                wglGetProcAddress("glBindProgramARB");
    glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC)
                wglGetProcAddress("glDeleteProgramsARB");
    glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)
                wglGetProcAddress("glGenProgramsARB");

    vtxPgmOK =
            (glProgramStringARB != NULL) &&
            (glBindProgramARB != NULL) &&
            (glDeleteProgramsARB != NULL) &&
            (glGenProgramsARB != NULL);

    // WGL_ARB_make_current_read
    wglMakeContextCurrentARB = (PFNWGLMAKECONTEXTCURRENTARBPROC)
                wglGetProcAddress("wglMakeContextCurrentARB");
    wglGetCurrentReadDCARB = (PFNWGLGETCURRENTREADDCARBPROC)
                wglGetProcAddress("wglGetCurrentReadDCARB");

    makeCurReadOK =
            (wglMakeContextCurrentARB != NULL) &&
            (wglGetCurrentReadDCARB != NULL);
#endif
    glBindBuffer = (PFNGLBINDBUFFERARBPROC)
                wglGetProcAddress("glBindBufferARB");
    glBufferData = (PFNGLBUFFERDATAARBPROC)
                wglGetProcAddress("glBufferDataARB");
    glBufferSubData = (PFNGLBUFFERSUBDATAARBPROC)
                wglGetProcAddress("glBufferSubDataARB");
    glDeleteBuffers = (PFNGLDELETEBUFFERSARBPROC)
                wglGetProcAddress("glDeleteBuffersARB");
    glGenBuffers = (PFNGLGENBUFFERSARBPROC)
                wglGetProcAddress("glGenBuffersARB");
    glMapBuffer = (PFNGLMAPBUFFERPROC)
                wglGetProcAddress("glMapBufferARB");
    glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)
                wglGetProcAddress("glMapBufferRange");
    glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)
                wglGetProcAddress("glUnmapBufferARB");

    vboOK =
            (glBindBuffer != NULL) &&
            (glBufferData != NULL) &&
            (glBufferSubData !=  NULL) &&
            (glDeleteBuffers != NULL) &&
            (glGenBuffers != NULL ) &&
            (glMapBuffer != NULL) &&
            (glUnmapBuffer != NULL) &&
            (glMapBufferRange != NULL);

    return fboOK && gpuAffinityOK && vboOK && copyObjectOK && glFenceOK && timerQueryOK && glslOK;
}

#endif



