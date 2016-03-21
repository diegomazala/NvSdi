#ifndef __GL_NV_SDI_new_H__
#define __GL_NV_SDI_new_H__

#include "GLNvSdiCore.h"

#include "nvGPUutil.h"
#include "nvSDIin.h"
#include "nvSDIout.h"
#include "SdiOptions.h"
#include "Logger.h"





extern "C"
{
	GLNVSDI_API std::stringstream& SdiLog();

	GLNVSDI_API void SdiClearLog();
	GLNVSDI_API void SdiGetLog(void* log_str, int max_length);

	GLNVSDI_API void SdiSetupLogConsole();
	GLNVSDI_API void SdiSetupLogFile();

	
	/// Set the opengl rendering context. If the parameter is null, get the current context
	GLNVSDI_API void  SdiSetGLRC(HGLRC hglrc = NULL);
	GLNVSDI_API void  SdiSetCurrentGLRC();
	
	/// Return the rendering context that is being used for sdi.
	GLNVSDI_API HGLRC SdiGetGLRC();


	GLNVSDI_API void SdiSetAffinityDC(HDC hdc);
	GLNVSDI_API HDC  SdiGetAffinityDC();


	/// Set the device context. If the parameter is null, get the current context
	GLNVSDI_API void SdiSetDC(HDC hdc = NULL);
	GLNVSDI_API void SdiSetCurrentDC();
	
	/// Return the device context that is being used for sdi.
	GLNVSDI_API HDC  SdiGetDC();


	GLNVSDI_API int SdiSetupPixelFormat(HDC hDC);

	GLNVSDI_API bool SdiMakeCurrent();


	GLNVSDI_API void SdiSetCurrentDC();
	GLNVSDI_API void SdiSetUtyGLRC();
	GLNVSDI_API bool SdiMakeUtyCurrent();


	/// Get global sdi configuration options
	GLNVSDI_API SdiOptions& SdiGlobalOptions();


	/// Return the number of devices
	GLNVSDI_API int SdiGpuCount();

	/// Return the number of input devices
	GLNVSDI_API int SdiInputGpuCount();

	/// Return the number of output devices
	GLNVSDI_API int SdiOutputGpuCount();



	GLNVSDI_API bool SdiAncSetupInput();
	GLNVSDI_API void SdiAncCleanupInput();
	GLNVSDI_API bool SdiAncCapture();

	GLNVSDI_API bool SdiAncSetupOutput();
	GLNVSDI_API void SdiAncCleanupOutput();
	GLNVSDI_API void SdiAncPresent();

	GLNVSDI_API bool SdiAncGetTimeCode(void* timecode_int8, int video_index);


	
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	/////////////////////////////// INPUT ////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////


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



	/// Start the sdi capture
	GLNVSDI_API bool SdiInputStart();

	/// Stop the sdi capture
	GLNVSDI_API void SdiInputStop();

	/// Capture the current frame for all sdi inputs available
	GLNVSDI_API GLenum SdiInputCaptureVideo();



	

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	////////////////////////////// OUTPUT ////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////


	/// Return the Sdi Output object
	GLNVSDI_API CNvSDIout* SdiOutput();

	/// Invert the order to present the textures on sdi ouput
	GLNVSDI_API void SdiOutputInvertFields(bool invert);

	/// Set global sdi configuration options for output sdi
	GLNVSDI_API void SdiOutputSetGlobalOptions();

	GLNVSDI_API void SdiOutputSetVideoFormat(SdiVideoFormat video_format, SdiSyncSource sync_source, int h_delay, int v_delay, bool dual_output, int flip_queue_lenght);

	/// Return a texture
	GLNVSDI_API gl::Texture2D* SdiOutputGetTexture(int index = 0);

	/// Return a texture id
	GLNVSDI_API GLuint SdiOutputGetTextureId(int index = 0);

	/// Return a texture id
	GLNVSDI_API GLenum SdiOutputGetTextureType(int index = 0);

	/// Set a texture
	GLNVSDI_API void SdiOutputSetTexture(int index, GLuint id);

	/// Return a handle to output rendering context
	GLNVSDI_API HGLRC SdiOutputRC();


	/// Return video input width
	GLNVSDI_API int SdiOutputWidth();

	/// Return video input height
	GLNVSDI_API int SdiOutputHeight();


	/// Setup a pixel format for outpu sdi
	//GLNVSDI_API int SdiOutputSetupPixelFormat(HDC hDC);


	/// Initialize the NvSdi Input according to options set previously
	GLNVSDI_API bool SdiOutputInitialize();

	/// Uninitialize the sdi input pipeline
	GLNVSDI_API void SdiOutputUninitialize();



	/// Configure the sdi devices for startup
	GLNVSDI_API bool SdiOutputSetupDevices();

	/// Empty
	GLNVSDI_API void SdiOutputCleanupDevices();



	/// Setup opengl context for sdi capture
	GLNVSDI_API bool SdiOutputSetupContextGL(HDC hDC = NULL, HGLRC outRC = NULL);

	/// Setup opengl dependencies for sdi capture
	GLNVSDI_API bool SdiOutputSetupGL();

	/// Cleanup the opengl stuff used in sdi capture
	GLNVSDI_API void SdiOutputCleanupGL();


	/// Bind video device to opengl sdi
	GLNVSDI_API bool SdiOutputBindVideo();
	
	/// Unbind video device to opengl sdi
	GLNVSDI_API bool SdiOutputUnbindVideo();


	/// Start output sending sdi
	GLNVSDI_API bool SdiOutputStart();

	/// Stop output sending sdi
	GLNVSDI_API bool SdiOutputStop();


	/// Return true if the system was setup interlaced output
	GLNVSDI_API bool SdiOutputIsInterlaced();

	/// Return true if the system was setup for dual output
	//GLNVSDI_API bool SdiOutputIsDual();

	/// Setup for dual output
	//GLNVSDI_API void SdiOutputSetDual(bool dual=true);



	/// Create textures for output 
	GLNVSDI_API void SdiOutputCreateTextures();

	/// Destroy textures used for output 
	GLNVSDI_API void SdiOutputDestroyTextures();




	/// Setup fbo for output 
	GLNVSDI_API void SdiOutputInitializeFbo();

	/// Destroy fbo used for output 
	GLNVSDI_API void SdiOutputUninitializeFbo();





	/// Bind fbo to begin render
	GLNVSDI_API void SdiOutputBeginRender(int index = 0, int field = 0);

	/// Unbind fbo to finish render
	GLNVSDI_API void SdiOutputEndRender(int index = 0, int field = 0);



	/// Send the current frame to sdi output
	GLNVSDI_API void SdiOutputPresentFrame();


	/// Return the number of duplicated frames in the last update 
	GLNVSDI_API int SdiOutputDuplicatedFrames();

	/// Return the count of duplicated frames in the last run
	GLNVSDI_API int SdiOutputDuplicatedFramesCount();

	/// Reset the count of duplicated frames in the last run
	GLNVSDI_API void SdiOutputResetDuplicatedFramesCount();

};

#endif	//__GL_NV_SDI_new_H__