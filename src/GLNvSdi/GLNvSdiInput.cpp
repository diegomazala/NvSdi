
#include "GLNvSdiInput.h"
#include "glExtensions.h"
#include "DVP.h"
#include "GLNvDvp.h"
#include <cinttypes>

extern "C"
{
	namespace attr
	{
		bool ownTextures = false;
		static gl::Texture2D inputTextures[MAX_VIDEO_STREAMS * 2];

		HGLRC inputOwnRC = NULL;
		HGLRC	inputGLRC = NULL;
		HDC		inputDC = NULL;
		HDC		inputAffinityDC = NULL;

		static CNvSDIin sdiIn;

		bool isCapturing = false;

		static unsigned int droppedFrames = 0;
		static unsigned int droppedFramesCount = 0;
		static unsigned int sequenceFrameNumber = 0;


		GLenum captureStatus = GL_FAILURE_NV;
		static GLuint64EXT captureTime;

	}


	///////////////////////////////////////////////////////////////////////
	/// Return the Sdi Output object
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API CNvSDIin* SdiInput()
	{
		return &attr::sdiIn;
	}

	GLNVSDI_API bool SdiInputMakeCurrent()
	{
		return wglMakeCurrent(attr::inputDC, attr::inputGLRC);
	}


	GLNVSDI_API bool SdiInputLoadExtensions()
	{
		if (!loadCaptureVideoExtension() || !loadBufferObjectExtension())
		{
			printf("Could not load the required OpenGL extensions\n");
			return false;
		}

		if (!loadPresentVideoExtension() || !loadFramebufferObjectExtension())
		{
			MessageBox(NULL, "Couldn't load required OpenGL extensions.", "Error", MB_OK);
			return false;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////
	/// Return video input width
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API int SdiInputWidth()
	{
		return attr::sdiIn.GetWidth();
	}


	///////////////////////////////////////////////////////////////////////
	/// Return video input height
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API int SdiInputHeight()
	{
		return attr::sdiIn.GetHeight();
	}



	///////////////////////////////////////////////////////////////////////
	/// Return the stream count
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API int SdiInputVideoCount()
	{
		return attr::sdiIn.GetNumStreams();
	}

	
	///////////////////////////////////////////////////////////////////////
	/// Return the time spent by gpu
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API float SdiInputGpuTime()
	{
		return attr::sdiIn.m_gpuTime;
	}


	///////////////////////////////////////////////////////////////////////
	/// Return the time spent by gvi
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API float SdiInputGviTime()
	{
		return attr::sdiIn.m_gviTime;
	}

	GLNVSDI_API unsigned int SdiInputFrameNumber()
	{
		return attr::sequenceFrameNumber;
	}

	///////////////////////////////////////////////////////////////////////
	/// Return the number of dropped frames in the last update
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API unsigned int SdiInputDroppedFrames()
	{
		return attr::droppedFrames;
	}


	///////////////////////////////////////////////////////////////////////
	/// Return the number of dropped frames since the application started
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API unsigned int SdiInputDroppedFramesCount()
	{
		return attr::droppedFramesCount;
	}


	///////////////////////////////////////////////////////////////////////
	/// Reset the number of dropped frames since the application started
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void SdiInputResetDroppedFramesCount()
	{
		attr::droppedFramesCount = 0;
	}

	

	///////////////////////////////////////////////////////////////////////
	/// Return the opengl handle for a video input texture
	///////////////////////////////////////////////////////////////////////
 	GLNVSDI_API gl::Texture2D* SdiInputGetTexture(int index)
 	{
 		return &attr::inputTextures[index];
 	}

	GLNVSDI_API GLuint SdiInputGetTextureId(int index)
	{
		return attr::inputTextures[index].Id();
	}

 	GLNVSDI_API GLenum SdiInputGetTextureType(int index)
 	{
 		return attr::inputTextures[index].Type();
 	}

	GLNVSDI_API void SdiInputSetTexture(int index, GLuint id)
	{
		attr::inputTextures[index].SetId(id);
	}

	GLNVSDI_API void SdiInputSetTexturePtr(int index, void* texturePtr, int w, int h)
	{
		GLuint gltex = (GLuint)(size_t)(texturePtr);
		attr::inputTextures[index].SetId(gltex);
	}


	/// Create textures for sdi input
	GLNVSDI_API bool SdiInputCreateTextures(int count, int width, int height)
	{
		if (count > MAX_VIDEO_STREAMS * 2)
		{
			std::cerr << "ERROR: The count of textures is higher than allowed: " << count << " > " << MAX_VIDEO_STREAMS * 2 << std::endl;
			return false;
		}

		int count_valid = 0;
		for(int i = 0; i < count; i++)
		{
			attr::inputTextures[i].Create();
			attr::inputTextures[i].Bind();
			attr::inputTextures[i].SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			attr::inputTextures[i].BuildNull(width, height);
			assert(glGetError() == GL_NO_ERROR);

			if (attr::inputTextures[i].Valid())
				count_valid++;
		}
		
		attr::ownTextures = (count_valid == count);
		return attr::ownTextures;
	}


	/// Destroy the textures used for sdi input
	GLNVSDI_API void SdiInputDestroyTextures()
	{
		if (attr::ownTextures)
		{
			for (int i=0; i<MAX_VIDEO_STREAMS * 2; ++i)
				SdiInputGetTexture(i)->Destroy();
			attr::ownTextures = false;
		}
	}




	GLNVSDI_API void SdiInputSetGlobalOptions()
	{
		//set the defaults for all the relevant options
		SdiGlobalOptions().sampling = NVVIOCOMPONENTSAMPLING_422;
		SdiGlobalOptions().dualLink = false;
		SdiGlobalOptions().bitsPerComponent = 8;
		SdiGlobalOptions().expansionEnable = true;
		SdiGlobalOptions().captureDevice = 0;
		SdiGlobalOptions().captureGPU = CNvGpuTopology::instance().getPrimaryGpuIndex();

		switch(SdiGlobalOptions().sampling)
		{
			case NVVIOCOMPONENTSAMPLING_422:				
				break;
			case NVVIOCOMPONENTSAMPLING_4224:		
			case NVVIOCOMPONENTSAMPLING_444:
			case NVVIOCOMPONENTSAMPLING_4444:
				SdiGlobalOptions().dualLink = true;
				break;
			default:				
				break;
		}

		if(SdiGlobalOptions().bitsPerComponent > 10)
			SdiGlobalOptions().dualLink = true;
	}

	GLNVSDI_API void SdiInputSetBufferSize(int ringBufferSizeInFrames)
	{
		SdiGlobalOptions().inputRingBufferSize = ringBufferSizeInFrames;
	}

	GLNVSDI_API void SdiInputSetCaptureFields(bool capture_fields)
	{
		SdiGlobalOptions().captureFields = capture_fields;
	}



#if 0
	///////////////////////////////////////////////////////////////////////
	/// Restart the NvSdi Input according to options set previously
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API bool SdiInputRestart()
	{
		SdiInputStop();
		SdiAncCleanupInput();
		SdiInputCleanupGL();
		SdiInputCleanupDevices();

		SdiMakeCurrent();

		return (SdiInputInitialize() &&
				SdiInputSetupDevices() &&
				SdiInputCreateTextures() &&
				SdiInputSetupGL() &&
				SdiAncSetupInput() &&
				SdiInputStart());
	}
#endif


	///////////////////////////////////////////////////////////////////////
	/// Initialize the NvSdi Input according to options set previously
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API bool SdiInputInitialize()
	{
		attr::droppedFrames = 0;
		attr::droppedFramesCount = 0;

		// Note, this function enumerates GPUs which are both CUDA & GLAffinity capable (i.e. newer Quadros)  
		int numGPUs = CNvGpuTopology::instance().getNumGpu(); 

		if(SdiGpuCount() <= 0)
		{
			SdiLog() << "Unable to obtain system GPU topology" << std::endl;
			return false;
		}

		if(SdiInputGpuCount() <= 0)
		{
			SdiLog() << "Unable to obtain system Capture topology" << std::endl;
			return false;
		}
		
		return true;
	}


	///////////////////////////////////////////////////////////////////////
	/// Uninitialize the sdi input pipeline
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void SdiInputUninitialize()
	{
		CNvGpuTopology::destroy();
		std::cout << "Dropped Frames Count: " << attr::droppedFramesCount << std::endl;
	}


	///////////////////////////////////////////////////////////////////////
	/// Configure the sdi devices for startup
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API bool SdiInputSetupDevices()
	{
		if(SdiGlobalOptions().captureDevice >= SdiInputGpuCount())		
		{
			SdiLog() << "Unable to obtain system Capture topology" << std::endl;
			return false;
		}

		if(SdiGlobalOptions().captureGPU >= SdiGpuCount())		
		{
			SdiLog() << "Selected Capture GPU is out of range" << std::endl;
			return false;
		}

		
		attr::sdiIn.SetRingBufferSize(SdiGlobalOptions().inputRingBufferSize);
		attr::sdiIn.Init(&SdiGlobalOptions());

		// Initialize the video capture device.
		if (attr::sdiIn.SetupDevice(true, SdiGlobalOptions().captureDevice) != S_OK)
		{
			SdiLog() << "Error setting up video capture." << std::endl;
			return false;
		}
	
		return true;
	}


	///////////////////////////////////////////////////////////////////////
	/// Empty
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void SdiInputCleanupDevices()
	{
		attr::sdiIn.Cleanup();
	}


	///////////////////////////////////////////////////////////////////////
	/// Setup opengl dependencies for sdi capture
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API bool SdiInputSetupContextGL()
	{
		HGPUNV  gpuMask[2];
		gpuMask[0] = CNvGpuTopology::instance().getGpu(SdiGlobalOptions().captureGPU)->getAffinityHandle();
		gpuMask[1] = NULL;

		if (SdiGetExternalDC() == NULL)
		{
			attr::inputAffinityDC = wglCreateAffinityDCNV(gpuMask);
			attr::inputDC = attr::inputAffinityDC;

			if (!attr::inputAffinityDC)
			{
				std::cerr << "Unable to create GPU affinity DC " << std::endl;
			}

			PIXELFORMATDESCRIPTOR  pfd;
			int  iPixelFormat;

			// get the current pixel format index  
			iPixelFormat = GetPixelFormat(attr::inputDC);

			// obtain a detailed description of that pixel format  
			DescribePixelFormat(attr::inputDC, iPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

			//int pf = ChoosePixelFormat(attr::hAffinityDC, &pfd);
			// Set pixel format.
			if (SetPixelFormat(attr::inputDC, iPixelFormat, &pfd) == FALSE)
			{
				MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK);
				return false;
			}
		}
		else
		{
			attr::inputDC = SdiGetExternalDC();
		}

		//Create affinity-rendering context from affinity-DC
		attr::inputGLRC = wglCreateContext(attr::inputDC);
		if (attr::inputGLRC == NULL)
		{
			printf("Unable to create GPU affinity RC\n");
		}
		else
		{
			attr::inputOwnRC = attr::inputGLRC;
		}

		if (!wglShareLists(SdiGetExternalGLRC(), attr::inputGLRC))
		{
			std::cerr << "Could not share gl contexts" << std::endl;
		}
		
		// Make window rendering context current.
		SdiInputMakeCurrent();

		return true;
	}



	///////////////////////////////////////////////////////////////////////
	/// Setup opengl dependencies for sdi capture
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API bool SdiInputSetupGL()
	{
		//load the required OpenGL extensions:
		if(!loadCaptureVideoExtension() || !loadTimerQueryExtension() || !loadBufferObjectExtension() || !loadSwapIntervalExtension())
		{
			SdiLog() << "Could not load the required OpenGL extensions" << std::endl;
			return false;
		}
	
		if (wglSwapIntervalEXT) 
		{
			wglSwapIntervalEXT(0);
		}

		// CSC parameters
		GLfloat mat[4][4];
		float scale = 1.0f;

		GLfloat max[] = {5000, 5000, 5000, 5000};;
		GLfloat min[] = {0, 0, 0, 0};

		GLfloat offset[] = {0, 0, 0, 0};

		if (1) 
		{
			mat[0][0] = 1.164f *scale;
			mat[0][1] = 1.164f *scale;
			mat[0][2] = 1.164f *scale;
			mat[0][3] = 0;  

			mat[1][0] = 0;
			mat[1][1] = -0.392f *scale;
			mat[1][2] = 2.017f *scale;
			mat[1][3] = 0;

			mat[2][0] = 1.596f *scale;
			mat[2][1] = -0.813f *scale;
			mat[2][2] = 0.f;
			mat[2][3] = 0;

			mat[3][0] = 0;
			mat[3][1] = 0;
			mat[3][2] = 0;
			mat[3][3] = 1;

			offset[0] =-0.87f;
			offset[1] = 0.53026f;
			offset[2] = -1.08f;
			offset[3] = 0;
		}

		attr::sdiIn.SetCSCParams(&mat[0][0], offset, min, max);
		
		return true;
	}


	/// Bind textures and device for sdi input
	GLNVSDI_API bool SdiInputBindVideoTextureFrame()
	{
		GLuint gpuVideoSlot = 1;
		if (attr::sdiIn.BindDevice(gpuVideoSlot, attr::inputDC) == E_FAIL)
			return false;

		for (unsigned int i = 0; i < attr::sdiIn.GetNumStreams(); i++)
		{
			if (attr::sdiIn.BindVideoTexture(attr::inputTextures[i].Id(), i, attr::inputTextures[i].Type(), GL_FRAME_NV)  == E_FAIL)
				return false;
		}
		
		return true;
	}

	/// Unbind textures and device for sdi input
	GLNVSDI_API bool SdiInputUnbindVideoTextureFrame()
	{
		for(unsigned int i = 0; i < SdiInput()->GetNumStreams(); i++)
		{
			if (SdiInput()->UnbindVideoTexture(i, attr::inputTextures[i].Type(), GL_FRAME_NV) == E_FAIL)
				return false;
		}

		if (SdiInput()->UnbindDevice() == E_FAIL)
			return false;

		return true;
	}

	/// Bind textures and device for sdi input
	GLNVSDI_API bool SdiInputBindVideoTextureField()
	{
		GLuint gpuVideoSlot = 1;
		
		if (attr::sdiIn.BindDevice(gpuVideoSlot, attr::inputDC) == E_FAIL)
			return false;

		for (unsigned int i = 0; i < attr::sdiIn.GetNumStreams(); i++)
		{
			if (attr::sdiIn.BindVideoTexture(attr::inputTextures[i * 2 + 0].Id(), i, attr::inputTextures[i * 2 + 0].Type(), GL_FIELD_UPPER_NV) == E_FAIL)
				return false;
			if (attr::sdiIn.BindVideoTexture(attr::inputTextures[i * 2 + 1].Id(), i, attr::inputTextures[i * 2 + 1].Type(), GL_FIELD_LOWER_NV) == E_FAIL)
				return false;
		}

		return true;
	}

	/// Unbind textures and device for sdi input
	GLNVSDI_API bool SdiInputUnbindVideoTextureField()
	{
		for (unsigned int i = 0; i < SdiInput()->GetNumStreams(); i++)
		{
			if (attr::sdiIn.UnbindVideoTexture(i, attr::inputTextures[i * 2 + 0].Type(), GL_FIELD_UPPER_NV) == E_FAIL)
				return false;
			if (attr::sdiIn.UnbindVideoTexture(i, attr::inputTextures[i * 2 + 1].Type(), GL_FIELD_LOWER_NV) == E_FAIL)
				return false;
		}

		if (SdiInput()->UnbindDevice() == E_FAIL)
			return false;

		return true;
	}

	///////////////////////////////////////////////////////////////////////
	/// Cleanup the opengl stuff used in sdi capture
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void SdiInputCleanupGL()
	{
		SdiInputDestroyTextures();

		if (attr::inputGLRC != NULL && attr::inputGLRC == attr::inputOwnRC) 
		{
			// Delete OpenGL rendering context.
			wglMakeCurrent(NULL,NULL); 

			wglDeleteContext(attr::inputGLRC);
			attr::inputOwnRC = NULL;
			attr::inputGLRC = NULL;

			wglDeleteDCNV(attr::inputAffinityDC);
		}		
		
	}

	GLNVSDI_API bool SdiInputIsCapturing()
	{
		return attr::isCapturing;
	}


	///////////////////////////////////////////////////////////////////////
	/// Start the sdi capture
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API bool SdiInputStart()
	{
		if (attr::isCapturing)
		{
			SdiInputStop();
		}
			
		// Start video capture
		if(attr::sdiIn.StartCapture()!= S_OK)
		{
			attr::isCapturing = false;
			SdiLog() << "Error starting video capture." << std::endl;
			return false;
		}

		return attr::isCapturing = true;
	}


	///////////////////////////////////////////////////////////////////////
	/// Stop the sdi capture
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void SdiInputStop()
	{
		if (attr::isCapturing) 
		{
			attr::sdiIn.EndCapture();
			attr::isCapturing = false;
		}
	}

	GLNVSDI_API int SdiInputCaptureStatus()
	{
		return attr::captureStatus;
	}

	GLNVSDI_API uint64_t SdiInputCaptureTime()
	{
		return attr::captureTime;
	}

	GLNVSDI_API float SdiInputFrameRate()
	{
		float frameRate = 0;
		attr::sdiIn.GetFrameRate(&frameRate);
		return frameRate;
	}

	GLNVSDI_API double SdiInputFrameRateNanoSec()
	{
		return 1000000000.0 / SdiInputFrameRate();
	}

	///////////////////////////////////////////////////////////////////////
	/// Capture the current frame for all sdi inputs available
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API GLenum SdiInputCaptureVideo()
	{
		
		static GLuint prevSequenceNum = 0;
		attr::captureStatus = GL_FAILURE_NV;
		static int numFails = 0;
		static int numTries = 0;
		static bool bShowMessageBox = true;  
		
#if 0
		if(numFails < 100)
		{
#endif
			// Capture the video to a buffer object
			attr::captureStatus = attr::sdiIn.Capture(&attr::sequenceFrameNumber, &attr::captureTime);
			attr::droppedFrames = attr::sequenceFrameNumber - prevSequenceNum - 1;
			attr::droppedFramesCount += attr::droppedFrames;
			//if(sequenceNum - prevSequenceNum > 1)
			//{
			//	std::cout << "Frame: " << sequenceNum << " Dropped: " << attr::droppedFrames << std::endl;
			//}
			
			prevSequenceNum = attr::sequenceFrameNumber;
			switch (attr::captureStatus)
			{
				case GL_SUCCESS_NV:
					//std::cout << "Frame: " << sequenceNum << " gpuTime: " << attr::sdiIn.m_gpuTime << " gviTime: " << attr::sdiIn.m_gviTime << std::endl;
					numFails = 0;
					break;

				case GL_PARTIAL_SUCCESS_NV:
					//std::cout << "glVideoCaptureNV: GL_PARTIAL_SUCCESS_NV" << std::endl;
					numFails = 0;
					break;

				case GL_FAILURE_NV:
					//std::cout << "glVideoCaptureNV: GL_FAILURE_NV - Video capture failed." << std::endl;
					numFails++;
					break;

				default:
					//std::cout << "glVideoCaptureNV: Unknown return value." << std::endl;
					break;
			} // switch
#if 0
		}
#endif
#if 0
		else					// The incoming signal format or some other error occurred during
		{						// capture, shutdown and try to restart capture.
			if(numTries == 0)
			{		
				SdiInputStop();
				SdiInputCleanupGL();
				SdiInputCleanupDevices();			
			}
			// Initialize the video capture device.
			if (attr::sdiIn.SetupDevice(bShowMessageBox, SdiGlobalOptions().captureDevice) != S_OK)
			{
				bShowMessageBox = false;
				numTries++;
				return GL_FAILURE_NV;
			}
			// Reinitialize OpenGL.
			SdiInputSetupContextGL(SdiGetDC(), NULL);		
			SdiInputSetupGL();		
			SdiInputStart();
			numFails = 0;
			numTries = 0;

			bShowMessageBox = true;
			return GL_FAILURE_NV;
		}
#endif
		return attr::captureStatus;
	}




	static void UNITY_INTERFACE_API OnSdiInputRenderEvent(int render_event_id)
	{
		switch (static_cast<SdiRenderEvent>(render_event_id))
		{
			case SdiRenderEvent::CaptureFrame:
			{
				SdiInputMakeCurrent();

				if (SdiInputCaptureVideo() != GL_FAILURE_NV)
				{
					SdiAncCapture();
				}
				else
				{
					//Debug.LogError("Capture fail");
				}

				SdiMakeCurrentExternal();

				sdiError = (int)glGetError();

				break;
			}


			case SdiRenderEvent::Initialize:
			{
				SdiSetCurrentDC();		// get external DC
				SdiSetCurrentGLRC();	// get external GLRC

				if (!SdiInputSetupContextGL())
				{
					std::cerr << "ERROR: Could not setup gl context for input sdi. " << std::endl;
					return;
				}
				
				if (!SdiInputLoadExtensions())
				{
					std::cerr << "ERROR: Could not load gl extensions. " << std::endl;
					return;
				}

				if (!SdiInputInitialize())
				{
					SdiLog() << "SdiInputInitialize failed" << std::endl;
					sdiError = (int)glGetError();
					return;
				}
				
				SdiInputMakeCurrent();

				SdiInputSetGlobalOptions();

				if (!SdiInputSetupDevices())
				{
					SdiLog() << "SdiInputSetupDevices failed" << std::endl;
					sdiError = (int)glGetError();
					return;
				}

				if (!SdiInputMakeCurrent())
				{
					SdiLog() << "SdiMakeCurrent failed" << std::endl;
					sdiError = (int)glGetError();
					return;
				}

				sdiError = (int)glGetError();

				SdiMakeCurrentExternal();

				break;
			}

			case SdiRenderEvent::Setup:
			{
				sdiError = (int)glGetError();


				if (!SdiInputMakeCurrent())
				{
					SdiLog() << "SdiMakeCurrent failed" << std::endl;
					sdiError = (int)glGetError();
					return;
				}

				// Check if textures have already been created
				if (!attr::inputTextures[0].Id() > 0)
				{
					if (SdiGlobalOptions().captureFields)
					{
						if (!SdiInputCreateTextures(MAX_VIDEO_STREAMS * 2, SdiInputWidth(), SdiInputHeight() / 2))
						{
							std::cerr << "ERROR: Could not creat opengl textures for SDI input. Abort. " << std::endl;
							return;
						}
					}
					else
					{
						if (!SdiInputCreateTextures(MAX_VIDEO_STREAMS, SdiInputWidth(), SdiInputHeight()))
						{
							std::cerr << "ERROR: Could not creat opengl textures for SDI input. Abort. " << std::endl;
							return;
						}
					}
				}




				if (!SdiInputSetupGL())
				{
					//UnityEngine.Debug.LogError("GLNvSdi_Plugin: " + UtyGLNvSdi.SdiGetLog());
					//return false;
				}

				if (SdiGlobalOptions().captureFields)
				{
					if (!SdiInputBindVideoTextureField())
					{
						//UnityEngine.Debug.LogError("GLNvSdi_Plugin: " + UtyGLNvSdi.SdiGetLog());
						//return false;
					}
				}
				else
				{
					if (!SdiInputBindVideoTextureFrame())
					{
						//UnityEngine.Debug.LogError("GLNvSdi_Plugin: " + UtyGLNvSdi.SdiGetLog());
						//return false;
					}
				}

				if (!SdiAncSetupInput())
				{
					//UnityEngine.Debug.LogError("GLNvSdi_Plugin: " + UtyGLNvSdi.SdiGetLog());
					//return false;
				}

				sdiError = (int)glGetError();

				SdiMakeCurrentExternal();

				break;
			}

			case SdiRenderEvent::StartCapture:
			{
				if (!SdiInputMakeCurrent())
				{
					SdiLog() << "SdiMakeCurrent failed" << std::endl;
					sdiError = (int)glGetError();
					return;
				}


				if (!SdiInputStart())
				{
					//UnityEngine.Debug.LogError("GLNvSdi_Plugin: " + UtyGLNvSdi.SdiGetLog());
					//return false;
				}
				sdiError = (int)glGetError();

				SdiInputResetDroppedFramesCount();

				SdiMakeCurrentExternal();

				break;
			}

			case SdiRenderEvent::StopCapture:
			{
				SdiInputMakeCurrent();
				SdiInputStop();
				SdiAncCleanupInput();
				sdiError = (int)glGetError();
				break;
			}

			case SdiRenderEvent::Shutdown:
			{
				HGLRC ext_hglrc = wglGetCurrentContext();
				HDC ext_hdc = wglGetCurrentDC();

				SdiInputMakeCurrent();

				SdiInputStop();
				SdiAncCleanupInput();

				if (attr::sdiIn.IsInterlaced())
					SdiInputUnbindVideoTextureField();
				else
					SdiInputUnbindVideoTextureFrame();

				SdiInputCleanupGL();
				SdiInputUninitialize();

				sdiError = (int)glGetError();
				
				wglMakeCurrent(ext_hdc, ext_hglrc);

				break;
			}
		}
	}

	// --------------------------------------------------------------------------
	// GetRenderEventFunc, an example function we export which is used to get a rendering event callback function.
	UnityRenderingEvent GLNVSDI_API UNITY_INTERFACE_API GetSdiInputRenderEventFunc()
	{
		return OnSdiInputRenderEvent;
	}


};	//extern "C"
