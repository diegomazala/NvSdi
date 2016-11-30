#ifndef __GL_NV_SDI_OUTPUT_H__
#define __GL_NV_SDI_OUTPUT_H__

#include "GLNvSdi.h"


extern "C"
{

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
	GLNVSDI_API void SdiOutputSetTexturePtr(int index, void* texturePtr, int w, int h);

	/// Return a handle to output rendering context
	GLNVSDI_API HGLRC SdiOutputRC();


	/// Return video input width
	GLNVSDI_API int SdiOutputWidth();

	/// Return video input height
	GLNVSDI_API int SdiOutputHeight();


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

	/// Return true if the system is running properly
	GLNVSDI_API bool SdiOutputIsPresentingFrames();

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

	/// Function called by Unity through rendering events
	UnityRenderingEvent GLNVSDI_API UNITY_INTERFACE_API GetSdiOutputRenderEventFunc();

};

#endif	//__GL_NV_SDI_OUTPUT_H__
