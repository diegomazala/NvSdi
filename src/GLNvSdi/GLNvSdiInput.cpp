
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

		static CNvSDIin sdiIn;

		bool isCapturing = false;

		int droppedFrames = 0;
		int droppedFramesCount = 0;

		GLenum captureStatus = GL_FAILURE_NV;

		static C_DVP		dvp;
		static C_Frame*		framePtr[NVAPI_MAX_VIO_DEVICES] = { nullptr };
		static C_Frame*		prevFramePtr[NVAPI_MAX_VIO_DEVICES] = { nullptr };
		static GLuint		numDroppedFrames[NVAPI_MAX_VIO_DEVICES];
		static GLuint		drawTimeQuery;
		static GLuint64EXT	drawTimeStart;
		static GLuint64EXT	drawTimeEnd;
		static GLuint64EXT	timeElapsed;
		static bool			dvpOk;
		static bool			ownDisplayTextures = false;
		
		static gl::TextureBlit texBlit;
		//
		static gl::Texture2D displayTextures[NVAPI_MAX_VIO_DEVICES][MAX_VIDEO_STREAMS];
		//
		static gl::TextureRectNV decodeTextures[NVAPI_MAX_VIO_DEVICES][MAX_VIDEO_STREAMS];
		// The raw SDI data from the captured buffer
		// needs to be copied to a texture, so that
		// the data can be read by a shader and
		// processed for display in OpenGL. For 
		// example the shader may do 422->444 expansion	
	}


	///////////////////////////////////////////////////////////////////////
	/// Return the Sdi Output object
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API CNvSDIin* SdiInput()
	{
		return &attr::sdiIn;
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

	///////////////////////////////////////////////////////////////////////
	/// Return the number of dropped frames in the last update
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API int SdiInputDroppedFrames()
	{
		return attr::droppedFrames;
	}


	///////////////////////////////////////////////////////////////////////
	/// Return the number of dropped frames since the application started
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API int SdiInputDroppedFramesCount()
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




	GLNVSDI_API void SdiInputSetGlobalOptions(int ringBufferSizeInFrames)
	{
		//set the defaults for all the relevant options
		SdiGlobalOptions().sampling = NVVIOCOMPONENTSAMPLING_422;
		SdiGlobalOptions().dualLink = false;
		SdiGlobalOptions().bitsPerComponent = 8;
		SdiGlobalOptions().expansionEnable = true;
		SdiGlobalOptions().captureDevice = 0;
		SdiGlobalOptions().captureGPU = CNvGpuTopology::instance().getPrimaryGpuIndex();

		SdiGlobalOptions().inputRingBufferSize = ringBufferSizeInFrames;

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
	GLNVSDI_API bool SdiInputSetupContextGL(HDC hDC, HGLRC hRC)
	{
		if (hRC == NULL)
		{
			hRC = SdiGetGLRC();
		}

		SdiSetDC(hDC);
		SdiSetGLRC(hRC);

		if (SdiGetGLRC() == NULL)
		{
			HGPUNV  gpuMask[2];
			gpuMask[0] = CNvGpuTopology::instance().getGpu(SdiGlobalOptions().captureGPU)->getAffinityHandle();
			gpuMask[1] = NULL;

			if (hDC == NULL)
			{
				SdiSetAffinityDC(wglCreateAffinityDCNV(gpuMask));
				SdiSetDC(SdiGetAffinityDC());
				if (!SdiGetAffinityDC()) 
				{
					std::cerr << "Unable to create GPU affinity DC " << std::endl;
				}

				PIXELFORMATDESCRIPTOR  pfd;
				int  iPixelFormat;

				// get the current pixel format index  
				iPixelFormat = GetPixelFormat(SdiGetDC()); 

				// obtain a detailed description of that pixel format  
				DescribePixelFormat(SdiGetDC(), iPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

				//int pf = ChoosePixelFormat(attr::hAffinityDC, &pfd);
				// Set pixel format.
				if (SetPixelFormat(SdiGetDC(), iPixelFormat, &pfd) == FALSE) 
				{ 
					MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK); 
					return false; 
				}
			}

			//Create affinity-rendering context from affinity-DC
			SdiSetGLRC(wglCreateContext(SdiGetDC()));
			if (SdiGetGLRC() == NULL)
			{
				printf("Unable to create GPU affinity RC\n");
			}
			else
			{
				attr::inputOwnRC = SdiGetGLRC();
			}
		}
		
		// Make window rendering context current.
		SdiMakeCurrent();

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
		if (attr::sdiIn.BindDevice(gpuVideoSlot, SdiGetDC()) == E_FAIL) 
			return false;

		for(int i = 0; i < attr::sdiIn.GetNumStreams(); i++)
		{
			if (attr::sdiIn.BindVideoTexture(attr::inputTextures[i].Id(), i, attr::inputTextures[i].Type(), GL_FRAME_NV)  == E_FAIL)
				return false;
		}
		
		return true;
	}

	/// Unbind textures and device for sdi input
	GLNVSDI_API bool SdiInputUnbindVideoTextureFrame()
	{
		for(int i = 0; i < SdiInput()->GetNumStreams(); i++)
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
		
		if (attr::sdiIn.BindDevice(gpuVideoSlot, SdiGetDC()) == E_FAIL)
			return false;

		for(int i = 0; i < attr::sdiIn.GetNumStreams(); i++)
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
		for(int i = 0; i < SdiInput()->GetNumStreams(); i++)
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

		if (SdiGetGLRC() != NULL && SdiGetGLRC() == attr::inputOwnRC) 
		{
			// Delete OpenGL rendering context.
			wglMakeCurrent(NULL,NULL); 

			wglDeleteContext(SdiGetGLRC());
			attr::inputOwnRC = NULL;
			SdiSetGLRC(NULL);

			wglDeleteDCNV(SdiGetAffinityDC());
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

	///////////////////////////////////////////////////////////////////////
	/// Capture the current frame for all sdi inputs available
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API GLenum SdiInputCaptureVideo()
	{
		static GLint64EXT captureTime;
		GLuint sequenceNum;    
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
			attr::captureStatus = attr::sdiIn.Capture(&sequenceNum, &captureTime);
			attr::droppedFrames = sequenceNum - prevSequenceNum - 1;
			attr::droppedFramesCount += attr::droppedFrames;
			if(sequenceNum - prevSequenceNum > 1)
			{
				std::cout << "Frame: " << sequenceNum << " Dropped: " << attr::droppedFrames << std::endl;
			}
			
			prevSequenceNum = sequenceNum;
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
				if (SdiInputCaptureVideo() != GL_FAILURE_NV)
				{
					SdiAncCapture();
				}
				else
				{
					//Debug.LogError("Capture fail");
				}

				sdiError = (int)glGetError();

				break;
			}


			case SdiRenderEvent::Initialize:
			{

				//SdiSetupLogFile();
				SdiSetCurrentDC();
				SdiSetCurrentGLRC();

				if (!SdiInputInitialize())
				{
					SdiLog() << "SdiInputInitialize failed" << std::endl;
					sdiError = (int)glGetError();
					return;
				}

				const int ringBufferSizeInFrames = 2;
				SdiInputSetGlobalOptions(ringBufferSizeInFrames);

				if (!SdiInputSetupDevices())
				{
					SdiLog() << "SdiInputSetupDevices failed" << std::endl;
					sdiError = (int)glGetError();
					return;
				}

				if (!SdiMakeCurrent())
				{
					SdiLog() << "SdiMakeCurrent failed" << std::endl;
					sdiError = (int)glGetError();
					return;
				}

				sdiError = (int)glGetError();

				break;
			}

			case SdiRenderEvent::Setup:
			{
				sdiError = (int)glGetError();

				if (!SdiInputSetupGL())
				{
					//UnityEngine.Debug.LogError("GLNvSdi_Plugin: " + UtyGLNvSdi.SdiGetLog());
					//return false;
				}

				if (attr::sdiIn.IsInterlaced())
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

				break;
			}

			case SdiRenderEvent::StartCapture:
			{
				if (!SdiInputStart())
				{
					//UnityEngine.Debug.LogError("GLNvSdi_Plugin: " + UtyGLNvSdi.SdiGetLog());
					//return false;
				}
				sdiError = (int)glGetError();

				SdiInputResetDroppedFramesCount();
				break;
			}

			case SdiRenderEvent::StopCapture:
			{
				SdiMakeCurrent();
				SdiInputStop();
				SdiAncCleanupInput();
				sdiError = (int)glGetError();
				break;
			}

			case SdiRenderEvent::Shutdown:
			{
				HGLRC uty_hglrc = wglGetCurrentContext();
				HDC uty_hdc = wglGetCurrentDC();
				SdiMakeCurrent();

				SdiInputStop();
				SdiAncCleanupInput();

				if (attr::sdiIn.IsInterlaced())
					SdiInputUnbindVideoTextureField();
				else
					SdiInputUnbindVideoTextureFrame();

				SdiInputCleanupGL();
				SdiInputUninitialize();

				sdiError = (int)glGetError();
				wglMakeCurrent(uty_hdc, uty_hglrc);

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


	GLNVSDI_API bool DvpIsOk()
	{
		return attr::dvpOk;
	}

	//GLNVSDI_API bool DvpCheckAvailability()
	//{
	//	attr::dvpOk = false;

	//	int numGPUs;
	//	// Note, this function enumerates GPUs which are both CUDA & GLAffinity capable (i.e. newer Quadros)  
	//	numGPUs = CNvGpuTopology::instance().getNumGpu();

	//	if (numGPUs <= 0)
	//	{
	//		MessageBox(NULL, "Unable to obtain system GPU topology", "Error", MB_OK);
	//		return false;
	//	}

	//	int numCaptureDevices = CNvSDIinTopology::instance().getNumDevice();

	//	if (numCaptureDevices <= 0)
	//	{
	//		MessageBox(NULL, "Unable to obtain system Capture topology", "Error", MB_OK);
	//		return false;
	//	}


	//	if (attr::dvp.m_options.captureGPU >= numGPUs)
	//	{
	//		MessageBox(NULL, "Selected GPU is out of range", "Error", MB_OK);
	//		return false;
	//	}

	//	attr::dvpOk = true;
	//	return true;
	//}

	GLNVSDI_API bool DvpPreSetup()
	{
		//load the required OpenGL extensions:
		if (!loadTimerQueryExtension() ||
			!loadBufferObjectExtension() ||
			!loadShaderObjectsExtension() ||
			!loadFramebufferObjectExtension() ||
			!loadCopyImageExtension() ||
			!loadSwapIntervalExtension())
		{
			std::cout << "Could not load the required OpenGL extensions" << std::endl;
			return false;
		}

		if (wglSwapIntervalEXT)
			wglSwapIntervalEXT(0);


		if (attr::dvp.SetupSDIPipeline() != S_OK)
			return false;

		return (glGetError() == GL_NO_ERROR);
	}

	GLNVSDI_API bool DvpSetup()
	{
		int videoWidth = attr::dvp.GetVideoWidth();
		int videoHeight = attr::dvp.GetVideoHeight();

		// check if DvpPreSetup has been called previously
		if (videoWidth < 1 || videoHeight < 1)
			return false;

		attr::dvp.SetupSDIinGL(SdiGetDC(), SdiGetGLRC());



		SdiMakeCurrent();

		assert(glGetError() == GL_NO_ERROR);

		// To view the buffers we need to load an appropriate shader
		attr::dvp.SetupDecodeProgram();
		int activeDeviceCount = attr::dvp.GetActiveDeviceCount();
		if (activeDeviceCount == 0)
			return false;


		//wglMakeCurrent(attr::dvp.m_hCaptureDC, attr::dvp.m_hCaptureRC);

		//GLuint gpuVideoSlot = 1;
		//for (int i = 0; i < activeDeviceCount; i++)
		//{
		//	//m_SDIin[i].BindDevice(gpuVideoSlot++, m_hCaptureDC);


		//	for (int j = 0; j < attr::dvp.m_SDIin[i].GetNumStreams(); j++)
		//	{
		//		glEnable(attr::displayTextures[i][j].Type());
		//		attr::displayTextures[i][j].Bind();
		//		if (attr::dvp.m_SDIin[i].BindVideoTexture(attr::displayTextures[i][j + 0].Id(), j, attr::displayTextures[i][j].Type(), GL_FIELD_UPPER_NV) == E_FAIL)
		//			return false;
		//		if (attr::dvp.m_SDIin[i].BindVideoTexture(attr::displayTextures[i][j + 1].Id(), j, attr::displayTextures[i][j].Type(), GL_FIELD_LOWER_NV) == E_FAIL)
		//			return false;
		//	}
		//}


		//SdiMakeCurrent();

		//assert(glGetError() == GL_NO_ERROR);

		//
		// Check if the textures have been created
		//
		if (attr::displayTextures[0][0].Id() < 1)
		{
			//allocate the textures for display
			if (!DvpInputCreateDisplayTextures(videoWidth, videoHeight))
				return false;
		}


		//create the textures to go with the buffers and frame buffer objects to create the display textures
		for (UINT d = 0; d < activeDeviceCount; d++)
		{
			int numStreams = attr::dvp.GetNumStreamsPerFrame(d);
#ifdef USE_ALL_STREAMS	
			numStreams = NUM_VIDEO_STREAMS;
#endif		        

			glGenFramebuffersEXT(numStreams, &attr::dvp.m_vidFbos[d][0]);
			assert(glGetError() == GL_NO_ERROR);

			for (unsigned int s = 0; s < numStreams; s++)
			{

				attr::decodeTextures[d][s].Create();
				attr::decodeTextures[d][s].Bind();
				attr::decodeTextures[d][s].SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				attr::decodeTextures[d][s].SetParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				
				// Allocate storage for the decode texture.
				glTexImage2D(attr::decodeTextures[d][s].Type(), 
					0, GL_RGBA8UI,
					(GLsizei)(videoWidth*0.5), videoHeight, 
					0, GL_RGBA_INTEGER_EXT, 
					GL_UNSIGNED_BYTE, NULL);

				assert(glGetError() == GL_NO_ERROR);

				// Configure the decode->output FBO.
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, attr::dvp.m_vidFbos[d][s]);
				assert(glGetError() == GL_NO_ERROR);
				
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					GL_COLOR_ATTACHMENT0_EXT,
					attr::displayTextures[d][s].Type(),
					attr::displayTextures[d][s].Id(),
					0);
				assert(glGetError() == GL_NO_ERROR);


				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			}

		}

		glGenQueries(1, &attr::drawTimeQuery);

		attr::texBlit.CreateVbo();

		return (glGetError() == GL_NO_ERROR);
	}

	GLNVSDI_API bool DvpCleanup()
	{
		attr::dvp.CleanupSDIPipeline();

		attr::dvp.CleanupSDIinGL();

		SdiMakeCurrent();

		int activeDeviceCount = attr::dvp.GetActiveDeviceCount();
		if (activeDeviceCount == 0)
			return false;

		for (UINT i = 0; i < activeDeviceCount; i++)
		{
			int numStreams = attr::dvp.GetNumStreamsPerFrame(i);
#ifdef USE_ALL_STREAMS		
			numStreams = NUM_VIDEO_STREAMS;
#endif		  
			for (int j = 0; j < numStreams; ++j)
			{
				attr::decodeTextures[i][j].Destroy();

				if (attr::ownDisplayTextures)
					attr::displayTextures[i][j].Destroy();
			}

			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			glDeleteFramebuffersEXT(numStreams, &attr::dvp.m_vidFbos[i][0]);
		}
		attr::ownDisplayTextures = false;
		attr::dvp.DestroyDecodeProgram();

		glDeleteQueries(1, &attr::drawTimeQuery);


		attr::texBlit.DestroyVbo();

		// Delete OpenGL rendering context.
		wglMakeCurrent(NULL, NULL);

		if (SdiGetGLRC())
		{
			//wglDeleteContext(SdiGetGLRC());
			SdiSetGLRC(NULL);
		}

		//ReleaseDC(this->hWnd, SdiGetDC());

		if (SdiGetAffinityDC() != nullptr)
			wglDeleteDCNV(SdiGetAffinityDC());

		if (SdiGetDC() != nullptr)
			SdiSetDC(nullptr);


		for (int i = 0; i < NVAPI_MAX_VIO_DEVICES; ++i)
		{
			attr::framePtr[i] = nullptr;
			attr::prevFramePtr[i] = nullptr;
		}


		return (glGetError() == GL_NO_ERROR);
	}

	GLNVSDI_API bool DvpStart()
	{
		return (attr::dvp.StartSDIPipeline() == S_OK);
	}

	GLNVSDI_API bool DvpStop()
	{
		return (attr::dvp.StopSDIPipeline() == S_OK);
	}

	GLNVSDI_API C_Frame* DvpUpdateFrame(int device_index)
	{
		attr::framePtr[device_index] = attr::dvp.GetFrame(device_index);

		if (attr::framePtr[device_index] != nullptr)
		{
			if (attr::prevFramePtr[device_index] != nullptr)
			{
				attr::numDroppedFrames[device_index] +=
					attr::framePtr[device_index]->sequenceNum - attr::prevFramePtr[device_index]->sequenceNum - 1;
				
				attr::dvp.ReleaseUsedFrame(device_index, attr::prevFramePtr[device_index]);
			}
			attr::prevFramePtr[device_index] = attr::framePtr[device_index];
		}
		else
		{
			if (attr::prevFramePtr[device_index] != nullptr)
				attr::framePtr[device_index] = attr::prevFramePtr[device_index];
		}

		return attr::framePtr[device_index];
	}

	GLNVSDI_API C_Frame* DvpFrame(int device_index)
	{
		return attr::framePtr[device_index];
	}

	GLNVSDI_API C_Frame* DvpPreviousFrame(int device_index)
	{
		return attr::prevFramePtr[device_index];
	}

	GLNVSDI_API GLuint64EXT DvpNumDroppedFrames(int device_index)
	{
		return attr::numDroppedFrames[device_index];
	}

	GLNVSDI_API int DvpActiveDeviceCount()
	{
		int activeDevices = attr::dvp.GetActiveDeviceCount();
		return attr::dvp.GetActiveDeviceCount();
	}

	GLNVSDI_API int DvpDeviceId(int device_index)
	{
		return attr::dvp.GetDeviceNumber(device_index);
	}

	GLNVSDI_API int DvpStreamsPerFrame(int device_index)
	{
		#ifdef USE_ALL_STREAMS
			return NUM_VIDEO_STREAMS;
		#else
			return attr::dvp.GetNumStreamsPerFrame(device_index);
		#endif	
	}
	
	GLNVSDI_API NVVIOSIGNALFORMAT DvpSignalFormat()
	{
		return attr::dvp.GetSignalFormat();
	}

	GLNVSDI_API C_DVP* DvpPtr()
	{
		return &attr::dvp;
	}

	GLNVSDI_API int DvpWidth()
	{
		return attr::dvp.GetVideoWidth();
	}

	GLNVSDI_API int DvpHeight()
	{
		return attr::dvp.GetVideoHeight();
	}

	GLNVSDI_API void DvpBeginTimeQuery()
	{
		assert(glGetError() == GL_NO_ERROR);
		glBeginQuery(GL_TIME_ELAPSED_EXT, attr::drawTimeQuery);
		assert(glGetError() == GL_NO_ERROR);
		glGetInteger64v(GL_CURRENT_TIME_NV, (GLint64 *)&attr::drawTimeStart);
	}

	GLNVSDI_API void DvpEndTimeQuery()
	{
		assert(glGetError() == GL_NO_ERROR);
		glEndQuery(GL_TIME_ELAPSED_EXT);
		assert(glGetError() == GL_NO_ERROR);
		glGetQueryObjectui64vEXT(attr::drawTimeQuery, GL_QUERY_RESULT, &attr::timeElapsed);
		glGetInteger64v(GL_CURRENT_TIME_NV, (GLint64 *)&attr::drawTimeEnd);
	}

	GLNVSDI_API GLuint64EXT DvpGpuTimeElapsed()
	{
		return attr::timeElapsed;
	}

	GLNVSDI_API GLuint64EXT DvpCpuTimeElapsed()
	{
		return attr::drawTimeEnd - attr::drawTimeStart;
	}
	
	GLNVSDI_API GLuint64EXT DvpFrameNumber(int device_index)
	{
		//return (attr::dvp.GetFrame(device_index) != nullptr)
		//	? attr::dvp.GetFrame(device_index)->sequenceNum
		//	: 0;
		return (attr::framePtr[device_index] != nullptr)
			? attr::framePtr[device_index]->sequenceNum
			: 0;
	}

	GLNVSDI_API GLuint64EXT DvpDroppedFrames(int device_index)
	{
		//return (attr::dvp.GetFrame(device_index) != nullptr)
		//	? attr::dvp.GetFrame(device_index)->numDroppedFrames
		//	: UINT64_MAX;
		return (attr::framePtr[device_index] != nullptr)
			? attr::framePtr[device_index]->numDroppedFrames
			: UINT64_MAX;
	}

	GLNVSDI_API float DvpCaptureElapsedTime(int device_index)
	{
		return (attr::framePtr[device_index] != nullptr)
			? attr::framePtr[device_index]->captureElapsedTime
			: UINT32_MAX;
	}

	GLNVSDI_API float DvpUploadElapsedTime(int device_index)
	{
		return (attr::framePtr[device_index] != nullptr)
			? attr::framePtr[device_index]->uploadElapsedTime
			: UINT32_MAX;
	}

	GLNVSDI_API float DvpDownloadElapsedTime(int device_index)
	{
		return (attr::framePtr[device_index] != nullptr)
			? attr::framePtr[device_index]->downloadElapsedTime
			: UINT32_MAX;
	}

	GLNVSDI_API void DvpReleaseFrame(C_Frame* used_frame, int device_index)
	{
		attr::dvp.ReleaseUsedFrame(device_index, used_frame);
	}

	GLNVSDI_API GLuint DvpPixelBufferId(int device_index, int video_stream_index)
	{
		return (attr::framePtr[device_index] != nullptr)
			? attr::framePtr[device_index]->getDstObject(video_stream_index)
			: UINT_MAX;
	}

	GLNVSDI_API GLuint DvpDecodeTextureId(int device_index, int video_stream_index)
	{
		return attr::decodeTextures[device_index][video_stream_index].Id();
	}

	GLNVSDI_API GLuint DvpDisplayTextureId(int device_index, int video_stream_index)
	{
		return attr::displayTextures[device_index][video_stream_index].Id();
	}

	GLNVSDI_API gl::Texture2D* DvpDisplayTexture(int device_index, int video_stream_index)
	{
		return &attr::displayTextures[device_index][video_stream_index];
	}

	GLNVSDI_API void DvpSetDisplayTexture(int target_texture_id, int target_texture_type, int device_index, int video_stream_index)
	{
		attr::displayTextures[device_index][video_stream_index].SetId(target_texture_id);
		attr::displayTextures[device_index][video_stream_index].SetType(target_texture_type);
	}
	GLNVSDI_API void DvpSetDisplayTexturePtr(void* texturePtr, int device_index, int video_stream_index)
	{
		GLuint gltex = (GLuint)(size_t)(texturePtr);
		DvpSetDisplayTexture(gltex, GL_TEXTURE_2D, device_index, video_stream_index);
	}

	GLNVSDI_API bool DvpInputCreateDisplayTextures(int videoWidth, int videoHeight)
	{
		attr::ownDisplayTextures = true;

		const int activeDeviceCount = DvpActiveDeviceCount();

		for (UINT i = 0; i < activeDeviceCount; i++)
		{
			int numStreams = DvpStreamsPerFrame(i);
			for (UINT j = 0; j < numStreams; j++)
			{
				attr::displayTextures[i][j].Create();
				attr::displayTextures[i][j].Bind();
				attr::displayTextures[i][j].SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				attr::displayTextures[i][j].BuildNull(videoWidth, videoHeight);
				assert(glGetError() == GL_NO_ERROR);
			}
		}

		return (glGetError() == GL_NO_ERROR);
	}


	// Blit the frame to the texture
	GLNVSDI_API bool DvpBlitTexture(int target_texture_id, int target_texture_type, int device_index, int video_stream_index)
	{
		if (attr::framePtr[device_index] == nullptr)
			return false;

		const int videoWidth = attr::dvp.GetVideoWidth();
		const int videoHeight = attr::dvp.GetVideoHeight();

				// First blit the buffer object into a texture and chroma expand
		GLint rowLength = attr::framePtr[device_index]->getPitch() / 4;
		glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);

		// Blit the frame to the texture.
		glBindBuffer(
			GL_PIXEL_UNPACK_BUFFER_ARB, 
			attr::framePtr[device_index]->getDstObject(video_stream_index));

		assert(glGetError() == GL_NO_ERROR);

		GLuint id = attr::decodeTextures[device_index][video_stream_index].Id();

		glTexSubImage2D(
			attr::decodeTextures[device_index][video_stream_index].Type(),
			0, 0, 0, 
			(GLsizei)(0.5*videoWidth),
			videoHeight,
			GL_RGBA_INTEGER_EXT, 
			GL_UNSIGNED_BYTE, 
			NULL);

		assert(glGetError() == GL_NO_ERROR);

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

		// Draw a textured quad to the FBO to do the chroma expansion
		// and colorspace conversion.
		glBindFramebufferEXT(
			GL_FRAMEBUFFER_EXT, 
			attr::dvp.m_vidFbos[device_index][video_stream_index]);

		// if not set, use the default texture
		if (target_texture_id < 1)
			target_texture_id = attr::displayTextures[device_index][video_stream_index].Id();

		glFramebufferTexture2DEXT(
			GL_FRAMEBUFFER_EXT,
			GL_COLOR_ATTACHMENT0_EXT,
			target_texture_type,
			target_texture_id,
			0);

		assert(glGetError() == GL_NO_ERROR);

		glClear(GL_COLOR_BUFFER_BIT);

		attr::texBlit.Blit(attr::dvp.decodeProgram, target_texture_id, videoWidth, videoHeight);
		assert(glGetError() == GL_NO_ERROR);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		assert(glGetError() == GL_NO_ERROR);

		return (glGetError() == GL_NO_ERROR);
	}

	GLNVSDI_API bool DvpBlitTextures(int device_index)
	{
		const int videoWidth = DvpWidth();
		const int videoHeight = DvpHeight();
		const int numStreams = DvpStreamsPerFrame(device_index);

		glViewport(0, 0, videoWidth, videoHeight);


		for (int j = 0; j < numStreams; j++)
		{
			
			assert(glGetError() == GL_NO_ERROR);
			attr::displayTextures[device_index][j].Bind();
			assert(glGetError() == GL_NO_ERROR);
			

			assert(glGetError() == GL_NO_ERROR);
			attr::decodeTextures[device_index][j].Bind();
			assert(glGetError() == GL_NO_ERROR);

			DvpBlitTexture(
				attr::displayTextures[device_index][j].Id(), 
				attr::displayTextures[device_index][j].Type(),
				device_index, j);

			attr::decodeTextures[device_index][j].Unbind();
			attr::displayTextures[device_index][j].Unbind();
		}


		assert(glGetError() == GL_NO_ERROR);

		return (glGetError() == GL_NO_ERROR);
	}

	
	static void UNITY_INTERFACE_API OnDvpRenderEvent(int render_event_id)
	{
		switch (static_cast<SdiRenderEvent>(render_event_id))
		{
			case SdiRenderEvent::CaptureFrame:
			{
				SdiMakeCurrent();
				C_Frame *prevFrame[NVAPI_MAX_VIO_DEVICES] = { nullptr };
				const int activeDeviceCount = DvpActiveDeviceCount();
				for (int i = 0; i < activeDeviceCount; i++)
				{
					if (DvpUpdateFrame(i))
						DvpBlitTextures(i);
				}

				attr::dvpOk = (glGetError() == GL_NO_ERROR);
				break;
			}


			case SdiRenderEvent::Initialize:
			{
				SdiSetCurrentDC();
				SdiSetCurrentGLRC();

				attr::dvpOk = DvpCheckAvailability();
				break;
			}

			case SdiRenderEvent::PreSetup:
			{
				attr::dvpOk = DvpPreSetup();
				break;
			}
			case SdiRenderEvent::Setup:
			{
				attr::dvpOk = DvpSetup();
				break;
			}

			case SdiRenderEvent::StartCapture:
			{
				attr::dvpOk = DvpStart();
				break;
			}

			case SdiRenderEvent::StopCapture:
			{
				SdiMakeCurrent();
				attr::dvpOk = DvpStop();
				break;
			}

			case SdiRenderEvent::Shutdown:
			{
				HGLRC uty_hglrc = wglGetCurrentContext();
				HDC uty_hdc = wglGetCurrentDC();
				SdiMakeCurrent();

				attr::dvpOk = DvpStop();
				attr::dvpOk = DvpCleanup();

				wglMakeCurrent(uty_hdc, uty_hglrc);

				break;
			}
		}
	}

	// --------------------------------------------------------------------------
	// GetRenderEventFunc, an example function we export which is used to get a rendering event callback function.
	UnityRenderingEvent GLNVSDI_API UNITY_INTERFACE_API GetDvpRenderEventFunc()
	{
		return OnDvpRenderEvent;
	}


};	//extern "C"
