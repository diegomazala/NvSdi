#ifndef __GL_NV_DVP_H__
#define __GL_NV_DVP_H__



#include "UnityPlugin.h"
#include "IUnityGraphics.h"

#include "GLNvSdiCore.h"

#include "nvGPUutil.h"
#include "nvSDIin.h"
#include "nvSDIout.h"
#include "SdiOptions.h"
#include "Logger.h"

class C_DVP;
class C_Frame;


enum class DvpRenderEvent
{
	CheckAvalability,
	Initialize,
	Setup,
	Update,
	Cleanup,
	Uninitialize
};


extern "C"
{
	GLNVSDI_API bool DvpIsOk();

	GLNVSDI_API bool DvpInputIsAvailable();
	GLNVSDI_API bool DvpOutputIsAvailable();

	GLNVSDI_API void DvpInputDisable();
	GLNVSDI_API void DvpOutputDisable();

	GLNVSDI_API bool DvpCheckAvailability();
	GLNVSDI_API void DvpSetGlobalOptions();

	GLNVSDI_API bool DvpCreateAffinityContext();
	GLNVSDI_API void DvpDestroyAffinityContext();
	GLNVSDI_API void DvpGetAffinityContext(HDC& hDC, HGLRC& hGLRC);
	GLNVSDI_API void DvpSetAffinityContext(HDC _hDC = nullptr, HGLRC _hGLRC = nullptr);
	GLNVSDI_API void DvpSetExternalContext(HDC _hDC = nullptr, HGLRC _hGLRC = nullptr);
	GLNVSDI_API bool DvpMakeAffinityCurrent();
	GLNVSDI_API bool DvpMakeExternalCurrent();

	GLNVSDI_API int  DvpInputActiveDeviceCount();
	GLNVSDI_API int  DvpInputStreamsPerFrame(int device_index = 0);

	GLNVSDI_API int  DvpInputWidth();
	GLNVSDI_API int  DvpInputHeight();
	GLNVSDI_API bool DvpInputCreateTextures(int videoWidth, int videoHeight);
	
	GLNVSDI_API void DvpInputSetTexture(GLuint id, int device_index = 0, int video_stream_index = 0);
	GLNVSDI_API void DvpInputSetTexturePtr(void* texturePtr, int device_index = 0, int video_stream_index = 0);
	GLNVSDI_API gl::Texture2D* DvpInputDisplayTexture(int device_index = 0, int video_stream_index = 0);

	GLNVSDI_API void DvpOutputSetTexture(int index, GLuint id);
	GLNVSDI_API void DvpOutputSetTexturePtr(int index, void* texturePtr, int w, int h);

	GLNVSDI_API C_DVP*	 DvpInputPtr();
	GLNVSDI_API C_Frame* DvpInputFrame(int device_index = 0);
	GLNVSDI_API C_Frame* DvpInputPreviousFrame(int device_index = 0);
	GLNVSDI_API uint64_t DvpInputDroppedFrames(int device_index = 0);
	GLNVSDI_API NVVIOSIGNALFORMAT DvpInputSignalFormat();

	
		
	UnityRenderingEvent GLNVSDI_API UNITY_INTERFACE_API GetGLNvDvpRenderEventFunc();
};

#endif	//__GL_NV_DVP_H__
