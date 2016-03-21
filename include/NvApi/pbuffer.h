#ifndef _CFPBUFFER
#define _CFPBUFFER

class CFPBuffer
{
    public:
        CFPBuffer() 
          : width(0), height(0), depth(0), handle(NULL), hDC(NULL), hRC(NULL), 
            hWindowDC(NULL), hWindowRC(NULL), valid(false), wglChoosePixelFormatARB(NULL),
			wglCreatePbufferARB(NULL), wglGetPbufferDCARB(NULL), wglReleasePbufferDCARB(NULL),
			wglDestroyPbufferARB(NULL)
        { 
		}
    
        bool create(int w, int h, int d, bool multisamp, int numsamps, bool texture = GL_FALSE)
        {
			wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
			wglCreatePbufferARB = (PFNWGLCREATEPBUFFERARBPROC)wglGetProcAddress("wglCreatePbufferARB");
			wglGetPbufferDCARB = (PFNWGLGETPBUFFERDCARBPROC)wglGetProcAddress("wglGetPbufferDCARB");
			wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC)wglGetProcAddress("wglReleasePbufferDCARB");
			wglDestroyPbufferARB = (PFNWGLDESTROYPBUFFERARBPROC)wglGetProcAddress("wglDestroyPbufferARB");

            // clean up pbuffer resources if already created    
			if (valid)
                destroy();
    
            width = w;
            height = h;
			depth = d;
            
            // get a copy of the main windows device context and rendering
            // context
            hWindowDC = wglGetCurrentDC();
            hWindowRC = wglGetCurrentContext();
    
            int format = 0;
            unsigned int nformats;
			int attribList[30];
			int i = 0;

			if (depth == 8) {
				attribList[i++] = WGL_RED_BITS_ARB;
				attribList[i++] = 8;
				attribList[i++] = WGL_GREEN_BITS_ARB;
				attribList[i++] = 8;
				attribList[i++] = WGL_BLUE_BITS_ARB;
				attribList[i++] = 8;
				attribList[i++] = WGL_ALPHA_BITS_ARB;
				attribList[i++] = 8;
				attribList[i++] = WGL_STENCIL_BITS_ARB;
				attribList[i++] = 8;
				attribList[i++] = WGL_DEPTH_BITS_ARB;
				attribList[i++] = 24;
				attribList[i++] = WGL_FLOAT_COMPONENTS_NV;
				attribList[i++] = GL_FALSE;
			} else if (depth == 16) {
				attribList[i++] = WGL_RED_BITS_ARB;
				attribList[i++] = 16;
				attribList[i++] = WGL_GREEN_BITS_ARB;
				attribList[i++] = 16;
				attribList[i++] = WGL_BLUE_BITS_ARB;
				attribList[i++] = 16;
				attribList[i++] = WGL_ALPHA_BITS_ARB;
				attribList[i++] = 16;
				attribList[i++] = WGL_STENCIL_BITS_ARB;
				attribList[i++] = 8;
				attribList[i++] = WGL_DEPTH_BITS_ARB;
				attribList[i++] = 24;
				attribList[i++] = WGL_FLOAT_COMPONENTS_NV;
				attribList[i++] = GL_TRUE;
			} else {
				printf("Illegal depth %d specified.\n", depth);
					return false;
			}

			attribList[i++] = WGL_DRAW_TO_PBUFFER_ARB;
			attribList[i++] = GL_TRUE;
			attribList[i++] = WGL_BIND_TO_VIDEO_RGBA_NV;
			attribList[i++] = GL_TRUE;
			attribList[i++] = WGL_DOUBLE_BUFFER_ARB;
			attribList[i++] = GL_FALSE;			
			attribList[i++] = WGL_SUPPORT_OPENGL_ARB;
			attribList[i++] = GL_TRUE;

			if (multisamp) {
				attribList[i++] = WGL_SAMPLE_BUFFERS_ARB;
				attribList[i++] = GL_TRUE;
				attribList[i++] = WGL_SAMPLES_ARB;
				attribList[i++] = numsamps;
			}
		
			if (texture) {
				attribList[i++] = WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RGBA_NV;
				attribList[i++] = GL_TRUE;
			}

			attribList[i++] = 0;
	
			// Find pixel formats that match attribute list.
            wglChoosePixelFormatARB(hWindowDC, attribList, NULL, 1, &format, &nformats);
            if (nformats == 0)
            {
                printf("Unable to find specified pixel formats\n");
                return false;
            }
        
            // Setup attribute list.
			i = 0;
            attribList[i++] = WGL_BIND_TO_VIDEO_RGBA_NV;
			attribList[i++] = GL_TRUE;
		
			if (texture) {
				attribList[i++] = WGL_TEXTURE_FORMAT_ARB;
				attribList[i++] = WGL_TEXTURE_FLOAT_RGBA_NV; 
				attribList[i++] = WGL_TEXTURE_TARGET_ARB;
				attribList[i++] = WGL_TEXTURE_RECTANGLE_NV; 
			}

			attribList[i++] = 0;
        
			// Create pbuffer.
            handle = wglCreatePbufferARB(hWindowDC, format, width, height, attribList);
            if (handle == NULL) 
            {
                printf("Unable to create pbuffer (wglCreatePbufferARB failed)\n");
                return false;
            }
        
            hDC = wglGetPbufferDCARB(handle);
            if (hDC == NULL) 
            {
                printf("Unable to retrieve handle to pbuffer device context\n");
                return false;
            }
        
            hRC = wglCreateContext(hDC);
            if (hRC == NULL) 
            {
                printf("Unable to create a rendering context for the pbuffer\n");
                return false;
            }    
            
            if (!wglShareLists(hWindowRC, hRC)) 
            {
                printf("Unable to share data between rendering contexts\n");
                return false;
            }
        
            valid = true;
    
            return true;
        }
    
        void activate()
        {
            wglMakeCurrent(hDC, hRC);
        }
    
        void deactivate()
        {
            wglMakeCurrent(hWindowDC, hWindowRC);
        }
    
        void destroy()
        {
            if (valid)
            {
                // Delete pbuffer and related
                wglDeleteContext(hRC);
                wglReleasePbufferDCARB(handle, hDC);
                wglDestroyPbufferARB(handle);
        
                // Return to the normal context
                wglMakeCurrent(hWindowDC, hWindowRC);
    
                width = 0;
                height = 0;
				depth = 0;
                handle = NULL;
                hDC = NULL;
                hRC = NULL;
                hWindowDC = NULL;
                hWindowRC = NULL;
                valid = false;
            }
        }
    
        int getWidth()
        { return width; }

        int getHeight()
        { return height; }

		int getDepth()
		{ return depth; }

		HPBUFFERARB getHandle()
		{ return handle; }

		HDC getDeviceContext()
		{ return hDC; }

		HGLRC getRenderingContext()
		{ return hRC; }

        ~CFPBuffer()
        {
            destroy();
        }
    
    private:
        int width;
        int height;
		int depth;

        HPBUFFERARB handle;
        HDC hDC;
        HGLRC hRC;

        HDC hWindowDC;
        HGLRC hWindowRC;

		PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
		PFNWGLCREATEPBUFFERARBPROC wglCreatePbufferARB;
		PFNWGLGETPBUFFERDCARBPROC wglGetPbufferDCARB;
		PFNWGLRELEASEPBUFFERDCARBPROC wglReleasePbufferDCARB;
		PFNWGLDESTROYPBUFFERARBPROC wglDestroyPbufferARB;

        bool valid;
};
#endif