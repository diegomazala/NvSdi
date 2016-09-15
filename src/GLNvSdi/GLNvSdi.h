#ifndef __GL_NV_SDI_H__
#define __GL_NV_SDI_H__



enum SdiRenderEvent
{
	PreInitialize,
	Initialize,
	Setup,
	StartCapture,
	StopCapture,
	CaptureFrame,
	PresentFrame,
	Shutdown
};


#include "UnityPlugin.h"
#include "IUnityGraphics.h"

#include "GLNvSdiCore.h"

#include "nvGPUutil.h"
#include "nvSDIin.h"
#include "nvSDIout.h"
#include "SdiOptions.h"
#include "Logger.h"

#include "GLNvSdiInput.h"
#include "GLNvSdiOutput.h"


static int sdiError = 0;

extern "C"
{
	GLNVSDI_API int SdiError();

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

	
};

#endif	//__GL_NV_SDI_H__
