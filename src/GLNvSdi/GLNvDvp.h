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
	UpdateInput,
	Update,
	UpdateOutput,
	Cleanup,
	Uninitialize
};


enum class DvpSyncSource
{
	SDI_SYNC,
	COMP_SYNC,
	NONE
};

enum class DvpVideoFormat
{
	HD_1080P_29_97,
	HD_1080P_30_00,
	HD_1080I_59_94,
	HD_1080I_60_00,
	HD_720P_29_97,
	HD_720P_30_00,
	HD_720P_59_94,
	HD_720P_60_00,
	SD_487I_59_94
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
	GLNVSDI_API void DvpSetOptions(DvpSyncSource syncSource, int inputBufferSize, int outputBufferSize, int hDelay, int vDelay);

	GLNVSDI_API bool DvpCreateAffinityContext();
	GLNVSDI_API void DvpDestroyAffinityContext();
	GLNVSDI_API void DvpGetAffinityContext(HDC& hDC, HGLRC& hGLRC);
	GLNVSDI_API void DvpSetAffinityContext(HDC _hDC = nullptr, HGLRC _hGLRC = nullptr);
	GLNVSDI_API void DvpSetExternalContext(HDC _hDC = nullptr, HGLRC _hGLRC = nullptr);
	GLNVSDI_API BOOL DvpMakeAffinityCurrent();
	GLNVSDI_API BOOL DvpMakeExternalCurrent();

	GLNVSDI_API int  DvpInputActiveDeviceCount();
	GLNVSDI_API int  DvpInputStreamsPerFrame(int device_index = 0);

	GLNVSDI_API int  DvpInputWidth();
	GLNVSDI_API int  DvpInputHeight();
	GLNVSDI_API bool DvpInputCreateTextures(int videoWidth, int videoHeight);
	
	GLNVSDI_API void DvpInputSetTexture(GLuint id, int device_index = 0, int video_stream_index = 0);
	GLNVSDI_API void DvpInputSetTexturePtr(void* texturePtr, int device_index = 0, int video_stream_index = 0);
	GLNVSDI_API gl::Texture2D* DvpInputDisplayTexture(int device_index = 0, int video_stream_index = 0);

	GLNVSDI_API void DvpOutputSetTexture(int index, GLuint id);
	GLNVSDI_API void DvpOutputSetTexturePtr(void* texturePtr, int index);

	GLNVSDI_API C_DVP*	 DvpInputPtr();
	GLNVSDI_API C_Frame* DvpInputFrame(int device_index = 0);
	GLNVSDI_API C_Frame* DvpInputPreviousFrame(int device_index = 0);
	GLNVSDI_API uint64_t DvpInputDroppedFrames(int device_index = 0);
	GLNVSDI_API uint64_t DvpDroppedFrames(int device_index = 0);
	GLNVSDI_API NVVIOSIGNALFORMAT DvpInputSignalFormat();
	GLNVSDI_API DvpVideoFormat DvpInputVideoFormat();

		
	UnityRenderingEvent GLNVSDI_API UNITY_INTERFACE_API GetGLNvDvpRenderEventFunc();
};

#endif	//__GL_NV_DVP_H__
