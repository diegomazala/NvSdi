
//*******************************************************************************
//multiCaptureAsync: 
//Performs an asynchronous(to rendering)capture of video and ancillary data from all the capture cards 
//present in the system  
//The video feeds are displayed on the main GPU. 
//The capture and rendering are done in separate threads 
//The communication between the capture thread and the render is buffered.
//*******************************************************************************

#ifndef _DVP_H_
#define _DVP_H_


#include <windows.h>
#include <assert.h>
#include <stdio.h>

#include <GL/gl.h>


#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/wglext.h>
#include <cuda.h>
#include <cudaGL.h>
#include "nvSDIin.h"
#include "CommandLine.h"

#include "ANCapi.h"

//threading utilities
#include "platform.h"

#include <deque>
#include <map>


#undef USE_ALL_STREAMS
#define NUM_VIDEO_STREAMS 4
//#define USE_ALL_STREAMS

//=========================C_Frame============================================================
//A struct to encapsulate all the captured data that will get passed around in the application
//============================================================================================

typedef class C_Frame
{
private:
	int numStreamsPerFrame;
	int videoWidth;
	int videoHeight;	
	int videoPitch;
	NvU32  sequenceLength;                // Audio sample sequence length
	NvU32 *numAudioSamples;		        // Number of audio samples
	CNvSDIin *sdiIn;
	HGLRC srcRC;
	HGLRC dstRC;
	HDC srcDC;
	HDC dstDC;		
	CUcontext cuSrcContext;	
	CUcontext cuDstContext;	
	CUgraphicsResource cuSrcVideoObjects[MAX_VIDEO_STREAMS];
	CUdeviceptr cuSrcDevicePtr[MAX_VIDEO_STREAMS];	
	
	CUgraphicsResource cuDstVideoObjects[MAX_VIDEO_STREAMS];
	CUdeviceptr cuDstDevicePtr[MAX_VIDEO_STREAMS];	
	
	unsigned char *hostVideoObject[MAX_VIDEO_STREAMS];

	GLuint srcVideoObjects[MAX_VIDEO_STREAMS];
	CUevent cuEventUploadFinished[MAX_VIDEO_STREAMS];
	GLuint dstVideoObjects[MAX_VIDEO_STREAMS];
	CUevent cuEventTotalUploadFinished;
	CUevent cuEventTotalUploadStarted;
	CUevent cuEventTotalDownloadFinished;
	CUevent cuEventTotalDownloadStarted;

	bool makeAllSrcCtxCurrent();
	bool makeAllDstCtxCurrent();

	bool makeAllSrcCtxNotCurrent();
	bool makeAllDstCtxNotCurrent();
	
public:
	
	NVVIOANCDATAFRAME ancData[MAX_VIDEO_STREAMS];  
	GLuint64EXT captureTime;
	GLuint sequenceNum;  	
	GLuint numDroppedFrames;	
	float captureElapsedTime;	
	float uploadElapsedTime;
	float downloadElapsedTime;
	C_Frame();
	~C_Frame();
	NvU32 init(CNvSDIin *SDIin, HDC captureDC, HGLRC captureRC, CUcontext captureCUctx, HDC renderDC, HGLRC renderRC,  CUcontext renderCUctx);	
	NvU32 deinit();
	NvU32 bindSrcObjects();	
	NvU32 unbindSrcObjects();	
	GLuint getDstObject(int i);
	//NvU32 update();		
	NvU32 sync();		
	int getWidth(){return videoWidth;}
	int getHeight(){return videoHeight;}
	int getPitch(){return videoPitch;}
}C_Frame;


//=========================C_RingBuffer=======================================================
//A struct to encapsulate all the captured data that will get passed around in the application
//This is a ring buffer between the rendering thread and the capture thread that contains:
//-copy of video objects in the drawing context
//-anc data 
//There is only one consumer and one producer
//If one of the threads is slower, the other thread does not wait
//============================================================================================
#define NUM_RING_BUFFER_FRAMES 3
typedef class C_RingBuffer
{
private:
	C_Frame *m_Frames[NUM_RING_BUFFER_FRAMES];
	std::deque<C_Frame *> m_Repository;
	std::deque<C_Frame *> m_Pending;
    
    Lock m_hLock;        // Synchronization lock
	
public:
	C_RingBuffer();
	~C_RingBuffer();
	// Member Functions
    NvU32 init(CNvSDIin *SDIin, HDC captureDC, HGLRC captureRC,CUcontext captureCUctx, HDC renderDC, HGLRC renderRC,CUcontext renderCUctx);
    NvU32 deinit();
    C_Frame *getPendingFrame();
	C_Frame *getRepositoryFrame();
    NvU32 releaseFrame(C_Frame *frame);
	NvU32 addFrame(C_Frame *frame);
}C_RingBuffer;

