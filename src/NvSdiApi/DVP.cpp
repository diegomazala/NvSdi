
//*******************************************************************************
//multiCaptureAsync: 
//Performs an asynchronous(to rendering)capture of video and ancillary data from all the capture cards 
//present in the system  
//The video feeds are displayed on the main GPU. 
//The capture and rendering are done in separate threads 
//The communication between the capture thread and the render is buffered.
//*******************************************************************************


#include "DVP.h"
#include "nvGPUutil.h"
#include "nvSDIutil.h"
#include "glExtensions.h"
#include "error.h"
#include "GLNvSdi.h"

//=========================C_Frame============================================================
//A struct to encapsulate all the captured data that will get passed around in the application
//============================================================================================




C_Frame::C_Frame()
{
	videoWidth = 0;
	videoHeight = 0;
	srcDC = NULL;
	srcRC = NULL;
	dstDC = NULL;
	dstRC = NULL;
	numAudioSamples = NULL;
	sequenceLength = 0;
}
C_Frame::~C_Frame()
{

}
bool C_Frame::makeAllSrcCtxCurrent()
{
	wglMakeCurrent(srcDC,srcRC);	
	assert(glGetError() == GL_NO_ERROR);

	CUresult cerr;
	cerr = cuCtxPushCurrent(cuSrcContext);
	CheckError(cerr);	

	return true;
}

bool C_Frame::makeAllDstCtxCurrent()
{
	wglMakeCurrent(dstDC,dstRC);	
	assert(glGetError() == GL_NO_ERROR);
	CUresult cerr;
	cerr = cuCtxPushCurrent(cuDstContext);
	CheckError(cerr);	
	return true;
}

bool C_Frame::makeAllSrcCtxNotCurrent()
{
	CUresult cerr;	
	cerr = cuCtxPopCurrent(&cuSrcContext);
	CheckError(cerr);	
	wglMakeCurrent(NULL,NULL);	

	return true;
}

bool C_Frame::makeAllDstCtxNotCurrent()
{
	CUresult cerr;
	cerr = cuCtxPopCurrent(&cuDstContext);
	CheckError(cerr);	
	wglMakeCurrent(NULL,NULL);	
	return true;
}


