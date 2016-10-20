
#include "GLNvDvp.h"
#include "glExtensions.h"
#include "DVP.h"
#include "SdiPresentFrame.h"
#include <cinttypes>

namespace global
{
	static bool			dvpInputAvailable = false;
	static bool			dvpOutputAvailable = false;

	static SdiOptions	dvpOptions;

	//static HDC			hDC = nullptr;
	//static HGLRC		hGLRC = nullptr;
	static HDC			externalDC = nullptr;
	static HGLRC		externalGLRC = nullptr;
	static HDC			affinityDC = nullptr;
	static HGLRC		affinityGLRC = nullptr;
	static C_DVP		dvp;
	static C_Frame*		framePtr[NVAPI_MAX_VIO_DEVICES] = { nullptr };
	static C_Frame*		prevFramePtr[NVAPI_MAX_VIO_DEVICES] = { nullptr };
	static GLuint		numDroppedFrames[NVAPI_MAX_VIO_DEVICES];
	static GLuint		drawTimeQuery;
	static GLuint64EXT	drawTimeStart;
	static GLuint64EXT	drawTimeEnd;
	static GLuint64EXT	timeElapsed;
	static bool			dvpOk;
	static bool			ownInputinputDisplayTextures = false;

	static gl::TextureBlit texBlit;
	//
	static gl::Texture2D inputDisplayTextures[NVAPI_MAX_VIO_DEVICES][MAX_VIDEO_STREAMS];
	//
	static gl::TextureRectNV inputDecodeTextures[NVAPI_MAX_VIO_DEVICES][MAX_VIDEO_STREAMS];
	// The raw SDI data from the captured buffer
	// needs to be copied to a texture, so that
	// the data can be read by a shader and
	// processed for display in OpenGL. For 
	// example the shader may do 422->444 expansion	

	static CNvSDIout				sdiOut;						// SDI out object

	static HVIDEOOUTPUTDEVICENV*	outVideoDevices = nullptr;
	static SdiPresentFrame			presentFrame;
	static const int				MaxOutputTextureCount = 4;
	static gl::Texture2D			outTexture[MaxOutputTextureCount];

	static int						duplicateFramesCount = 0;

}

