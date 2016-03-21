#ifndef _CFBO
#define _CFBO


// Useful Macros

class CFBO
{
    public:
        CFBO() 
          : fboId(0), numRenderbuffers(0), valid(GL_FALSE),
			glBindFramebufferEXT(NULL), glBindRenderbufferEXT(NULL), 
			glDeleteFramebuffersEXT(NULL), glDeleteRenderbuffersEXT(NULL), 
			glFramebufferRenderbufferEXT(NULL), glGenFramebuffersEXT(NULL),
			glGenRenderbuffersEXT(NULL), glRenderbufferStorageEXT(NULL),
			glFramebufferTexture2DEXT(NULL), glCheckFramebufferStatusEXT(NULL),
			glRenderbufferStorageMultisampleEXT(NULL), glBlitFramebufferEXT(NULL)
        { 
		}


		void status()
		{
			GLenum status;
			status = (GLenum) glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			switch(status) {
				case GL_FRAMEBUFFER_COMPLETE_EXT:
					break;
				case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
					printf("Unsupported framebuffer format\n");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
					printf("Framebuffer incomplete, incomplete attachment\n");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
					printf("Framebuffer incomplete, missing attachment\n");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
					printf("Framebuffer incomplete, attached images must have same dimensions\n");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
					printf("Framebuffer incomplete, attached images must have same format\n");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
					printf("Framebuffer incomplete, missing draw buffer\n");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
					printf("Framebuffer incomplete, missing read buffer\n");
					break;
				default:
					assert(0);
			}
		}

        bool create(int width, int height,
                    int sizeBits,             // bits per component
					int num_samples,		  // number of samples
                    GLboolean alpha,          // alpha
                    GLboolean depth,		  // depth
					GLuint textureObject)	  // texture object          

        {
			GLenum texFormat;

			glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
			glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbufferEXT");
			glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffersEXT");
			glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)wglGetProcAddress("glDeleteRenderbuffersEXT");
			glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
			glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
			glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
			glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
			glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)wglGetProcAddress("glRenderbufferStorageMultisampleEXT");
			glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
			glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
			glBlitFramebufferEXT = (PFNGLBLITFRAMEBUFFEREXTPROC)wglGetProcAddress("glBlitFramebufferEXT");
			glGetRenderbufferParameterivEXT = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)wglGetProcAddress("glGetRenderbufferParameterivEXT");

			// Clean up fbo resources if already created    
			if (valid)
                destroy();

            switch (sizeBits) {
            case 8:
                if (alpha) {
                    texFormat = GL_RGBA8;
                } else {
                    texFormat = GL_RGB8;
                }
                break;

            case 16:
                if (alpha) {
                    texFormat = GL_RGBA16F_ARB;
                } else {
                    texFormat = GL_RGB16F_ARB;
                }
                break;

            default:
                return GL_FALSE;
            }

			// Generate FBO
            glGenFramebuffersEXT(1, &fboId);

			// Bind FBO
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);

			// Calculate the number of required render buffers.
            numRenderbuffers = 1;
            if (depth) {
                numRenderbuffers++;
            }

			// Generate required render buffers.
            glGenRenderbuffersEXT(numRenderbuffers, renderbufferIds);

			// Bind color render buffer.
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbufferIds[0]);

			// Allocate storage for color render buffer
			if ((!textureObject) && (num_samples > 1)) {
				glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, num_samples, texFormat, width, height);
			} else if (!textureObject){
				glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, texFormat, width, height);
			}

			// Get number of samples from allocated color render buffer
			glGetRenderbufferParameterivEXT(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_SAMPLES_EXT, &num_samples);

			// Attach color render buffer.
			if (!textureObject) {
				glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
					                         GL_COLOR_ATTACHMENT0_EXT,
						                     GL_RENDERBUFFER_EXT,
							                 renderbufferIds[0]);
			} else {
				glBindTexture(GL_TEXTURE_RECTANGLE_NV, textureObject);
				glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
					                       GL_TEXTURE_RECTANGLE_NV, textureObject, 0 );
			}

			status();

			// Bind and allocate storage for depth render buffer
            if (depth) {
				glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbufferIds[1]);
				if (num_samples > 1) {
					glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, num_samples, GL_DEPTH_COMPONENT, width, height);
				} else {
					glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
				}
			}

			// Attached depth render buffer.
            if (depth) {
                glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                                             GL_DEPTH_ATTACHMENT_EXT,
                                             GL_RENDERBUFFER_EXT,
                                             renderbufferIds[1]);
            }
   
			status();

            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

            if (glGetError() == GL_NO_ERROR) {
                valid = GL_TRUE;
				return GL_TRUE;
            } else {
                destroy();
			}

            return GL_FALSE;
     
		}

        void bind(int width, int height)
        {
            if (!valid) {
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            } else {
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,
                                     fboId);
            }

            glViewport(0, 0, width, height);
        }

        void bindRead(int width, int height)
        {
            if (!valid) {
                glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
            } else {
                glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT,
                                     fboId);
            }

            glViewport(0, 0, width, height);
        }

		void bindDraw(int width, int height)
        {
            if (!valid) {
                glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
            } else {
                glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,
                                     fboId);
            }

            glViewport(0, 0, width, height);
        }

		void unbind()
		{
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		}

		void destroy()
        {
            if (valid)
            {
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

            glDeleteFramebuffersEXT(1, &fboId);

            glDeleteRenderbuffersEXT(numRenderbuffers, renderbufferIds);

			valid = GL_FALSE;

			}
		}

		~CFBO()
        {
            destroy();
        }
		
//	private:
		GLuint fboId;

        // Max two renderbuffers per FBO:
        //   - One colorbuffer
        // - One [optional] depthbuffer
        GLuint renderbufferIds[2];

        GLint numRenderbuffers;
        GLboolean valid;

		(PFNGLBINDFRAMEBUFFEREXTPROC)glBindFramebufferEXT;
		(PFNGLBINDRENDERBUFFEREXTPROC)glBindRenderbufferEXT;
		(PFNGLDELETEFRAMEBUFFERSEXTPROC)glDeleteFramebuffersEXT;
		(PFNGLDELETERENDERBUFFERSEXTPROC)glDeleteRenderbuffersEXT;
		(PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)glFramebufferRenderbufferEXT;
		(PFNGLGENFRAMEBUFFERSEXTPROC)glGenFramebuffersEXT;
		(PFNGLGENRENDERBUFFERSEXTPROC)glGenRenderbuffersEXT;
		(PFNGLRENDERBUFFERSTORAGEEXTPROC)glRenderbufferStorageEXT;
		(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)glRenderbufferStorageMultisampleEXT;
		(PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)glFramebufferTexture2DEXT;
		(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)glCheckFramebufferStatusEXT;
		(PFNGLBLITFRAMEBUFFEREXTPROC) glBlitFramebufferEXT;
		(PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)glGetRenderbufferParameterivEXT;
};
#endif