NvU32  C_Frame::init(CNvSDIin *SDIin, HDC captureDC, HGLRC captureRC, CUcontext captureCUctx, HDC renderDC, HGLRC renderRC,  CUcontext renderCUctx)
{
	sdiIn = SDIin;
	numStreamsPerFrame = SDIin->GetNumStreams();	

#ifdef USE_ALL_STREAMS
	numStreamsPerFrame = NUM_VIDEO_STERAMS;	
#endif		

	srcDC = captureDC;
	srcRC = captureRC;
	dstDC = renderDC;
	dstRC = renderRC;
	cuSrcContext = captureCUctx;
	cuDstContext = renderCUctx;
	
	// Determine the length of the audio sample sequence.
	NvVIOANCAPI_NumAudioSamples(SDIin->GetVioHandle(), 
								NVVIOANCAUDIO_SAMPLING_RATE_48_0,
								(NvU32 *)&sequenceLength, 
								NULL);

	if (numAudioSamples) 
		free(numAudioSamples);

	numAudioSamples = (NvU32 *)calloc((size_t)sequenceLength, sizeof(NvU32));
	if(numAudioSamples == NULL)
		return FALSE;
	// Determine number of audio samples based on signal format
	// and audio sampling rate.
	NvVIOANCAPI_NumAudioSamples(SDIin->GetVioHandle(), 
								NVVIOANCAUDIO_SAMPLING_RATE_48_0,
								(NvU32 *)&sequenceLength, 
								(NvU32 *)numAudioSamples);
			
	// Determine the maximum number of audio sample for any given frame.
	// Use this value when allocating space to store audio samples
	NvU32 maxNumAudioSamples = 0;
	for (unsigned int i = 0; i < sequenceLength; i++) {
		if (numAudioSamples[i] > maxNumAudioSamples) {
			maxNumAudioSamples = numAudioSamples[i];
		}
	}


	for (int i = 0; i < numStreamsPerFrame; i++) {
		ancData[i].version = NVVIOANCDATAFRAME_VERSION;
		ancData[i].fields = NVVIOANCDATAFRAME_VITC | NVVIOANCDATAFRAME_CUSTOM;
		ancData[i].numCustomPackets = 255;
		ancData[i].CustomPackets = (NVVIOANCDATAPACKET *)calloc(ancData[i].numCustomPackets, sizeof(NVVIOANCDATAFRAME));
		if(	ancData[i].CustomPackets == NULL)
			return FALSE;
		// Allocate space required for audio data packets for each audio group, four channels per group.  
		// Space required depends upon signal format and audio rate.  Set bit field to indicate 
		// desired audio channels to capture.

		for (int j = 0; j < 4; j++) {
			ancData[i].AudioGroup1.numAudioSamples = maxNumAudioSamples;
			ancData[i].AudioGroup1.audioData[j] = (NvU32 *)calloc(maxNumAudioSamples, sizeof(NvU32));
			if(ancData[i].AudioGroup1.audioData[j] == NULL)
				return FALSE;
			ancData[i].fields |= NVVIOANCDATAFRAME_AUDIO_GROUP_1;

			ancData[i].AudioGroup2.numAudioSamples = maxNumAudioSamples;
			ancData[i].AudioGroup2.audioData[j] = (NvU32 *)calloc(maxNumAudioSamples, sizeof(NvU32));
			if(ancData[i].AudioGroup2.audioData[j] == NULL)
				return FALSE;
			ancData[i].fields |= NVVIOANCDATAFRAME_AUDIO_GROUP_2;

			ancData[i].AudioGroup3.numAudioSamples = maxNumAudioSamples;
			ancData[i].AudioGroup3.audioData[j] = (NvU32 *)calloc(maxNumAudioSamples, sizeof(NvU32));
			if(ancData[i].AudioGroup3.audioData[j] == NULL)
				return FALSE;
			ancData[i].fields |= NVVIOANCDATAFRAME_AUDIO_GROUP_3;

			ancData[i].AudioGroup4.numAudioSamples = maxNumAudioSamples;
			ancData[i].AudioGroup4.audioData[j] = (NvU32 *)calloc(maxNumAudioSamples, sizeof(NvU32));
			if(ancData[i].AudioGroup4.audioData[j] == NULL)
				return FALSE;
			ancData[i].fields |= NVVIOANCDATAFRAME_AUDIO_GROUP_4;			
		}
	}

	//create placeholders for the captured textures in the src context
	makeAllSrcCtxCurrent();

	videoWidth = SDIin->GetWidth();
	videoHeight = SDIin->GetHeight();
	
	glGenBuffers(numStreamsPerFrame, srcVideoObjects);	
	
	for (UINT i = 0; i < numStreamsPerFrame; i++) {	
		
		SDIin->BindVideoFrameBuffer(srcVideoObjects[i],GL_YCBYCR8_422_NV,i);
		videoPitch = SDIin->GetBufferObjectPitch(0);
		glBindBuffer(GL_VIDEO_BUFFER_NV, srcVideoObjects[i]);
		assert(glGetError() == GL_NO_ERROR);
		
		// Allocate required space in video capture buffer
		glBufferData(GL_VIDEO_BUFFER_NV,videoPitch * videoHeight,
				NULL, GL_STREAM_COPY);
		assert(glGetError() == GL_NO_ERROR);

				
	}

	CUresult cerr;
	for (unsigned int i = 0; i < numStreamsPerFrame; i++) {
		cerr = cuGraphicsGLRegisterBuffer(&cuSrcVideoObjects[i],srcVideoObjects[i],CU_GRAPHICS_MAP_RESOURCE_FLAGS_NONE);
		CheckError(cerr);	
	}
	
	for (unsigned int i = 0; i < numStreamsPerFrame; i++) {
		cerr = cuMemHostAlloc((void **)&hostVideoObject[i],videoPitch*videoHeight, CU_MEMHOSTALLOC_PORTABLE);//|CU_MEMHOSTALLOC_WRITECOMBINED);
		CheckError(cerr);	
	}
	//for (unsigned int i = 0; i < numStreamsPerFrame; i++) {
		//cerr = cuEventCreate(&cuEventUploadFinished[i],CU_EVENT_BLOCKING_SYNC);
		//CheckError(cerr);	
	//}
	cerr = cuEventCreate(&cuEventTotalUploadFinished,CU_EVENT_BLOCKING_SYNC);
	CheckError(cerr);	
	cerr = cuEventCreate(&cuEventTotalUploadStarted,CU_EVENT_BLOCKING_SYNC);
	CheckError(cerr);	
	makeAllSrcCtxNotCurrent();

	//create placeholders for the captured textures in the src context
	makeAllDstCtxCurrent();

	glGenBuffers(numStreamsPerFrame, dstVideoObjects);
	assert(glGetError() == GL_NO_ERROR);
	
	for (UINT i = 0; i < numStreamsPerFrame; i++) {	

		glBindBuffer(GL_VIDEO_BUFFER_NV, dstVideoObjects[i]);
		assert(glGetError() == GL_NO_ERROR);
		// Allocate required space in video capture buffer
		glBufferData(GL_VIDEO_BUFFER_NV,videoPitch * videoHeight,
				NULL, GL_STREAM_COPY);
		assert(glGetError() == GL_NO_ERROR);


				
	}	

	for (unsigned int i = 0; i < numStreamsPerFrame; i++) {
		cerr = cuGraphicsGLRegisterBuffer(&cuDstVideoObjects[i],dstVideoObjects[i],CU_GRAPHICS_MAP_RESOURCE_FLAGS_NONE);
		CheckError(cerr);	
	}
	cerr = cuEventCreate(&cuEventTotalDownloadFinished,CU_EVENT_BLOCKING_SYNC);
	CheckError(cerr);	
	cerr = cuEventCreate(&cuEventTotalDownloadStarted,CU_EVENT_BLOCKING_SYNC);
	CheckError(cerr);	
	makeAllDstCtxNotCurrent();
	return TRUE;
}
NvU32  C_Frame::deinit()
{
	if (numAudioSamples) {
		free(numAudioSamples);
		numAudioSamples = NULL;
		sequenceLength = 0;
	}
	// Free custom data space
	for(int i = 0; i < 4; i++) {
		if(ancData[i].CustomPackets)
			free(ancData[i].CustomPackets);
		//free the audio packet space
		for (int j = 0; j < 4; j++) {
			if(ancData[i].AudioGroup1.audioData[j])
				free(ancData[i].AudioGroup1.audioData[j]);
			if(ancData[i].AudioGroup2.audioData[j])
				free(ancData[i].AudioGroup2.audioData[j]);
			if(ancData[i].AudioGroup3.audioData[j])
				free(ancData[i].AudioGroup3.audioData[j]);
			if(ancData[i].AudioGroup4.audioData[j])
				free(ancData[i].AudioGroup4.audioData[j]);
		}		
	}

	
	makeAllSrcCtxCurrent();
	CUresult cerr;
	for (unsigned int i = 0; i < numStreamsPerFrame; i++) {
		cerr = cuGraphicsUnregisterResource(cuSrcVideoObjects[i]);
		CheckError(cerr);	
	}
	for (unsigned int i = 0; i < numStreamsPerFrame; i++) {
		cerr = cuMemFreeHost(hostVideoObject[i]);
		CheckError(cerr);	
	}
	//for (unsigned int i = 0; i < numStreamsPerFrame; i++) {
	//	cerr = cuEventDestroy(cuEventUploadFinished[i]);
	//	CheckError(cerr);	
	//}
	cerr = cuEventDestroy(cuEventTotalUploadFinished);
	CheckError(cerr);	
	cerr = cuEventDestroy(cuEventTotalUploadStarted);
	CheckError(cerr);	

	glDeleteBuffers(numStreamsPerFrame,srcVideoObjects);

	assert(glGetError() == GL_NO_ERROR);	

	makeAllSrcCtxNotCurrent();

	makeAllDstCtxCurrent();


	for (unsigned int i = 0; i < numStreamsPerFrame; i++) {
		cerr = cuGraphicsUnregisterResource(cuDstVideoObjects[i]);
		CheckError(cerr);	
	}
	cerr = cuEventDestroy(cuEventTotalDownloadFinished);
	CheckError(cerr);	
	cerr = cuEventDestroy(cuEventTotalDownloadStarted);
	CheckError(cerr);	

	glDeleteBuffers(numStreamsPerFrame,dstVideoObjects);
	makeAllDstCtxNotCurrent();

	return TRUE;
}
NvU32 C_Frame::bindSrcObjects()
{	
	
	for(int i = 0; i < sdiIn->GetNumStreams();i++)
	{
		sdiIn->BindVideoFrameBuffer(srcVideoObjects[i],GL_YCBYCR8_422_NV, i);
	}

	return TRUE;
}
NvU32 C_Frame::unbindSrcObjects()
{
	for(int i = 0; i < sdiIn->GetNumStreams();i++)
	{
		sdiIn->UnbindVideoFrameBuffer(i);		
	}

	CUresult cerr;	
	size_t size;	

	cerr = cuEventRecord(cuEventTotalUploadStarted,0);
	CheckError(cerr);			

	cerr = cuGraphicsMapResources(numStreamsPerFrame, cuSrcVideoObjects,0);
	CheckError(cerr);	

	for(int i = 0; i< numStreamsPerFrame; i++)
	{
		cerr = cuGraphicsResourceGetMappedPointer(&cuSrcDevicePtr[i], &size, cuSrcVideoObjects[i]);		
		CheckError(cerr);			
		//initiate an upload to sysmem
		//cerr = cuMemcpyDtoHAsync(hostVideoObject[i],cuSrcDevicePtr[i],videoPitch*videoHeight,0);		
		cerr = cuMemcpyDtoH(hostVideoObject[i],cuSrcDevicePtr[i],videoPitch*videoHeight);		
		CheckError(cerr);			
	}

	//cerr = cuEventRecord(cuEventUploadFinished[i],0);
	//CheckError(cerr);			
	cerr = cuGraphicsUnmapResources(numStreamsPerFrame, cuSrcVideoObjects,0);
	CheckError(cerr);	
	cerr = cuEventRecord(cuEventTotalUploadFinished,0);
	CheckError(cerr);			

	cuEventSynchronize(cuEventTotalUploadFinished);
	cerr = cuEventElapsedTime(&uploadElapsedTime, cuEventTotalUploadStarted, cuEventTotalUploadFinished);
	CheckError(cerr);	
	uploadElapsedTime /= 1000;
	return TRUE;
}
GLuint C_Frame::getDstObject(int i)
{
	if(i >= 0 && i < MAX_VIDEO_STREAMS)
		return dstVideoObjects[i];
	else 
		return 0;

}
//NvU32  C_Frame::sync()
//{
//	CUresult cerr;	
//	cerr = cuCtxSynchronize();
//	CheckError(cerr);
//	return TRUE;
//}
NvU32  C_Frame::sync()
{
	CUresult cerr;	
	size_t size;	
	cerr = cuEventRecord(cuEventTotalDownloadStarted,0);
	CheckError(cerr);			

	cerr = cuGraphicsMapResources(numStreamsPerFrame, cuDstVideoObjects,0);
	CheckError(cerr);	


	for(int i = 0; i< numStreamsPerFrame; i++)
	{
		//wait until the upload to sysmem finished to begin download
		//cerr = cuEventSynchronize(cuEventUploadFinished[i]); 
		//CheckError(cerr);			

		cerr = cuGraphicsResourceGetMappedPointer(&cuDstDevicePtr[i], &size, cuDstVideoObjects[i]);	
		CheckError(cerr);		
		cerr = cuMemcpyHtoD(cuDstDevicePtr[i],hostVideoObject[i],videoPitch*videoHeight);		
		CheckError(cerr);			
	}

	cerr = cuGraphicsUnmapResources(numStreamsPerFrame, cuDstVideoObjects,0);		
	CheckError(cerr);	
	cerr = cuEventRecord(cuEventTotalDownloadFinished,0);
	CheckError(cerr);			

	cuEventSynchronize(cuEventTotalDownloadFinished);
	cerr = cuEventElapsedTime(&downloadElapsedTime, cuEventTotalDownloadStarted, cuEventTotalDownloadFinished);
	CheckError(cerr);	

	downloadElapsedTime /= 1000;

	return TRUE;
	//GLboolean imageCopySuccessful;
	////for(int i = 0; i < numStreamsPerFrame; i++)
	//for(int i = 0; i < MAX_VIDEO_STREAMS; i++)
	//{
	//	imageCopySuccessful = wglCopyImageSubDataNV(srcRC,srcVideoObjects[i], GL_TEXTURE_RECTANGLE_NV,0,0,0,0,
	//						  dstRC,dstVideoObjects[i],GL_TEXTURE_RECTANGLE_NV,0,0,0,0,
	//						  videoWidth,videoHeight,1);
	//	if(imageCopySuccessful == GL_FALSE)
	//		return FALSE;
	//}
	//return TRUE;
}
//=========================C_RingBuffer=======================================================
//A struct to encapsulate all the captured data that will get passed around in the application
//This is a ring buffer between the rendering thread and the capture thread that contains:
//-copy of video objects in the drawing context
//-anc data 
//There is only one consumer and one producer
//If one of the threads is slower, the other thread does not wait
//============================================================================================
C_RingBuffer::C_RingBuffer()
{	
	lock_create(&m_hLock);		

	for(int n = 0; n< NUM_RING_BUFFER_FRAMES;n++)
	{
		m_Frames[n] = NULL;
	}
}
C_RingBuffer::~C_RingBuffer()
{
	lock_destroy(&m_hLock);


}