extern "C"
{
	
	GLNVSDI_API bool DvpIsOk()				{ return global::dvpOk; }
	GLNVSDI_API bool DvpInputIsAvailable()	{ return global::dvpInputAvailable; }
	GLNVSDI_API bool DvpOutputIsAvailable()	{ return global::dvpOutputAvailable; }

	GLNVSDI_API void DvpInputDisable()	{ global::dvpInputAvailable = false; }
	GLNVSDI_API void DvpOutputDisable() { global::dvpOutputAvailable = false; }

	GLNVSDI_API bool DvpCheckAvailability()
	{
		global::dvpInputAvailable = false;
		global::dvpOutputAvailable = false;

		HWND hWnd;
		HGLRC hGLRC = wglGetCurrentContext();
		bool dummy_window_created = false;

		if (hGLRC == NULL)
		{
			dummy_window_created = CreateDummyGLWindow(&hWnd, &hGLRC);

			if (!dummy_window_created)
				return false;
		}


		int numGPUs;
		// Note, this function enumerates GPUs which are both CUDA & GLAffinity capable (i.e. newer Quadros)  
		numGPUs = CNvGpuTopology::instance().getNumGpu();

		if (numGPUs > 0)
		{
			global::dvpInputAvailable = (CNvSDIinTopology::instance().getNumDevice() > 0);

			if (global::dvp.m_options.captureGPU >= numGPUs)
			{
				MessageBox(NULL, "Selected GPU is out of range", "Error", MB_OK);
				global::dvpInputAvailable = false;
			}

			global::dvpOutputAvailable = (CNvSDIoutGpuTopology::instance().getNumGpu() > 0);
			
			CNvSDIoutGpu* pOutGpu = CNvSDIoutGpuTopology::instance().getGpu(global::dvpOptions.gpu);
			if (pOutGpu == NULL || (pOutGpu->isSDIoutput() == false))
			{
				MessageBox(NULL, "GPU doesn't have a valid SDI output device attached", "Error", MB_OK);
				global::dvpOutputAvailable = false;
			}

			DvpSetGlobalOptions();

			// We can kill the dummy window now
			if (dummy_window_created)
				DestroyGLWindow(&hWnd, &hGLRC);

			return true;
		}
		else
		{
			MessageBox(NULL, "Unable to obtain system GPU topology", "Error", MB_OK);
			global::dvpInputAvailable = false;
			global::dvpOutputAvailable = false;
			return false;
		}
	}

	GLNVSDI_API void DvpSetGlobalOptions()
	{
		//set the defaults for all the relevant options
		//
		// Input Options
		//
		global::dvpOptions.sampling = NVVIOCOMPONENTSAMPLING_422;
		global::dvpOptions.dualLink = false;
		global::dvpOptions.bitsPerComponent = 8;
		global::dvpOptions.expansionEnable = true;
		global::dvpOptions.captureDevice = 0;
		global::dvpOptions.captureGPU = CNvGpuTopology::instance().getPrimaryGpuIndex();

		global::dvpOptions.inputRingBufferSize = 3;

		switch (global::dvpOptions.sampling)
		{
			case NVVIOCOMPONENTSAMPLING_422:
				break;
			case NVVIOCOMPONENTSAMPLING_4224:
			case NVVIOCOMPONENTSAMPLING_444:
			case NVVIOCOMPONENTSAMPLING_4444:
				global::dvpOptions.dualLink = true;
				break;
			default:
				break;
		}

		if (global::dvpOptions.bitsPerComponent > 10)
			global::dvpOptions.dualLink = true;

		//
		// Output Options
		//
		global::dvpOptions.block = FALSE;
		global::dvpOptions.console = TRUE;
		global::dvpOptions.dataFormat = NVVIODATAFORMAT_R8G8B8A8_TO_YCRCBA4224;
		global::dvpOptions.syncSource = NVVIOSYNCSOURCE_COMPSYNC;
		global::dvpOptions.videoFormat = NVVIOSIGNALFORMAT_1080I_59_94_SMPTE274;
		global::dvpOptions.field = FALSE;
		global::dvpOptions.flipQueueLength = 3;
		global::dvpOptions.fps = TRUE;
		global::dvpOptions.frameLock = FALSE;
		global::dvpOptions.fsaa = 1;
		global::dvpOptions.hDelay = 0;
		global::dvpOptions.vDelay = 0;
		global::dvpOptions.log = FALSE;
		global::dvpOptions.numFrames = 0;
		global::dvpOptions.repeat = 1;
		global::dvpOptions.syncEnable = TRUE;
		global::dvpOptions.videoInfo = FALSE;
		global::dvpOptions.cscEnable = FALSE;
		global::dvpOptions.cscOffset[0] = 0.0625; global::dvpOptions.cscOffset[1] = 0.5; global::dvpOptions.cscOffset[2] = 0.5;
		global::dvpOptions.cscScale[0] = 0.85547; global::dvpOptions.cscScale[1] = 0.875; global::dvpOptions.cscScale[2] = 0.875;
		global::dvpOptions.cscMatrix[0][0] = 0.2130f; global::dvpOptions.cscMatrix[0][1] = 0.7156f; global::dvpOptions.cscMatrix[0][2] = 0.0725f;
		global::dvpOptions.cscMatrix[1][0] = 0.5000f; global::dvpOptions.cscMatrix[1][1] = -0.4542f; global::dvpOptions.cscMatrix[1][2] = -0.0455f;
		global::dvpOptions.cscMatrix[2][0] = 0.1146f; global::dvpOptions.cscMatrix[2][1] = -0.3350f; global::dvpOptions.cscMatrix[2][2] = 0.5000f;
		global::dvpOptions.gamma[0] = 1.0f; global::dvpOptions.gamma[1] = 1.0f; global::dvpOptions.gamma[2] = 1.0f;
		global::dvpOptions.fullScreen = FALSE;
		global::dvpOptions.gpu = 0;
	}
	
	

	GLNVSDI_API bool DvpCreateAffinityContext()
	{
		// Make  SDI output GL context current.
		wglMakeCurrent(NULL, NULL);

		CNvSDIoutGpu* pOutGpu = CNvSDIoutGpuTopology::instance().getGpu(global::dvpOptions.gpu);
		if (pOutGpu == nullptr)
			return false;

		//Create Affinity DC for SDI output
		HGPUNV handles[2];
		handles[0] = pOutGpu->getAffinityHandle();
		handles[1] = nullptr;

		global::affinityDC = wglCreateAffinityDCNV(handles);
		if (global::affinityDC == nullptr)
		{
			int error = GetLastError();
			std::cerr << "Error: wglCreateAffinityDCNV error code: " << error << std::endl;
			return false;
		}

		int	PixelFormat;									// Holds The Results After Searching For A Match
		// We need a pixel format descriptor.  A PFD tells how OpenGL draws
		PIXELFORMATDESCRIPTOR pfd =							// pfd Tells Windows How We Want Things To Be
		{
			sizeof(PIXELFORMATDESCRIPTOR),					// Size Of This Pixel Format Descriptor
			1,												// Version Number
			PFD_DRAW_TO_WINDOW |							// Format Must Support Window
			PFD_SUPPORT_OPENGL |							// Format Must Support OpenGL
			PFD_DOUBLEBUFFER,								// Must Support Double Buffering
			PFD_TYPE_RGBA,									// Request An RGBA Format
			32,												// Select Our Color Depth
			0, 0, 0, 0, 0, 0,								// Color Bits Ignored
			1,												// Alpha Buffer
			0,												// Shift Bit Ignored
			0,												// No Accumulation Buffer
			0, 0, 0, 0,										// Accumulation Bits Ignored
			24,												// 24 Bit Z-Buffer (Depth Buffer)  
			8,												// 8 Bit Stencil Buffer
			0,												// No Auxiliary Buffer
			PFD_MAIN_PLANE,									// Main Drawing Layer
			0,												// Reserved
			0, 0, 0											// Layer Masks Ignored
		};

		// get the appropriate pixel format
		PixelFormat = ChoosePixelFormat(global::affinityDC, &pfd);
		if (PixelFormat == 0) 
		{
			std::cerr << "ChoosePixelFormat() failed:  Cannot find format specified." << std::endl;
			return false;
		}

		// set the pixel format 
		if (SetPixelFormat(global::affinityDC, PixelFormat, &pfd) == FALSE) 
		{
			std::cerr << "SetPixelFormat() failed:  Cannot set format specified." << std::endl;
			return false;
		}

		// Create rendering context from the affinity device context	
		global::affinityGLRC = wglCreateContext(global::affinityDC);
		

		//if (!wglShareLists(global::affinityGLRC, global::externalGLRC))
		if (!wglShareLists(global::externalGLRC, global::affinityGLRC))
		{
			std::cout << "Could not share OpenGL contexts: " << GetLastError() << std::endl;
			return false;
		}


		// Make  SDI output GL context current.
		wglMakeCurrent(global::affinityDC, global::affinityGLRC);
		
		return true;
	}
		
	GLNVSDI_API void DvpDestroyAffinityContext()
	{
		if (global::affinityDC != nullptr)
		{
			wglDeleteDCNV(global::affinityDC);
			global::affinityDC = nullptr;
		}

		if (global::affinityGLRC != nullptr)
		{
			wglDeleteContext(global::affinityGLRC);
			global::affinityGLRC = nullptr;
		}
	}
	
	GLNVSDI_API void DvpGetAffinityContext(HDC& hDC, HGLRC& hGLRC)
	{
		hDC = global::affinityDC;
		hGLRC = global::affinityGLRC;
	}


	GLNVSDI_API void DvpSetAffinityContext(HDC _hDC, HGLRC _hGLRC)
	{
		global::affinityDC = (_hDC != nullptr) ? _hDC : wglGetCurrentDC();
		global::affinityGLRC = (_hGLRC != nullptr) ? _hGLRC : wglGetCurrentContext();
	}


	GLNVSDI_API void DvpSetExternalContext(HDC _hDC, HGLRC _hGLRC)
	{
		global::externalDC = (_hDC != nullptr) ? _hDC : wglGetCurrentDC();
		global::externalGLRC = (_hGLRC != nullptr) ? _hGLRC : wglGetCurrentContext();
	}

	GLNVSDI_API bool DvpMakeAffinityCurrent()
	{
		return wglMakeCurrent(global::affinityDC, global::affinityGLRC);
	}

	GLNVSDI_API bool DvpMakeExternalCurrent()
	{
		return wglMakeCurrent(global::externalDC, global::externalGLRC);
	}


	GLNVSDI_API C_DVP* DvpInputPtr()
	{
		return &global::dvp;
	}

	GLNVSDI_API C_Frame* DvpInputFrame(int device_index)
	{
		return global::framePtr[device_index];
	}

	GLNVSDI_API C_Frame* DvpInputPreviousFrame(int device_index)
	{
		return global::prevFramePtr[device_index];
	}

	GLNVSDI_API uint64_t DvpInputDroppedFrames(int device_index)
	{
		return global::numDroppedFrames[device_index];
	}

	GLNVSDI_API NVVIOSIGNALFORMAT DvpInputSignalFormat()
	{
		return global::dvp.GetSignalFormat();
	}


	GLNVSDI_API bool DvpInputInitialize()
	{
#if 1
		DvpCreateAffinityContext();		// create own context and share it
#else
		DvpSetAffinityContext(global::externalDC, global::externalGLRC);
#endif

		DvpMakeAffinityCurrent();


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


		if (global::dvp.SetupSDIPipeline() != S_OK)
			return false;

		return (glGetError() == GL_NO_ERROR);
	}

	GLNVSDI_API int  DvpInputWidth()
	{
		return global::dvp.GetVideoWidth();
	}


	GLNVSDI_API int  DvpInputHeight()
	{
		return global::dvp.GetVideoHeight();
	}


	GLNVSDI_API bool DvpInputCreateTextures(int videoWidth, int videoHeight)
	{
		global::ownInputinputDisplayTextures = true;

		const int activeDeviceCount = DvpInputActiveDeviceCount();

		for (UINT i = 0; i < activeDeviceCount; i++)
		{
			int numStreams = DvpInputStreamsPerFrame(i);
			for (UINT j = 0; j < numStreams; j++)
			{
				global::inputDisplayTextures[i][j].Create();
				global::inputDisplayTextures[i][j].Bind();
				global::inputDisplayTextures[i][j].SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				global::inputDisplayTextures[i][j].BuildNull(videoWidth, videoHeight);
				assert(glGetError() == GL_NO_ERROR);
			}
		}

		return (glGetError() == GL_NO_ERROR);
	}


	GLNVSDI_API void DvpInputSetTexture(GLuint id, int device_index, int video_stream_index)
	{
		global::inputDisplayTextures[device_index][video_stream_index].SetId(id);
	}


	GLNVSDI_API void DvpInputSetTexturePtr(void* texturePtr, int device_index, int video_stream_index)
	{
		GLuint gltex = (GLuint)(size_t)(texturePtr);
		global::inputDisplayTextures[device_index][video_stream_index].SetId(gltex);
	}


	GLNVSDI_API gl::Texture2D* DvpInputDisplayTexture(int device_index, int video_stream_index)
	{
		return &global::inputDisplayTextures[device_index][video_stream_index];
	}


	GLNVSDI_API int DvpInputActiveDeviceCount()
	{
		return global::dvp.GetActiveDeviceCount();
	}


	GLNVSDI_API int DvpInputStreamsPerFrame(int device_index)
	{
#ifdef USE_ALL_STREAMS
		return NUM_VIDEO_STREAMS;
#else
		return global::dvp.GetNumStreamsPerFrame(device_index);
#endif	
	}


	GLNVSDI_API bool DvpInputSetup()
	{
		int videoWidth = global::dvp.GetVideoWidth();
		int videoHeight = global::dvp.GetVideoHeight();

		// check if DvpPreSetup has been called previously
		if (videoWidth < 1 || videoHeight < 1)
			return false;


		global::dvp.SetupSDIinGL(global::affinityDC, global::affinityGLRC);

		DvpMakeAffinityCurrent();

		assert(glGetError() == GL_NO_ERROR);

		// To view the buffers we need to load an appropriate shader
		global::dvp.SetupDecodeProgram();
		int activeDeviceCount = global::dvp.GetActiveDeviceCount();
		if (activeDeviceCount == 0)
			return false;

		//
		// Check if the textures have been created
		//
		if (global::inputDisplayTextures[0][0].Id() < 1)
		{
			//allocate the textures for display
			if (!DvpInputCreateTextures(videoWidth, videoHeight))
				return false;
		}


		//create the textures to go with the buffers and frame buffer objects to create the display textures
		for (UINT d = 0; d < activeDeviceCount; d++)
		{
			int numStreams = global::dvp.GetNumStreamsPerFrame(d);
#ifdef USE_ALL_STREAMS	
			numStreams = NUM_VIDEO_STREAMS;
#endif		        

			glGenFramebuffersEXT(numStreams, &global::dvp.m_vidFbos[d][0]);
			assert(glGetError() == GL_NO_ERROR);

			for (unsigned int s = 0; s < numStreams; s++)
			{
				global::inputDecodeTextures[d][s].Create();
				global::inputDecodeTextures[d][s].Bind();
				global::inputDecodeTextures[d][s].SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				global::inputDecodeTextures[d][s].SetParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				// Allocate storage for the decode texture.
				glTexImage2D(
					global::inputDecodeTextures[d][s].Type(),
					0, GL_RGBA8UI,
					(GLsizei)(videoWidth*0.5), videoHeight,
					0, GL_RGBA_INTEGER_EXT,
					GL_UNSIGNED_BYTE, NULL);

				assert(glGetError() == GL_NO_ERROR);

				// Configure the decode->output FBO.
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, global::dvp.m_vidFbos[d][s]);
				assert(glGetError() == GL_NO_ERROR);

				glFramebufferTexture2DEXT(
					GL_FRAMEBUFFER_EXT,
					GL_COLOR_ATTACHMENT0_EXT,
					global::inputDisplayTextures[d][s].Type(),
					global::inputDisplayTextures[d][s].Id(),
					0);
				assert(glGetError() == GL_NO_ERROR);


				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			}

		}

		glGenQueries(1, &global::drawTimeQuery);

		global::texBlit.CreateVbo();

		return (glGetError() == GL_NO_ERROR);
	}

	GLNVSDI_API bool DvpInputCleanup()
	{
		global::dvp.CleanupSDIPipeline();

		global::dvp.CleanupSDIinGL();

		DvpMakeAffinityCurrent();

		int activeDeviceCount = global::dvp.GetActiveDeviceCount();
		if (activeDeviceCount == 0)
			return false;

		for (UINT i = 0; i < activeDeviceCount; i++)
		{
			int numStreams = global::dvp.GetNumStreamsPerFrame(i);
#ifdef USE_ALL_STREAMS		
			numStreams = NUM_VIDEO_STREAMS;
#endif		  
			for (int j = 0; j < numStreams; ++j)
			{
				global::inputDecodeTextures[i][j].Destroy();

				if (global::ownInputinputDisplayTextures)
					global::inputDisplayTextures[i][j].Destroy();
			}

			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			glDeleteFramebuffersEXT(numStreams, &global::dvp.m_vidFbos[i][0]);
		}
		global::ownInputinputDisplayTextures = false;
		global::dvp.DestroyDecodeProgram();

		global::texBlit.DestroyVbo();

		for (int i = 0; i < NVAPI_MAX_VIO_DEVICES; ++i)
		{
			global::framePtr[i] = nullptr;
			global::prevFramePtr[i] = nullptr;
		}
				
		return (glGetError() == GL_NO_ERROR);
	}

	GLNVSDI_API C_Frame* DvpInputUpdateFrame(int device_index)
	{
		global::framePtr[device_index] = global::dvp.GetFrame(device_index);

		if (global::framePtr[device_index] != nullptr)
		{
			if (global::prevFramePtr[device_index] != nullptr)
			{
				global::numDroppedFrames[device_index] +=
					global::framePtr[device_index]->sequenceNum - global::prevFramePtr[device_index]->sequenceNum - 1;

				global::dvp.ReleaseUsedFrame(device_index, global::prevFramePtr[device_index]);
			}
			global::prevFramePtr[device_index] = global::framePtr[device_index];
		}
		else
		{
			if (global::prevFramePtr[device_index] != nullptr)
				global::framePtr[device_index] = global::prevFramePtr[device_index];
		}

		return global::framePtr[device_index];
	}


	// Blit the frame to the texture
	GLNVSDI_API bool DvpInputBlitTexture(int target_texture_id, int target_texture_type, int device_index, int video_stream_index)
	{
		if (global::framePtr[device_index] == nullptr)
			return false;

		const int videoWidth = global::dvp.GetVideoWidth();
		const int videoHeight = global::dvp.GetVideoHeight();

		// First blit the buffer object into a texture and chroma expand
		GLint rowLength = global::framePtr[device_index]->getPitch() / 4;
		glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);

		// Blit the frame to the texture.
		glBindBuffer(
			GL_PIXEL_UNPACK_BUFFER_ARB,
			global::framePtr[device_index]->getDstObject(video_stream_index));

		assert(glGetError() == GL_NO_ERROR);

		GLuint id = global::inputDecodeTextures[device_index][video_stream_index].Id();

		glTexSubImage2D(
			global::inputDecodeTextures[device_index][video_stream_index].Type(),
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
			global::dvp.m_vidFbos[device_index][video_stream_index]);

		// if not set, use the default texture
		if (target_texture_id < 1)
			target_texture_id = global::inputDisplayTextures[device_index][video_stream_index].Id();

		glFramebufferTexture2DEXT(
			GL_FRAMEBUFFER_EXT,
			GL_COLOR_ATTACHMENT0_EXT,
			target_texture_type,
			target_texture_id,
			0);

		assert(glGetError() == GL_NO_ERROR);

		glClear(GL_COLOR_BUFFER_BIT);

		global::texBlit.Blit(global::dvp.decodeProgram, target_texture_id, videoWidth, videoHeight);
		assert(glGetError() == GL_NO_ERROR);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		assert(glGetError() == GL_NO_ERROR);

		return (glGetError() == GL_NO_ERROR);
	}

	GLNVSDI_API bool DvpInputBlitTextures(int device_index)
	{
		const int videoWidth = DvpInputWidth();
		const int videoHeight = DvpInputHeight();
		const int numStreams = DvpInputStreamsPerFrame(device_index);

		glViewport(0, 0, videoWidth, videoHeight);

		for (int j = 0; j < numStreams; j++)
		{
			assert(glGetError() == GL_NO_ERROR);
			global::inputDisplayTextures[device_index][j].Bind();
			assert(glGetError() == GL_NO_ERROR);

			assert(glGetError() == GL_NO_ERROR);
			global::inputDecodeTextures[device_index][j].Bind();
			assert(glGetError() == GL_NO_ERROR);

			DvpInputBlitTexture(
				global::inputDisplayTextures[device_index][j].Id(),
				global::inputDisplayTextures[device_index][j].Type(),
				device_index, j);

			global::inputDecodeTextures[device_index][j].Unbind();
			global::inputDisplayTextures[device_index][j].Unbind();
		}


		assert(glGetError() == GL_NO_ERROR);

		return (glGetError() == GL_NO_ERROR);
	}

	///////////////////////////////////////////////////////////////////////
	/// Setup opengl dependencies for sdi capture
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API bool DvpOutputSetupGL()
	{
		if (!loadPresentVideoExtension() 
			|| !loadAffinityExtension()
			|| !loadFramebufferObjectExtension() 
			|| !loadTimerQueryExtension() 
			|| !loadSwapIntervalExtension())
		{
			//SdiLog() << "Couldn't load required OpenGL extensions." << std::endl;
			return false;
		}

		// Don't sync graphics drawing to the vblank.m  This permits
		// drawing to be synchronized to the SDI scanout.  Otherwise,
		// duplicate frames on the SDI output will result as drawing
		// of the next frame is blocked until the SwapBuffer call
		// returns.
		wglSwapIntervalEXT(0);


		// Bind the video device for output.

		// Enumerate the available video devices and
		// bind to the first one found

		// Get list of available video devices.
		int numDevices = wglEnumerateVideoDevicesNV(global::affinityDC, NULL);

		if (numDevices <= 0)
		{
			//SdiLog() << "wglEnumerateVideoDevicesNV() did not return any devices." << std::endl;
			return false;
		}

		global::outVideoDevices = (HVIDEOOUTPUTDEVICENV *)malloc(numDevices * sizeof(HVIDEOOUTPUTDEVICENV));

		if (!global::outVideoDevices)
		{
			//SdiLog() << "malloc failed.  OOM?" << std::endl;
			return false;
		}

		if (numDevices != wglEnumerateVideoDevicesNV(global::affinityDC, global::outVideoDevices))
		{
			free(global::outVideoDevices);
			//SdiLog() << "Inconsistent results from wglEnumerateVideoDevicesNV()" << std::endl;
			return false;
		}

		global::presentFrame.Initialize();


		//Bind the first device found that is connected to the output GPU
		if (!wglBindVideoDeviceNV(global::affinityDC, 1, global::outVideoDevices[0], NULL))
		{
			//SdiLog() << "Failed to bind a videoDevice to slot 0." << std::endl;
			return false;
		}


		return true;
	}

	///////////////////////////////////////////////////////////////////////
	/// Cleanup the opengl stuff used in sdi output
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void DvpOutputCleanupGL()
	{
		DvpMakeAffinityCurrent();

		//Bind the first device found that is connected to the output GPU
		if (!wglBindVideoDeviceNV(global::affinityDC, 1, NULL, NULL))
		{
			//SdiLog() << "Failed to bind a videoDevice to slot 0." << std::endl;
			return;
		}

		// Free video devices
		free(global::outVideoDevices);

		global::presentFrame.Uninitialize();
	}

	GLNVSDI_API void DvpOutputSetTexture(int index, GLuint id)
	{
		global::outTexture[index].SetId(id);
	}


	GLNVSDI_API void DvpOutputSetTexturePtr(int index, void* texturePtr, int w, int h)
	{
		GLuint gltex = (GLuint)(size_t)(texturePtr);
		global::outTexture[index].SetId(gltex);
	}

		

	///////////////////////////////////////////////////////////////////////
	/// Send the current frame to sdi output
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void DvpOutputPresentFrame()
	{
		const bool dual_output = global::dvpOptions.IsDualOutput();

		int tex0 = 0;
		int tex1 = 1;
		int tex2 = 2;
		int tex3 = 3;

		if (global::dvpOptions.InvertFields())
		{
			int tex0 = 1;
			int tex1 = 0;
			int tex2 = 3;
			int tex3 = 2;
		}

		if (global::sdiOut.IsInterlaced())
		{
			if (dual_output)
				global::presentFrame.PresentFrameDual(global::outTexture[0].Type(),
				global::outTexture[tex0].Id(), global::outTexture[tex1].Id(),
				global::outTexture[tex2].Id(), global::outTexture[tex3].Id());
			else
				global::presentFrame.PresentFrame(global::outTexture[0].Type(),
				global::outTexture[tex0].Id(), global::outTexture[tex1].Id());
		}
		else
		{
			if (dual_output)
				global::presentFrame.PresentFrameDual(global::outTexture[0].Type(),
				global::outTexture[tex0].Id(), global::outTexture[tex1].Id());
			else
				global::presentFrame.PresentFrame(global::outTexture[0].Type(),
				global::outTexture[tex0].Id());
		}

		const int duplicated_frames = global::presentFrame.GetStats().durationTime - 1;
		
		global::duplicateFramesCount += duplicated_frames;
	}



	static void UNITY_INTERFACE_API OnGLNvDvpRenderEventFunc(int render_event_id)
	{

		switch (static_cast<DvpRenderEvent>(render_event_id))
		{

		case DvpRenderEvent::Update:
		{
			DvpMakeAffinityCurrent();

			//
			// Capture Frame
			//
			if (global::dvpInputAvailable)
			{
				C_Frame *prevFrame[NVAPI_MAX_VIO_DEVICES] = { nullptr };
				const int activeDeviceCount = DvpInputActiveDeviceCount();
				for (int i = 0; i < activeDeviceCount; i++)
				{
					if (DvpInputUpdateFrame(i))
						DvpInputBlitTextures(i);
				}
			}
						
			if (global::dvpOutputAvailable)
				DvpOutputPresentFrame();

			if (global::affinityDC != global::externalDC)
				::SwapBuffers(global::affinityDC);

			global::dvpOk = (glGetError() == GL_NO_ERROR);

			DvpMakeExternalCurrent();

			break;
		}

		case DvpRenderEvent::CheckAvalability:
		{
			global::dvpOk = DvpCheckAvailability();
			break;
		}

		case DvpRenderEvent::Initialize:
		{
			DvpSetExternalContext();
			
			if (global::dvpInputAvailable)
				global::dvpOk = DvpInputInitialize();

			DvpMakeExternalCurrent();
			break;
		}

		case DvpRenderEvent::Setup:
		{
			DvpMakeAffinityCurrent();
			
			if (global::dvpInputAvailable)
				global::dvpOk = DvpInputSetup();

			global::dvpOptions.videoFormat = global::dvp.GetSignalFormat();

			if (global::dvpOutputAvailable && global::dvpOk)
			{
				global::dvpOk = DvpOutputSetupGL();
								
				if (false) //(captureFields)
				{
					DvpOutputSetTexture(0, global::inputDisplayTextures[0][0].Id());
					DvpOutputSetTexture(1, global::inputDisplayTextures[0][1].Id());
				}
				else
				{
					DvpOutputSetTexture(0, global::inputDisplayTextures[0][0].Id());
					DvpOutputSetTexture(1, global::inputDisplayTextures[0][0].Id());
				}
			}

			if (global::dvpInputAvailable)
				global::dvpOk = (global::dvp.StartSDIPipeline() == S_OK);

			DvpMakeExternalCurrent();

			break;
		}

		case DvpRenderEvent::Cleanup:
		{
			DvpMakeAffinityCurrent();

			if (global::dvpInputAvailable)
			{
				global::dvpOk = (global::dvp.StopSDIPipeline() == S_OK);
				DvpInputCleanup();
			}

			if (global::dvpOutputAvailable)
			{
				DvpOutputCleanupGL();
				global::sdiOut.Cleanup();
			}

			DvpMakeExternalCurrent();
			break;
		}

		case DvpRenderEvent::Uninitialize:
		{
			DvpDestroyAffinityContext();
			global::affinityDC = nullptr;
			global::affinityGLRC = nullptr;
			DvpMakeExternalCurrent();
			break;
		}

		}	// end switch
	}

	// --------------------------------------------------------------------------
	// GetRenderEventFunc, an example function we export which is used to get a rendering event callback function.
	UnityRenderingEvent GLNVSDI_API UNITY_INTERFACE_API GetGLNvDvpRenderEventFunc()
	{
		return OnGLNvDvpRenderEventFunc;
	}


};	//extern "C"
