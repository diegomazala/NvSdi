#ifndef __GL_NV_SDI_CORE_H__
#define __GL_NV_SDI_CORE_H__



#ifdef GLNVSDI_EXPORTS
#define GLNVSDI_API __declspec(dllexport)
#else
#define GLNVSDI_API __declspec(dllimport)
#endif


#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <windows.h>
#include <assert.h>
#include <stdio.h>

#include <string>
#include <iostream>
#include <fstream>

#include <ctime>
#include <iostream>
#include <sstream>
#include <exception>

#include "EnumString.h"
#include "OpenGL.h"
#include "Texture.h"
#include "OpenGLExt.h"


#ifdef USE_NVAPI
#include "NvApi/nvapi.h"
#else
#include "NvGvoApi.h"
#include "NvApiError.h"
#endif


// Constants
#define NUM_QUERIES 5

#define MAX_GPUS 4
#define MAX_CAPTURE_CARDS 4
#define MAX_OUTPUT_CARDS  4
typedef struct GLNVSDI_API HNVGPUVIO
{
	HGPUNV hGpu; //GPU affinity handle
	NvVioHandle vioHandle; //VIO handle	
	bool hasSDIOutput;	
	GPU_DEVICE gpuDevice; // gdi display device structure
	bool hasDisplay;
	bool isPrimary;
}HNVGPUVIO;



namespace nv
{
	struct GLNVSDI_API exception : public std::exception
	{
		explicit exception(const char* msg): std::exception((std::string("Nv_Error: ")+msg).c_str()) {}
	};
}


#ifdef _DEBUG 
#define CHECK_WGL_ERROR(x) {        \
	BOOL retval = (x);              \
	if (!retval) {                  \
	printf("%s Error\n", #x);   \
	}                               \
}
#else
#define CHECK_WGL_ERROR(x) {        \
	(x);                            \
}                                   
#endif

#ifdef _DEBUG
#define CHECK_OGL_ERROR {               \
	assert(glGetError() == GL_NO_ERROR); \
}
#else
#define CHECK_OGL_ERROR {               \
}
#endif




#endif	//__GL_NV_SDI_CORE_H__