//=========================C_DVP===============================================
//A class to encapsulate the functionality of a digital video pipeline
//=============================================================================

typedef class C_DVP
{
protected:
	
	CNvSDIin m_SDIin[NVAPI_MAX_VIO_DEVICES];
	
	C_RingBuffer *m_CaptureRingBuffer[NVAPI_MAX_VIO_DEVICES];
	HDC	m_hCaptureDC;								// Device context
	HGLRC m_hCaptureRC;							// OpenGL rendering context	
	HDC	m_hRenderDC;								// Device context
	HGLRC m_hRenderRC;							// OpenGL rendering context	
	HGPUNV m_hRenderGPU;
	HGPUNV m_hCaptureGPU;
	CUcontext m_cuRenderCtx;								// Device context
	CUcontext m_cuCaptureCtx;								// Device context
	CUdevice m_cuRenderDevice;
	CUdevice m_cuCaptureDevice;
	static void CaptureThreadWrapper(void *thisPtr) { ((C_DVP *)thisPtr)->CaptureThread(); };
	void CaptureThread();
	Thread m_CaptureThreadID;		
	bool m_QuitCapture;
	int m_activeDeviceCount;
	GLboolean setupGL();
	GLboolean cleanupGL();	
	CUresult  setupCUDA();	
	CUresult cleanupCUDA();	
	bool makeAllCaptureCtxCurrent();
	bool makeAllRenderCtxCurrent();

	bool makeAllCaptureCtxNotCurrent();
	bool makeAllRenderCtxNotCurrent();
	
public:
	Options m_options;
	C_DVP();
	~C_DVP();
	C_Frame *GetFrame(int deviceIndex);
	void ReleaseUsedFrame(int deviceIndex ,C_Frame *frame);
	HRESULT SetupSDIPipeline();
	HRESULT CleanupSDIPipeline();
	HRESULT StartSDIPipeline();
	HRESULT StopSDIPipeline();
	HRESULT SetupSDIinGL(HDC renderDC, HGLRC renderRC);	
	HRESULT CleanupSDIinGL();

	int GetVideoHeight();
	int GetVideoWidth();	
	int GetNumStreamsPerFrame(int deviceIndex);
	NVVIOSIGNALFORMAT GetSignalFormat();
	int GetActiveDeviceCount(){return m_activeDeviceCount;}
	int GetDeviceNumber(int activeDeviceIndex);

	GLuint decodeShader;                    // Shader code to unpack buffers
	GLuint decodeProgram;                   // Shader program that unpacks the buffers
	HRESULT SetupDecodeProgram();
	HRESULT DestroyDecodeProgram();
	HRESULT CompileShader(GLuint shader, const char *shaderStr);


	GLuint m_DisplayTextures[NVAPI_MAX_VIO_DEVICES][MAX_VIDEO_STREAMS];
	GLuint m_decodeTextures[NVAPI_MAX_VIO_DEVICES][MAX_VIDEO_STREAMS];  // The raw SDI data from the captured buffer
	// needs to be copied to a texture, so that
	// the data can be read by a shader and
	// processed for display in OpenGL. For 
	// example the shader may do 422->444 expansion	

	GLuint m_vidFbos[NVAPI_MAX_VIO_DEVICES][MAX_VIDEO_STREAMS];    // When capturing to buffers, the decode shader
	// will fill the video textures with color data.
	// A frame buffer object is required to peform 
	// this off screen rendering.

}C_DVP;

//=========================C_multiCaptureAsync==============================================
//The main class that's responsible to orchestrate the capture and display
//==========================================================================================

#endif	// _DVP_H_