NvU32 C_RingBuffer::init(CNvSDIin *SDIin, HDC captureDC, HGLRC captureRC,CUcontext captureCUctx, HDC renderDC, HGLRC renderRC,CUcontext renderCUctx)
{	
	if(SDIin == NULL || captureDC == NULL || captureRC == NULL || captureDC == NULL || captureRC == NULL)
		return TRUE;
	//allocate the frames and put them in the repository
	for(int n = 0; n< NUM_RING_BUFFER_FRAMES;n++)
	{
		m_Frames[n] = (C_Frame *)calloc(1, sizeof(C_Frame));	
		if(m_Frames[n]->init(SDIin, captureDC, captureRC, captureCUctx, renderDC, renderRC, renderCUctx) == FALSE)
		{			
			return FALSE;
		}
		m_Repository.push_back(m_Frames[n]);
	}	

    return TRUE;
}


NvU32 C_RingBuffer::deinit()
{
	for(int n = 0; n< NUM_RING_BUFFER_FRAMES;n++)
	{
		if(m_Frames[n])
		{
			m_Frames[n]->deinit();
			free(m_Frames[n]);
		}
	}
	return S_OK;
}

C_Frame *C_RingBuffer::getPendingFrame()
{	
	C_Frame *result;
	lock_acquire(&m_hLock);
	if(m_Pending.empty())
	{
		result = NULL;
	}
	else
	{
		result = m_Pending.front();
		m_Pending.pop_front();
	}
	lock_release(&m_hLock);
	return result;
}

