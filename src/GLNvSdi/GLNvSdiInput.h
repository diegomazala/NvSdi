#ifndef __GL_NV_SDI_INPUT_H__
#define __GL_NV_SDI_INPUT_H__

#include "GLNvSdi.h"


enum SdiInputRenderEvent
{
	Initialize,
	Setup,
	StartCapture,
	StopCapture,
	Render,
	Shutdown
};

extern "C"
{
	
	/// Return the Sdi Input object
	GLNVSDI_API CNvSDIin* SdiInput();


	/// Set global sdi configuration options for input sdi
	GLNVSDI_API void SdiInputSetGlobalOptions(int ringBufferSizeInFrames);

	

	/// Return video input width
	GLNVSDI_API int SdiInputWidth();

	/// Return video input height
	GLNVSDI_API int SdiInputHeight();


	/// Return the stream count
	GLNVSDI_API int SdiInputVideoCount();

	
	/// Return the time spent by gpu
	GLNVSDI_API float SdiInputGpuTime();

	/// Return the time spent by gvi
	GLNVSDI_API float SdiInputGviTime();

	/// Return the number of input dropped frames in the last update
	GLNVSDI_API int SdiInputDroppedFrames();

	/// Return the count of input dropped frames in the last run
	GLNVSDI_API int SdiInputDroppedFramesCount();

	/// Reset the count of input dropped frames in the last run
	GLNVSDI_API void SdiInputResetDroppedFramesCount();


	/// Return the a pointer to a video input texture
	GLNVSDI_API gl::Texture2D* SdiInputGetTexture(int index = 0);

	/// Return the opengl handle for a video input texture
	GLNVSDI_API GLuint SdiInputGetTextureId(int index = 0);

	/// Return the opengl type for a video input texture
	GLNVSDI_API GLenum SdiInputGetTextureType(int index = 0);

	/// Set the opengl texture for a video input channel
	GLNVSDI_API void SdiInputSetTexture(int index, GLuint id);
	GLNVSDI_API void SdiInputSetTexturePtr(int index, void* texturePtr, int w, int h);


	/// Create textures for sdi input
	GLNVSDI_API bool SdiInputCreateTextures(int count, int width, int height);
	
	/// Destroy the textures used for sdi input
	GLNVSDI_API void SdiInputDestroyTextures();


	/// Restart the NvSdi Input according to options set previously
	//GLNVSDI_API bool SdiInputRestart();
	
	/// Initialize the NvSdi Input according to options set previously
	GLNVSDI_API bool SdiInputInitialize();

	/// Uninitialize the sdi input pipeline
	GLNVSDI_API void SdiInputUninitialize();



	/// Configure the sdi devices for startup
	GLNVSDI_API bool SdiInputSetupDevices();

	/// Empty
	GLNVSDI_API void SdiInputCleanupDevices();



	/// Setup opengl context for sdi capture
	GLNVSDI_API bool SdiInputSetupContextGL(HDC hDC, HGLRC hRC);
	
	/// Setup opengl dependencies for sdi capture
	GLNVSDI_API bool SdiInputSetupGL();

	/// Cleanup the opengl stuff used in sdi capture
	GLNVSDI_API void SdiInputCleanupGL();



	/// Bind textures and device for sdi input
	GLNVSDI_API bool SdiInputBindVideoTextureFrame();

	/// Unbind textures and device for sdi input
	GLNVSDI_API bool SdiInputUnbindVideoTextureFrame();


	/// Bind textures and device for sdi input
	GLNVSDI_API bool SdiInputBindVideoTextureField();

	/// Unbind textures and device for sdi input
	GLNVSDI_API bool SdiInputUnbindVideoTextureField();

	/// Check if it is capturing
	GLNVSDI_API bool SdiInputIsCapturing();

	/// Start the sdi capture
	GLNVSDI_API bool SdiInputStart();

	/// Stop the sdi capture
	GLNVSDI_API void SdiInputStop();

	/// Capture the current frame for all sdi inputs available
	GLNVSDI_API GLenum SdiInputCaptureVideo();

	/// Return Sdi capture status (GL_SUCCESS_NV, GL_FAILURE_NV, GL_PARTIAL_SUCCESS_NV)
	GLNVSDI_API int SdiInputCaptureStatus();

	/// Function called by Unity through rendering events
	UnityRenderingEvent GLNVSDI_API UNITY_INTERFACE_API GetSdiInputRenderEventFunc();
};

#endif	//__GL_NV_SDI_INPUT_H__
