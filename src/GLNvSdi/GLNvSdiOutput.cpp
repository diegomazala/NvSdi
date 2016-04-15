
#include "GLNvSdiOutput.h"
#include "glExtensions.h"
#include "fbo.h"
#include "common.h"
#include "Texture.h"
#include "GLFbo.h"
#include "SdiPresentFrame.h"


extern "C"
{
	// global variables
	namespace attr
	{
		const int				cOutputTextureMaxCount	= 4;
		const int				cOutputFboMaxCount		= 2;	

		gl::Texture2D			outputTexture[cOutputTextureMaxCount];
		GLFbo					fbo[cOutputFboMaxCount];
		
		bool					textures = false;
		bool					fbos = false;

		HGLRC					outRC = NULL;	

		HVIDEOOUTPUTDEVICENV*	pVideoDevices;

		static CNvSDIout		sdiOut;						// SDI out object

		SdiPresentFrame			presentFrame;

		int						duplicateFramesCount = 0;

		bool					dualOutput = false;

		bool					isPresentingFrames = false;
	}
	

	GLNVSDI_API void SdiOutputInvertFields(bool invert)
	{
		SdiGlobalOptions().InvertFields(invert);
	}

	GLNVSDI_API void SdiOutputSetGlobalOptions()
	{
		SdiGlobalOptions().block = FALSE;
		SdiGlobalOptions().console = TRUE;
		SdiGlobalOptions().dataFormat = NVVIODATAFORMAT_R8G8B8A8_TO_YCRCBA4224;
		SdiGlobalOptions().syncSource = NVVIOSYNCSOURCE_COMPSYNC;
		SdiGlobalOptions().videoFormat = NVVIOSIGNALFORMAT_1080I_59_94_SMPTE274; 
		SdiGlobalOptions().field = FALSE;
		SdiGlobalOptions().flipQueueLength = 5;
		SdiGlobalOptions().fps = TRUE;
		SdiGlobalOptions().frameLock = FALSE;
		SdiGlobalOptions().fsaa = 1;
		SdiGlobalOptions().hDelay = 0;
		SdiGlobalOptions().vDelay = 0;
		SdiGlobalOptions().log = FALSE;
		SdiGlobalOptions().numFrames = 0;
		SdiGlobalOptions().repeat = 1;
		SdiGlobalOptions().syncEnable = TRUE;
		SdiGlobalOptions().videoInfo = FALSE;
		SdiGlobalOptions().cscEnable = FALSE;
		SdiGlobalOptions().cscOffset[0] = 0.0625; SdiGlobalOptions().cscOffset[1] = 0.5; SdiGlobalOptions().cscOffset[2] = 0.5;
		SdiGlobalOptions().cscScale[0] = 0.85547; SdiGlobalOptions().cscScale[1] = 0.875; SdiGlobalOptions().cscScale[2] = 0.875;
		SdiGlobalOptions().cscMatrix[0][0] = 0.2130f; SdiGlobalOptions().cscMatrix[0][1] = 0.7156f; SdiGlobalOptions().cscMatrix[0][2] = 0.0725f;
		SdiGlobalOptions().cscMatrix[1][0] = 0.5000f; SdiGlobalOptions().cscMatrix[1][1] = -0.4542f; SdiGlobalOptions().cscMatrix[1][2] = -0.0455f;
		SdiGlobalOptions().cscMatrix[2][0] = 0.1146f; SdiGlobalOptions().cscMatrix[2][1] = -0.3350f; SdiGlobalOptions().cscMatrix[2][2] = 0.5000f;
		SdiGlobalOptions().gamma[0] = 1.0f; SdiGlobalOptions().gamma[1] = 1.0f; SdiGlobalOptions().gamma[2] = 1.0f;
		SdiGlobalOptions().fullScreen = FALSE;
		SdiGlobalOptions().gpu = 0;
	}

	///////////////////////////////////////////////////////////////////////
	/// Setup the main attributes for sdi output
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void SdiOutputSetVideoFormat(SdiVideoFormat video_format, SdiSyncSource sync_source, int h_delay, int v_delay, bool dual_output, int flip_queue_lenght)
	{
		SdiGlobalOptions().SetVideoFormat(video_format, sync_source, h_delay, v_delay, dual_output);
		SdiGlobalOptions().flipQueueLength = flip_queue_lenght;
	}

	///////////////////////////////////////////////////////////////////////
	/// Return the Sdi Output object
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API CNvSDIout* SdiOutput()
	{
		return &attr::sdiOut;
	}


	GLNVSDI_API gl::Texture2D* SdiOutputGetTexture(int index)
	{
		return &attr::outputTexture[index];
	}

	GLNVSDI_API GLuint SdiOutputGetTextureId(int index)
	{
		return attr::outputTexture[index].Id();
	}

	GLNVSDI_API GLenum SdiOutputGetTextureType(int index)
	{
		return attr::outputTexture[index].Type();
	}

	GLNVSDI_API void SdiOutputSetTexture(int index, GLuint id)
	{
		attr::outputTexture[index].SetId(id);
	}

	GLNVSDI_API void SdiOutputSetTexturePtr(int index, void* texturePtr, int w, int h)
	{
		GLuint gltex = (GLuint)(size_t)(texturePtr);
		attr::outputTexture[index].SetId(gltex);
	}



	///////////////////////////////////////////////////////////////////////
	/// Return video input width
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API int SdiOutputWidth()
	{
		return attr::sdiOut.GetWidth();
	}


	///////////////////////////////////////////////////////////////////////
	/// Return video input height
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API int SdiOutputHeight()
	{
		return attr::sdiOut.GetHeight();
	}


	/// Return the number of duplicated frames in the last update 
	GLNVSDI_API int SdiOutputDuplicatedFrames()
	{
		return attr::presentFrame.GetStats().durationTime - 1;
	}


	/// Return the count of duplicated frames in the last run
	GLNVSDI_API int SdiOutputDuplicatedFramesCount()
	{
		return attr::duplicateFramesCount;
	}

	/// Return the count of duplicated frames in the last run
	GLNVSDI_API void SdiOutputResetDuplicatedFramesCount()
	{
		attr::duplicateFramesCount = 0;
	}


	///////////////////////////////////////////////////////////////////////
	/// Initialize the NvSdi Input according to options set previously
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API bool SdiOutputInitialize()
	{	
		attr::duplicateFramesCount = 0;

		// Note, this function enumerates GPUs which are both CUDA & GLAffinity capable (i.e. newer Quadros)  
		if(SdiOutputGpuCount() <= 0)
		{
			SdiLog() << "Unable to obtain system GPU topology" << std::endl;
			return false;
		}

		CNvSDIoutGpu* pOutGpu = CNvSDIoutGpuTopology::instance().getGpu(SdiGlobalOptions().gpu);
		if(pOutGpu == NULL || (pOutGpu->isSDIoutput() == false))
		{
			SdiLog() << "GPU doesn't have a valid SDI output device attached" << std::endl;
			return false;
		}

		return true;
	}


	///////////////////////////////////////////////////////////////////////
	/// Uninitialize the sdi input pipeline
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void SdiOutputUninitialize()
	{
		CNvSDIoutGpuTopology::destroy();
		std::cout << "Duplicated Frames Count: " << attr::duplicateFramesCount << std::endl;
	}



	///////////////////////////////////////////////////////////////////////
	/// Configure the sdi devices for startup
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API bool SdiOutputSetupDevices()
	{	
		// Setup the video device.
		if (attr::sdiOut.Init(&SdiGlobalOptions(), CNvSDIoutGpuTopology::instance().getGpu(SdiGlobalOptions().gpu)) != S_OK) 
		{
			SdiLog() << "Unable to initialize video device. Check sync signal." << std::endl;
			return false;
		}

		return true;
	}


	///////////////////////////////////////////////////////////////////////
	/// Empty
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void SdiOutputCleanupDevices()
	{
		attr::sdiOut.Cleanup();
	}


	///////////////////////////////////////////////////////////////////////
	/// Create textures for output 
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void SdiOutputCreateTextures()
	{
		attr::textures = true;
		for (int i=0; i<attr::cOutputTextureMaxCount; ++i)
		{
			// Create texture object
			attr::outputTexture[i].Create();
			attr::outputTexture[i].Bind();
			attr::outputTexture[i].SetMipmap(false);
			attr::outputTexture[i].SetParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			attr::outputTexture[i].SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			attr::outputTexture[i].SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP);
			attr::outputTexture[i].SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP);
			attr::outputTexture[i].BuildNull(SdiOutputWidth(), SdiOutputHeight());
		}

	}

	///////////////////////////////////////////////////////////////////////
	/// Destroy textures used for output 
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void SdiOutputDestroyTextures()
	{
		if (!attr::textures)
			return;

		for (int i=0; i<attr::cOutputTextureMaxCount; ++i)
		{
			attr::outputTexture[i].Destroy();
		}
		attr::textures = false;
	}


	///////////////////////////////////////////////////////////////////////
	/// Create fbo for output 
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void SdiOutputInitializeFbo()
	{	
		attr::fbos = true;

		if (attr::sdiOut.IsInterlaced())
		{
			// Create FBO, pass in texture object to attach.
			for (int i=0; i<attr::cOutputFboMaxCount; ++i)
				attr::fbo[i].Initialize(SdiOutputWidth(), SdiOutputHeight(), 8, SdiGlobalOptions().fsaa, GL_TRUE, GL_TRUE, &attr::outputTexture[i*2], &attr::outputTexture[i*2+1]);
		}
		else
		{
			// Create FBO, pass in texture object to attach.
			for (int i=0; i<attr::cOutputFboMaxCount; ++i)
				attr::fbo[i].Initialize(SdiOutputWidth(), SdiOutputHeight(), 8, SdiGlobalOptions().fsaa, GL_TRUE, GL_TRUE, &attr::outputTexture[i], NULL);
		}
	}


	
	///////////////////////////////////////////////////////////////////////
	/// Destroy fbo used for output 
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void SdiOutputUninitializeFbo()
	{
		if (!attr::fbos)
			return;

		for (int i=0; i<attr::cOutputFboMaxCount; ++i)
			attr::fbo[i].Uninitialize();

		attr::fbos = false;
	}




	///////////////////////////////////////////////////////////////////////
	/// Setup opengl dependencies for sdi capture
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API bool SdiOutputSetupContextGL(HDC hDC, HGLRC outRC)
	{
		//Create Affinity DC for SDI output
		HGPUNV handles[2];
		handles[0] = CNvSDIoutGpuTopology::instance().getGpu(SdiGlobalOptions().gpu)->getAffinityHandle();
		handles[1] = NULL;

		if (hDC == NULL)
		{
			if (SdiGetDC() == NULL)
			{
				SdiSetAffinityDC(wglCreateAffinityDCNV(handles));
				SdiSetDC(SdiGetAffinityDC());
				if(SdiGetDC() == NULL)
				{
					int error = GetLastError();
					SdiLog() << "Error: wglCreateAffinityDCNV error code " << error << std::endl;
					return false;
				}
				if (SdiSetupPixelFormat(SdiGetDC()) == 0)
					return false;
			}
		}
		else
		{
			SdiSetDC(hDC);
		}
		
		// checking if a rendering context was passed, otherwise we must create it
		if (outRC == NULL)
		{
			if (SdiGetGLRC() == NULL)
			{
				// Create rendering context from the affinity device context	
				SdiSetGLRC(wglCreateContext(SdiGetDC())); 
				attr::outRC = SdiGetGLRC();
			}
			else
			{
				outRC = SdiGetGLRC();
			}
		}
		else
		{
			SdiSetGLRC(outRC);
		}


		// Make  SDI output GL context current.
		SdiMakeCurrent();

		CHECK_OGL_ERROR;

		return true;
	}




	///////////////////////////////////////////////////////////////////////
	/// Setup opengl dependencies for sdi capture
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API bool SdiOutputSetupGL()
	{
		if(!loadPresentVideoExtension() || !loadAffinityExtension() 
			|| !loadFramebufferObjectExtension() || !loadTimerQueryExtension() || !loadSwapIntervalExtension())
		{
			SdiLog() << "Couldn't load required OpenGL extensions." << std::endl;
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
		int numDevices = wglEnumerateVideoDevicesNV(SdiGetDC(), NULL);

		if (numDevices <= 0) 
		{
			SdiLog() << "wglEnumerateVideoDevicesNV() did not return any devices." << std::endl;
			return false;
		}

		attr::pVideoDevices = (HVIDEOOUTPUTDEVICENV *)malloc(numDevices * sizeof(HVIDEOOUTPUTDEVICENV));

		if (!attr::pVideoDevices) 
		{
			SdiLog() << "malloc failed.  OOM?" << std::endl;
			return false;
		}

		if (numDevices != wglEnumerateVideoDevicesNV(SdiGetDC(), attr::pVideoDevices)) 
		{
			free(attr::pVideoDevices);
			SdiLog() << "Inconsistent results from wglEnumerateVideoDevicesNV()" << std::endl;
			return false;
		}

		attr::presentFrame.Initialize();

		return true;
	}


	///////////////////////////////////////////////////////////////////////
	/// Cleanup the opengl stuff used in sdi capture
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void SdiOutputCleanupGL()
	{
		SdiMakeCurrent();

		// Free video devices
		free(attr::pVideoDevices);

		attr::presentFrame.Uninitialize();

		// Destroy objects
		SdiOutputUninitializeFbo();

		SdiOutputDestroyTextures();
		
		wglDeleteDCNV(SdiGetAffinityDC());

		if (attr::outRC != NULL)
		{
			wglDeleteContext(attr::outRC);		
			SdiSetGLRC(NULL);
			attr::outRC = NULL;
		}
	}


	///////////////////////////////////////////////////////////////////////
	/// Bind video device for opengl sdi
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API bool SdiOutputBindVideo()
	{
		//Bind the first device found that is connected to the output GPU
		if (!wglBindVideoDeviceNV(SdiGetDC(), 1, attr::pVideoDevices[0], NULL)) 
		{
			SdiLog() << "Failed to bind a videoDevice to slot 0." << std::endl;
			return false;
		}
		return true;
	}


	///////////////////////////////////////////////////////////////////////
	/// Unbind video device for opengl sdi
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API bool SdiOutputUnbindVideo()
	{
		if (!wglBindVideoDeviceNV(SdiGetDC(), 1, NULL, NULL)) 
		{
			SdiLog() << "Failed to unbind NULL videoDevice to slot 0." << std::endl;
			return false;
		}	
		return true;
	}



	///////////////////////////////////////////////////////////////////////
	/// Start sdi video output pipeline
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API bool SdiOutputStart()
	{
		attr::isPresentingFrames = (attr::sdiOut.Start() == S_OK);
		return attr::isPresentingFrames;
	}


	///////////////////////////////////////////////////////////////////////
	/// Stop sdi video output pipeline
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API bool SdiOutputStop()
	{
		attr::isPresentingFrames = (attr::sdiOut.Stop() == S_OK);
		return attr::isPresentingFrames;
	}

	/// Return true if the system is running properly
	GLNVSDI_API bool SdiOutputIsPresentingFrames()
	{
		return attr::isPresentingFrames;
	}

	/// Return true if the system was setup interlaced output
	GLNVSDI_API bool SdiOutputIsInterlaced()
	{
		return attr::sdiOut.IsInterlaced();
	}




	///////////////////////////////////////////////////////////////////////
	/// Bind fbo to begin render
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void SdiOutputBeginRender(int index, int field)
	{
		attr::fbo[index].BeginRender(field);
	}

	///////////////////////////////////////////////////////////////////////
	/// Unbind fbo to finish render
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void SdiOutputEndRender(int index, int field)
	{
		attr::fbo[index].EndRender(field);
	}




	
	///////////////////////////////////////////////////////////////////////
	/// Send the current frame to sdi output
	///////////////////////////////////////////////////////////////////////
	GLNVSDI_API void SdiOutputPresentFrame()
	{
		const bool dual_output = SdiGlobalOptions().IsDualOutput();

		int tex0 = 0;
		int tex1 = 1;
		int tex2 = 2;
		int tex3 = 3;

		if (SdiGlobalOptions().InvertFields())
		{
			int tex0 = 1;
			int tex1 = 0;
			int tex2 = 3;
			int tex3 = 2;
		}

		if (attr::sdiOut.IsInterlaced())
		{
			if (dual_output)
				attr::presentFrame.PresentFrameDual(SdiOutputGetTextureType(0), 
													SdiOutputGetTextureId(tex0), SdiOutputGetTextureId(tex1),
													SdiOutputGetTextureId(tex2), SdiOutputGetTextureId(tex3));
			else
				attr::presentFrame.PresentFrame(SdiOutputGetTextureType(0), SdiOutputGetTextureId(tex0), SdiOutputGetTextureId(tex1));
		}
		else
		{
			if (dual_output)
				attr::presentFrame.PresentFrameDual(SdiOutputGetTextureType(0), SdiOutputGetTextureId(tex0), SdiOutputGetTextureId(tex1));
			else
				attr::presentFrame.PresentFrame(SdiOutputGetTextureType(0), SdiOutputGetTextureId(0));
		}

		const int duplicated_frames = attr::presentFrame.GetStats().durationTime - 1;
		if (duplicated_frames > 0)
			std::cout << "Duplicated Frames: " << duplicated_frames << std::endl;

		attr::duplicateFramesCount += duplicated_frames;
	}




	static void UNITY_INTERFACE_API OnSdiOutputRenderEvent(int render_event_id)
	{
		switch (static_cast<SdiRenderEvent>(render_event_id))
		{
			case SdiRenderEvent::PresentFrame:
			{
				SdiMakeCurrent();
				SdiOutputPresentFrame();
				
				break;
			}

			case SdiRenderEvent::Initialize:
			{
				//SdiSetupLogFile();
				SdiSetCurrentDC();
				SdiSetCurrentGLRC();

				if (!SdiOutputInitialize())
				{
				}

				break;
			}

			case SdiRenderEvent::Setup:
			{
				//SdiOutputSetGlobalOptions();
				//SdiOutputSetVideoFormat(SdiVideoFormat::HD_1080I_59_94, SdiSyncSource::NONE, 0, 0, false, 5);

				if (!SdiOutputSetupDevices())
				{
					//UnityEngine.Debug.LogError("GLNvSdi_Plugin: " + UtyGLNvSdi.SdiGetLog());
					//return false;
				}

				SdiMakeCurrent();

				if (!SdiOutputSetupGL())
				{
					//UnityEngine.Debug.LogError("GLNvSdi_Plugin: " + UtyGLNvSdi.SdiGetLog());
					SdiOutputCleanupDevices();
					//return false;
				}

				if (!SdiOutputBindVideo())
				{
					//UnityEngine.Debug.LogError("GLNvSdi_Plugin: " + UtyGLNvSdi.SdiGetLog());
					SdiOutputCleanupDevices();
					//return false;
				}

				if (!SdiOutputStart())
				{
				}

				break;
			}


			case SdiRenderEvent::Shutdown:
			{
				SdiOutputStop();
				SdiOutputUnbindVideo();
				SdiOutputCleanupGL();
				SdiOutputCleanupDevices();
				SdiOutputUninitialize();

				break;
			}
		}

	}

	// --------------------------------------------------------------------------
	// GetRenderEventFunc, an example function we export which is used to get a rendering event callback function.
	UnityRenderingEvent GLNVSDI_API UNITY_INTERFACE_API GetSdiOutputRenderEventFunc()
	{
		return OnSdiOutputRenderEvent;
	}


};	//extern "C"