C_Frame *C_RingBuffer::getRepositoryFrame()
{
	C_Frame *result;
	lock_acquire(&m_hLock);
	if(m_Repository.empty())
	{
		//if the repository is ampty use the oldest element from the pending deque
		if(m_Pending.empty())
			result = NULL;
		else
		{
			
			result = m_Pending.front();			
			m_Pending.pop_front();
		}				
	}
	else
	{
		//take the element from the respoitory
		result = m_Repository.front();
		m_Repository.pop_front();
	}
	lock_release(&m_hLock);
	return result;
}

NvU32 C_RingBuffer::releaseFrame(C_Frame *frame)
{
	lock_acquire(&m_hLock);
	if(frame!=NULL)
		m_Repository.push_back(frame);
	lock_release(&m_hLock);
	return TRUE;
}

NvU32 C_RingBuffer::addFrame(C_Frame *frame)
{
	lock_acquire(&m_hLock);
	m_Pending.push_back(frame);
	lock_release(&m_hLock);
	return TRUE;
}

//=========================C_DVP==============================================
//A class to encapsulate the functionality of a distinct digital video pipeline
//============================================================================


C_DVP::C_DVP()
{
	

	for(int i = 0;i < NVAPI_MAX_VIO_DEVICES;i++)
	{		
		m_CaptureRingBuffer[i] = NULL;
	}	
	m_hCaptureRC = NULL;
	m_hCaptureDC = NULL;
	m_hRenderRC = NULL;
	m_hRenderDC = NULL;
	m_CaptureThreadID = NULL;		
	m_activeDeviceCount = 0;


	m_options.sampling = NVVIOCOMPONENTSAMPLING_422;
	m_options.dualLink = false;
	m_options.bitsPerComponent = 8;
	m_options.expansionEnable = false;
	m_options.captureGPU = 0;
}

C_DVP::~C_DVP()
{

}

bool C_DVP::makeAllCaptureCtxCurrent()
{
	wglMakeCurrent(m_hCaptureDC, m_hCaptureRC); 
	CUresult cerr;
	cerr = cuCtxPushCurrent(m_cuCaptureCtx);
	CheckError(cerr);	
	return true;
}
bool C_DVP::makeAllRenderCtxCurrent()
{

	wglMakeCurrent(m_hRenderDC, m_hRenderRC); 
	CUresult cerr;
	cerr = cuCtxPushCurrent(m_cuRenderCtx);
	CheckError(cerr);	
	return true;
}

bool C_DVP::makeAllRenderCtxNotCurrent()
{

	CUresult cerr;
	cerr = cuCtxPopCurrent(&m_cuRenderCtx);
	CheckError(cerr);
	wglMakeCurrent(NULL, NULL);

	return true;
}

bool C_DVP::makeAllCaptureCtxNotCurrent()
{
	CUresult cerr;
	cerr = cuCtxPopCurrent(&m_cuCaptureCtx);
	CheckError(cerr);
	wglMakeCurrent(NULL, NULL);

	return true;
}


HRESULT C_DVP::SetupSDIPipeline()
{	
	int numCaptureDevices = CNvSDIinTopology::instance().getNumDevice();
	
	for(int i = 0; i < numCaptureDevices; i++)
	{
		m_SDIin[m_activeDeviceCount].Init(&m_options);
	
		// Initialize the video capture device.
		if (m_SDIin[m_activeDeviceCount].SetupDevice(false,i) != S_OK)
		{
			continue;
		}
		m_activeDeviceCount++;
	}
	if(m_activeDeviceCount == 0)
		return E_FAIL;
	return S_OK;
}

HRESULT C_DVP::SetupSDIinGL(HDC renderDC,HGLRC renderRC)
{
	m_hCaptureGPU = CNvGpuTopology::instance().getGpu(m_options.captureGPU)->getAffinityHandle();		
	m_hRenderGPU = CNvGpuTopology::instance().getPrimaryGpu()->getAffinityHandle();

	m_hRenderRC = renderRC;
	m_hRenderDC = renderDC;
	if(setupGL() == GL_FALSE)
		return E_FAIL;
	if(setupCUDA()  != CUDA_SUCCESS)
		return E_FAIL;


	for(int i = 0; i < m_activeDeviceCount; i ++)
	{		

		m_CaptureRingBuffer[i] = new C_RingBuffer();
		if(m_CaptureRingBuffer[i] == NULL)
			return E_FAIL;
		if(m_CaptureRingBuffer[i]->init(&m_SDIin[i], m_hCaptureDC, m_hCaptureRC, m_cuCaptureCtx, m_hRenderDC, m_hRenderRC, m_cuRenderCtx) == FALSE)
		{
			m_CaptureRingBuffer[i]->deinit();
			delete m_CaptureRingBuffer[i];
			return E_FAIL;
		}
	}
	
	return S_OK;
}

HRESULT C_DVP::CleanupSDIinGL()
{
	for(int i = 0; i < m_activeDeviceCount; i ++)
	{		
		m_CaptureRingBuffer[i]->deinit();
		delete m_CaptureRingBuffer[i];	
	}
	cleanupCUDA();
	cleanupGL();	
	return S_OK;
}

HRESULT C_DVP::CleanupSDIPipeline()
{	

	return S_OK;
}
CUresult C_DVP::setupCUDA()
{

	CUresult cerr;
	cerr = cuInit(0);
	CheckError(cerr);

	wglMakeCurrent(m_hRenderDC, m_hRenderRC); 	
	//get a cuda device that corresponds to the affinity handle of the primary GPU
	cerr = cuWGLGetDevice(&m_cuRenderDevice,m_hRenderGPU);  
	CheckError(cerr);		
	// now create the CUDA context	
	cerr = cuGLCtxCreate(&m_cuRenderCtx, CU_CTX_MAP_HOST|CU_CTX_BLOCKING_SYNC,m_cuRenderDevice);
	CheckError(cerr);	
	cerr = cuCtxPopCurrent(&m_cuRenderCtx);
	CheckError(cerr);
	wglMakeCurrent(NULL, NULL);


	wglMakeCurrent(m_hCaptureDC, m_hCaptureRC); 		
	cerr = cuWGLGetDevice(&m_cuCaptureDevice,m_hCaptureGPU);  
	CheckError(cerr);		
	// now create the CUDA context	
	cerr = cuGLCtxCreate(&m_cuCaptureCtx, CU_CTX_MAP_HOST|CU_CTX_BLOCKING_SYNC,m_cuCaptureDevice);
	CheckError(cerr);	
	cerr = cuCtxPopCurrent(&m_cuCaptureCtx);
	CheckError(cerr);
	wglMakeCurrent(NULL, NULL);
	return CUDA_SUCCESS;
}
CUresult C_DVP::cleanupCUDA()
{
	CUresult cerr;
	wglMakeCurrent(m_hRenderDC, m_hRenderRC); 	
	cerr = cuCtxDestroy(m_cuRenderCtx);	
	CheckError(cerr);
	wglMakeCurrent(m_hCaptureDC, m_hCaptureRC); 				
	cerr = cuCtxDestroy(m_cuCaptureCtx);
	CheckError(cerr);
	return CUDA_SUCCESS;
}

GLboolean C_DVP::setupGL()
{
	// note, need to pass a null terminated list of handles, not jsut a single handle so make a list of two
    HGPUNV handles[2];
    handles[0] = m_hCaptureGPU;
    handles[1] = NULL;

    m_hCaptureDC = wglCreateAffinityDCNV(handles);
    if(m_hCaptureDC == NULL)
    {
       int error = GetLastError();
	   printf("Error: wglCreateAffinityDCNV error code: %d\n",error);
       return GL_FALSE;
    }
    //else
    //{
    //    printf("wglCreateAffinityDCNV succeeded\n");
    //}

	PIXELFORMATDESCRIPTOR pfd =							// pfd Tells Windows How We Want Things To Be
	{
		sizeof (PIXELFORMATDESCRIPTOR),					// Size Of This Pixel Format Descriptor
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
    GLuint pf = ::ChoosePixelFormat(m_hCaptureDC, &pfd);
    BOOL rslt = ::SetPixelFormat(m_hCaptureDC, pf, &pfd);


	// Create rendering context from the affinity device context	
	m_hCaptureRC = wglCreateContext(m_hCaptureDC); 

	// Make window rendering context current.
	wglMakeCurrent(m_hCaptureDC, m_hCaptureRC); 
	
	if(!loadCaptureVideoExtension() || 
		!loadTimerQueryExtension() || !loadBufferObjectExtension())
	{
		printf("Could not load the required OpenGL extensions\n");
		return false;
	}
	glClearColor( 0.0, 0.0, 0.0, 0.0); 
	glClearDepth( 1.0 ); 
	 
	glDisable(GL_DEPTH_TEST); 

	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);

	
	GLuint gpuVideoSlot = 1;
	for(int i = 0; i < m_activeDeviceCount; i++)
	{	
		m_SDIin[i].BindDevice(gpuVideoSlot++,m_hCaptureDC);	
	}

	wglMakeCurrent(NULL, NULL); 
	return GL_TRUE;
}


GLboolean C_DVP::cleanupGL()
{
	wglMakeCurrent(m_hCaptureDC, m_hCaptureRC); 

	for(int i = 0; i < m_activeDeviceCount; i++)
	{	
		m_SDIin[i].UnbindDevice();
	}
	// Delete OpenGL rendering context.
	wglMakeCurrent(NULL,NULL) ; 
	if (m_hCaptureRC) 
	{
		wglDeleteContext(m_hCaptureRC) ;
		m_hCaptureRC = NULL ;
	}		
	wglDeleteDCNV(m_hCaptureDC);
	return GL_TRUE;
}


HRESULT C_DVP::StartSDIPipeline()
{
	
	// Start video capture
	wglMakeCurrent(m_hCaptureDC, m_hCaptureRC); 	


	for(int i = 0; i < m_activeDeviceCount; i++)
	{	
	
		if(m_SDIin[i].StartCapture()!= S_OK)
		{
			MessageBox(NULL, "Error starting video capture.", "Error", MB_OK);
			return E_FAIL;
		}
	}

	m_QuitCapture = false;
	
	makeAllRenderCtxCurrent();

	thread_create(&m_CaptureThreadID, &C_DVP::CaptureThreadWrapper, this);	

	

	return S_OK;
}

HRESULT C_DVP::StopSDIPipeline()
{
	makeAllRenderCtxNotCurrent();
	m_QuitCapture = true;
	//wait for the capture thread to finish
	thread_wait(&m_CaptureThreadID);
	thread_destroy(&m_CaptureThreadID);	
	wglMakeCurrent(m_hCaptureDC, m_hCaptureRC); 
	
	for(int i = 0; i < m_activeDeviceCount; i++)
	{	
		m_SDIin[i].EndCapture();
	}
	wglMakeCurrent(NULL,NULL); 
	return S_OK;
}


void C_DVP::CaptureThread()
{	
	GLint64EXT captureTime;
	GLuint sequenceNum;    
	GLuint numDroppedFrames[NVAPI_MAX_VIO_DEVICES];
	static GLuint prevSequenceNum[NVAPI_MAX_VIO_DEVICES];
	GLenum ret;
	static int numFails = 0;
	static int numTries = 0;
	
	for(int i = 0; i < m_activeDeviceCount; i++)
	{	
		numDroppedFrames[i] = 0;
		prevSequenceNum[i] = 0;
	}
	makeAllCaptureCtxCurrent();

	GLint64EXT captureTimeStart;
	GLint64EXT captureTimeEnd;	
	while(!m_QuitCapture)
	{
		if(numFails < 100)
		{
			
			for(int i = 0; i < m_activeDeviceCount; i++)
			{	
				glGetInteger64v(GL_CURRENT_TIME_NV,(GLint64 *)&captureTimeStart);
				C_Frame *frame = m_CaptureRingBuffer[i]->getRepositoryFrame();							
				frame->bindSrcObjects();
				ret = m_SDIin[i].Capture(&sequenceNum, &captureTime);
				numDroppedFrames[i] += sequenceNum - prevSequenceNum[i] - 1;
			
				prevSequenceNum[i] = sequenceNum;
				switch(ret) {
					case GL_SUCCESS_NV:
					//printf("Frame:%d gpuTime:%f gviTime:%f\n", sequenceNum, m_SDIin[i].m_gpuTime,m_SDIin[i].m_gviTime);
					numFails = 0;
					break;
					case GL_PARTIAL_SUCCESS_NV:
					printf("glVideoCaptureNV: GL_PARTIAL_SUCCESS_NV\n");
					numFails = 0;
					break;
					case GL_FAILURE_NV:
					printf("glVideoCaptureNV: GL_FAILURE_NV - Video capture failed.\n");
					numFails++;
					break;
					default:
					printf("glVideoCaptureNV: Unknown return value.\n");
					break;
				} // switch
		
				//NvVIOANCAPI_CaptureANCData(m_SDIin[i]->getHandle(), frame->ancData);
				frame->unbindSrcObjects();
				glGetInteger64v(GL_CURRENT_TIME_NV,(GLint64 *)&captureTimeEnd);
				frame->captureTime = captureTime;
				frame->sequenceNum = sequenceNum;			
				//here we need to get only the raw upload time regardless of any latency
				//int latency = (int)(m_SDIin[i]->m_gviTime*GetFrameRate());
				//frame->captureElapsedTime = m_SDIin[i]->m_gviTime - latency/GetFrameRate() + frame->uploadElapsedTime;
				//frame->captureElapsedTime = frame->uploadElapsedTime;
				frame->captureElapsedTime = float((captureTimeEnd - captureTimeStart)*.000000001);//m_SDIin[i]->m_gpuTime+frame->uploadElapsedTime;
				frame->numDroppedFrames = numDroppedFrames[i];
				m_CaptureRingBuffer[i]->addFrame(frame);
			}
		}

	}
	makeAllCaptureCtxNotCurrent();
}



C_Frame *C_DVP::GetFrame(int deviceIndex)
{
	C_Frame *frame = m_CaptureRingBuffer[deviceIndex]->getPendingFrame();
	if(frame)
		frame->sync();
	return frame;
}

void C_DVP::ReleaseUsedFrame(int deviceIndex, C_Frame *frame)
{
	m_CaptureRingBuffer[deviceIndex]->releaseFrame(frame);
}


int  C_DVP::GetVideoHeight()
{
	return m_SDIin[0].GetHeight();
}
int  C_DVP::GetVideoWidth()
{
	return m_SDIin[0].GetWidth();
}

NVVIOSIGNALFORMAT  C_DVP::GetSignalFormat()
{
	return m_SDIin[0].GetSignalFormat();

}
int C_DVP::GetNumStreamsPerFrame(int deviceIndex)
{
	if(deviceIndex >= 0 && deviceIndex < m_activeDeviceCount)
		return m_SDIin[deviceIndex].GetNumStreams();
	return 0;
}

int C_DVP::GetDeviceNumber(int activeDeviceIndex)
{
	if(activeDeviceIndex >= 0 && activeDeviceIndex < m_activeDeviceCount)
		return m_SDIin[activeDeviceIndex].GetDeviceNumber();	
	return 0;
}
//=========================C_multiCaptureAsync==============================================
//The main class that's responsible to orchestrate the video piplines and display the video
//==========================================================================================


// From 8bit422toRGB.glsl:
const char *GLSL_8bit422toRGB =
"// Needed for array initializers:\n"
"#version 120\n"
"// Needed to use integer textures:\n"
"#extension GL_EXT_gpu_shader4 : enable\n"
"#extension GL_ARB_texture_rectangle : enable\n"
"\n"
"uniform mat3 csc;\n"
"uniform vec3 offset;\n"
"uniform vec3 scale;\n"
"\n"
"uniform usampler2DRect tex;\n"
"\n"
"uniform ivec2 size;\n"
"\n"
"uniform float divide;\n"
"\n"
"vec3 colorLookup(vec2 coord) {\n"
"    float y, cr, cb;\n"
"    vec2 texCoordAligned, texCoordOffset;\n"
"    uvec4 aligned, offsetTexel;\n"
"\n"
"    texCoordOffset = vec2(coord.x / 2, coord.y);\n"
"    texCoordAligned = vec2(floor(texCoordOffset.x), texCoordOffset.y);\n"
"    offsetTexel = uvec4(texture2DRect(tex, texCoordOffset));\n"
"    aligned = uvec4(texture2DRect(tex, texCoordAligned));\n"
"\n"
"    //\n"
"    // Input is laid out like:\n"
"    // Y  Cr Y' Cb\n"
"    // R  G  B  A\n"
"    //\n"
"    // For 4:2:2 -> 4:4:4 expansion, the data are colocated\n"
"    // temporally/spatially as:\n"
"    //\n"
"    // Y  | Y' | Y  | Y'\n"
"    // Cr |    | Cr |   ....\n"
"    // Cb |    | Cb |\n"
"    // ev   od   ev   od\n"
"    //\n"
"    //  The Cr/Cb interpolation is taken care of by using LINEAR texture\n"
"    // interpolation on the GPU and specifying the correct texture coordinates\n"
"    // in texCoordOffset above.\n"
"    // Y and Y', however, should never be interpolated and should just be\n"
"    // chosen based on odd or even pixels.\n"
"    //\n"
"    if (bool(int(coord.x) & 1)) {\n"
"        // Odd.\n"
"        y = float(aligned.b);\n"
"    } else {\n"
"        // Even.\n"
"        y = float(aligned.r);\n"
"    }\n"
"    \n"
"    cr = float(offsetTexel.a);\n"
"    cb = float(offsetTexel.g);    \n"
"\n"
"    {\n"
"       // Do color-space conversion.\n"
"       float r, g, b;\n"
"\n"
"       r = 1.164 * (y - 16.0) + 1.596 * (cr - 128.0);\n"
"       g = 1.164 * (y - 16.0) - 0.813 * (cr - 128.0) - 0.392 * (cb - 128.0);\n"
"       b = 1.164 * (y - 16.0)                        + 2.017 * (cb - 128.0);\n"
"\n"
"       r /= 256.0;\n"
"       g /= 256.0;\n"
"       b /= 256.0;\n"
"\n"
"       return clamp(vec3(r, g, b), 0.0, 1.0);\n"
"   }\n"
"}\n"
"\n"
"void main() {\n"
"    vec2 coord = vec2(gl_FragCoord.x, gl_FragCoord.y);\n"
"\n"
"    // Grab value at our coord\n"
"    vec3 input = colorLookup(coord);\n"
"\n"
"    gl_FragColor = vec4(input, 1.0);\n"
"}\n"
"";

HRESULT C_DVP::CompileShader(GLuint shader, const char *shaderStr)
{
	glShaderSource(shader, 1, (const GLchar **)&shaderStr, NULL);

	glCompileShader(shader);

	GLint val;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &val);
	if (!val) {
		char infoLog[10000];
		glGetShaderInfoLog(shader, 10000, NULL, infoLog);

		printf("Failed to load GLSL decode shader, INFO:\n\n%s\n", infoLog);
		return S_FALSE;
	}

	return S_OK;
}
HRESULT C_DVP::SetupDecodeProgram()
{
	// Create the decode shader
	decodeShader = glCreateShader(GL_FRAGMENT_SHADER);

	if (CompileShader(decodeShader, GLSL_8bit422toRGB) == S_FALSE) {
		fprintf(stderr, "Failed to load GLSL decode shader\n");
		return false;
	}

	decodeProgram = glCreateProgram();

	glAttachShader(decodeProgram, decodeShader);

	glLinkProgram(decodeProgram);

	GLint res;
	glGetProgramiv(decodeProgram, GL_LINK_STATUS, &res);

	if (!res) {
		fprintf(stderr, "Failed to link GLSL decode program\n");
		GLint infoLength;
		glGetProgramiv(decodeProgram, GL_INFO_LOG_LENGTH, &infoLength);
		if (infoLength) {
			char *buf;
			buf = (char *)malloc(infoLength);
			if (buf) {
				glGetProgramInfoLog(decodeProgram, infoLength, NULL, buf);
				fprintf(stderr, "Program log: \n");
				fprintf(stderr, buf);
				free(buf);
			}
		}
		return false;
	}

	assert(glGetError() == GL_NO_ERROR);
}
HRESULT C_DVP::DestroyDecodeProgram()
{
	glDetachShader(decodeProgram, decodeShader);
	assert(glGetError() == GL_NO_ERROR);
	glDeleteShader(decodeShader);
	assert(glGetError() == GL_NO_ERROR);
	glDeleteProgram(decodeProgram);
	assert(glGetError() == GL_NO_ERROR);
	return true;
}

