//*******************************************************************************
//passthru: Demonstrates passing video and ancillary data from the SDI capture
//device to the SDI output device.  Captures video/ancillary data from a single 
//capture card into texture objects on a Primary GPU and data buffers in system 
//memory.  The captured data is then displayed in the graphics window.  And, in 
//the case that an SDI output device is available, the video and ancillary data 
//is sent to the SDI output device.
//*******************************************************************************

#include "passthru.h"
#include "nvSDIutil.h"
#include "glExtensions.h"

C_passthru passthru;


C_passthru::C_passthru()
{
	m_bAncInInitialized = FALSE;
	m_bAncOutInitialized = FALSE;
	m_NumAudioSamples = NULL;
	m_bSDIout = FALSE;
}
C_passthru::~C_passthru()
{


}


GLboolean C_passthru::SetupGL()
{
	// Create window device context and rendering context.
	m_hDC = GetDC(m_hWnd); 

	HGPUNV  gpuMask[2];
	gpuMask[0] = m_gpu->getAffinityHandle();
	gpuMask[1] = NULL;
	if (!(m_hAffinityDC = wglCreateAffinityDCNV(gpuMask))) {
		printf("Unable to create GPU affinity DC\n");
	}

	PIXELFORMATDESCRIPTOR pfd =							// pfd Tells Windows How We Want Things To Be
	{
		sizeof (PIXELFORMATDESCRIPTOR),					// Size Of This Pixel Format Descriptor
		1,												// Version Number
		PFD_DRAW_TO_WINDOW |							// Format Must Support Window
		PFD_SUPPORT_OPENGL |							// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,								// Must Support Double Buffering
		PFD_TYPE_RGBA,									// Request An RGBA Format
		24,												// Select Our Color Depth
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

	GLuint pf = ChoosePixelFormat(m_hAffinityDC, &pfd);
	HRESULT rslt = SetPixelFormat(m_hAffinityDC, pf, &pfd);
	//		return NULL;
	//Create affinity-rendering context from affinity-DC
	if (!(m_hRC = wglCreateContext(m_hAffinityDC))) {
		printf("Unable to create GPU affinity RC\n");
	}

	//m_hRC = wglCreateContext(m_hDC); 

	// Make window rendering context current.
	wglMakeCurrent(m_hDC, m_hRC); 
	//load the required OpenGL extensions:
	if(!loadSwapIntervalExtension() || !loadTimerQueryExtension() || !loadAffinityExtension())
	{
		printf("Could not load the required OpenGL extensions\n");
		return false;
	}


	// Unlock capture/draw loop from vsync of graphics display.
	// This should lock the capture/draw loop to the vsync of 
	// input video.
	if (wglSwapIntervalEXT) {
		wglSwapIntervalEXT(0);
	}
	glClearColor( 0.0, 0.0, 0.0, 0.0); 
	glClearDepth( 1.0 ); 

	glDisable(GL_DEPTH_TEST); 

	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);

	setupSDIinGL();

	setupSDIoutGL();

	return GL_TRUE;
}

HRESULT C_passthru::Configure(char *szCmdLine[])
{
	int numGPUs;
	// Note, this function enumerates GPUs which are both CUDA & GLAffinity capable (i.e. newer Quadros)  
	numGPUs = CNvSDIoutGpuTopology::instance().getNumGpu(); 

	if(numGPUs <= 0)
	{
		MessageBox(NULL, "Unable to obtain system GPU topology", "Error", MB_OK);
		return E_FAIL;
	}

	int numCaptureDevices = CNvSDIinTopology::instance().getNumDevice();

	if(numCaptureDevices <= 0)
	{
		MessageBox(NULL, "Unable to obtain system Capture topology", "Error", MB_OK);
		return E_FAIL;
	}

	options.sampling = NVVIOCOMPONENTSAMPLING_422;
	options.dualLink = false;
	options.bitsPerComponent = 8;
	options.expansionEnable = true;
	options.captureDevice = 0;
	options.captureGPU = CNvSDIoutGpuTopology::instance().getPrimaryGpuIndex();

	ParseCommandLine(szCmdLine, &options);//get the user config

	if(options.captureDevice >= numCaptureDevices)		
	{
		MessageBox(NULL, "Selected Capture Device is out of range", "Error", MB_OK);
		return E_FAIL;
	}
	if(options.captureGPU >= numGPUs)		
	{
		MessageBox(NULL, "Selected Capture GPU is out of range", "Error", MB_OK);
		return E_FAIL;
	}
	m_gpu = CNvSDIoutGpuTopology::instance().getGpu(options.captureGPU);

	return S_OK;
}

HRESULT C_passthru::SetupSDIDevices()
{
	if (setupSDIinDevices() != S_OK)
	{
		MessageBox(NULL, "Error setting up video capture.", "Error", MB_OK);
		return E_FAIL;
	}

	if (setupSDIoutDevice() == S_OK) 
	{
		m_bSDIout = TRUE;
	} 
	else 
	{
		MessageBox(NULL, "SDI video output unavailable.", "Warning", MB_OK);
	}

	return S_OK;
}

/////////////////////////////////////
// Pipeline Activation
/////////////////////////////////////

HRESULT C_passthru::StartSDIPipeline()
{
	// Start video capture
	if(m_SDIin.StartCapture()!= S_OK)
	{
		MessageBox(NULL, "Error starting video capture.", "Error", MB_OK);
		return E_FAIL;
	}
	return S_OK;
}
/////////////////////////////////////
// Pipeline deactivation
/////////////////////////////////////

HRESULT C_passthru::StopSDIPipeline()
{
	m_SDIin.EndCapture();



	return S_OK;
}

HRESULT C_passthru::setupSDIinDevices()
{
	m_SDIin.Init(&options);

	// Initialize the video capture device.
	if (m_SDIin.SetupDevice(true,options.captureDevice) != S_OK)
		return E_FAIL;
	m_videoWidth = m_SDIin.GetWidth();
	m_videoHeight = m_SDIin.GetHeight();

	return S_OK;
}

HRESULT C_passthru::setupSDIoutDevice()
{
	// Output video format matches input video format.
	options.videoFormat = m_SDIin.GetSignalFormat();

	// Initialize additional SDI output options.
	options.dataFormat = NVVIODATAFORMAT_R8G8B8A8_TO_YCRCBA4224;
	options.flipQueueLength = 5;

	return (m_SDIout.Init(&options, m_gpu));
}

//
// Initialize OpenGL
//
HRESULT C_passthru::setupSDIinGL()
{
	if(!loadCaptureVideoExtension() || !loadBufferObjectExtension() )
	{
		printf("Could not load the required OpenGL extensions\n");
		return false;
	}

	//setup the textures for capture
	glGenTextures(m_SDIin.GetNumStreams(), m_videoTextures);
	assert(glGetError() == GL_NO_ERROR);
	for(int i = 0; i < m_SDIin.GetNumStreams();i++)
	{
		glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_videoTextures[i]);
		assert(glGetError() == GL_NO_ERROR);
		glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		assert(glGetError() == GL_NO_ERROR);

		glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA8, m_videoWidth, m_videoHeight, 
			0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		assert(glGetError() == GL_NO_ERROR);
	}

	// CSC parameters
	GLfloat mat[4][4];
	float scale = 1.0f;

	GLfloat max[] = {5000, 5000, 5000, 5000};;
	GLfloat min[] = {0, 0, 0, 0};

	GLfloat offset[] = {0, 0, 0, 0};

	if (1) {
		mat[0][0] = 1.164f *scale;
		mat[0][1] = 1.164f *scale;
		mat[0][2] = 1.164f *scale;
		mat[0][3] = 0;  

		mat[1][0] = 0;
		mat[1][1] = -0.392f *scale;
		mat[1][2] = 2.017f *scale;
		mat[1][3] = 0;

		mat[2][0] = 1.596f *scale;
		mat[2][1] = -0.813f *scale;
		mat[2][2] = 0.f;
		mat[2][3] = 0;

		mat[3][0] = 0;
		mat[3][1] = 0;
		mat[3][2] = 0;
		mat[3][3] = 1;

		offset[0] =-0.87f;
		offset[1] = 0.53026f;
		offset[2] = -1.08f;
		offset[3] = 0;
	}

	GLuint gpuVideoSlot = 1;
	m_SDIin.SetCSCParams(&mat[0][0], offset, min, max);
	m_SDIin.BindDevice(gpuVideoSlot, m_hDC);	
	for(int i = 0; i < m_SDIin.GetNumStreams(); i++)
		m_SDIin.BindVideoTexture(m_videoTextures[i],i);

	return S_OK;
}

HRESULT C_passthru::cleanupSDIinGL()
{
	for(int i = 0; i < m_SDIin.GetNumStreams(); i++)
		m_SDIin.UnbindVideoTexture(i);
	m_SDIin.UnbindDevice();
	glDeleteTextures(m_SDIin.GetNumStreams(),m_videoTextures);


	return S_OK;
}

HRESULT C_passthru::setupSDIoutGL()
{
	if(!loadPresentVideoExtension() ||  !loadFramebufferObjectExtension())
	{
		MessageBox(NULL, "Couldn't load required OpenGL extensions.", "Error", MB_OK);
		exit(1);
	}

	// Enumerate the available video devices and
	// bind to the first one found
	HVIDEOOUTPUTDEVICENV *videoDevices;

	// Get list of available video devices.
	int numDevices = wglEnumerateVideoDevicesNV(m_hDC, NULL);

	if (numDevices <= 0) {
		MessageBox(NULL, "wglEnumerateVideoDevicesNV() did not return any devices.", 
			"Error", MB_OK);
		exit(1);
	}

	videoDevices = (HVIDEOOUTPUTDEVICENV *)malloc(numDevices *
		sizeof(HVIDEOOUTPUTDEVICENV));

	if (!videoDevices) {
		fprintf(stderr, "malloc failed.  OOM?");
		exit(1);
	}

	if (numDevices != wglEnumerateVideoDevicesNV(m_hDC, videoDevices)) {
		free(videoDevices);
		MessageBox(NULL, "Invonsistent results from wglEnumerateVideoDevicesNV()",
			"Error", MB_OK);
		exit(1);
	}

	//Bind the first device found.
	if (!wglBindVideoDeviceNV(m_hDC, 1, videoDevices[0], NULL)) {
		free(videoDevices);
		MessageBox(NULL, "Failed to bind a videoDevice to slot 0.\n",
			"Error", MB_OK);
		exit(1);
	}

	// Free list of available video devices, don't need it anymore.
	free(videoDevices);

	// Start video transfers
	if ( m_SDIout.Start()!= S_OK ) {
		MessageBox(NULL, "Error starting video devices.", "Error", MB_OK);
	}
	return S_OK;
}

HRESULT C_passthru::cleanupSDIoutGL()
{
	return S_OK;
}

GLboolean C_passthru::CleanupGL()
{
	cleanupSDIinGL();
	if(m_bSDIout)
		cleanupSDIoutGL();
	// Delete OpenGL rendering context.
	wglMakeCurrent(NULL,NULL) ; 
	if (m_hRC) 
	{
		wglDeleteContext(m_hRC) ;
		m_hRC = NULL ;
	}		
	ReleaseDC(m_hWnd,m_hDC);

	wglDeleteDCNV(m_hAffinityDC);
	return S_OK;
}



/////////////////////////////////////
// Capture 
/////////////////////////////////////
GLenum C_passthru::CaptureVideo()
{
	static GLuint64EXT captureTime;
	GLuint sequenceNum;    
	static GLuint prevSequenceNum = 0;
	GLenum ret;
	static int numFails = 0;
	static int numTries = 0;
	static bool bShowMessageBox = true;  

	if(numFails < 100)
	{
		// Capture the video to a buffer object
		ret = m_SDIin.Capture(&sequenceNum, &captureTime);
		if(sequenceNum - prevSequenceNum > 1)
			printf("glVideoCaptureNV: Dropped %d frames\n",sequenceNum - prevSequenceNum);
		prevSequenceNum = sequenceNum;
		switch(ret) {
			case GL_SUCCESS_NV:
#ifdef _DEBUG
				//				printf("Frame:%d gpuTime:%f gviTime:%f\n", sequenceNum, m_SDIin.m_gpuTime,m_SDIin.m_gviTime);
#endif
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

		//captureAnc();
	}
	// The incoming signal format or some other error occurred during
	// capture, shutdown and try to restart capture.
	else {
		if(numTries == 0)
		{		
			StopSDIPipeline();
			CleanupSDIDevices();			
			CleanupGL();	
		}
		// Initialize the video capture device.
		if (m_SDIin.SetupDevice(bShowMessageBox,options.captureDevice) != S_OK)
		{
			bShowMessageBox = false;
			numTries++;
			return GL_FAILURE_NV;
		}
		// Reinitialize OpenGL.
		SetupGL();		
		StartSDIPipeline();
		numFails = 0;
		numTries = 0;
		bShowMessageBox = true;
		return GL_FAILURE_NV;
	}
	return ret;
}
GLboolean C_passthru::DisplayVideo()
{
	// Send to SDI output
	if (m_bSDIout && glPresentFrameKeyedNV) 
	{
		GLenum l_eVal = GL_NO_ERROR;

		// Bind video texture object
		glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_videoTextures[0]);
		glEnable(GL_TEXTURE_RECTANGLE_NV);  

		// Send ANC data
		if (m_bAncOutInitialized) {
			m_ancData[0].fields = NVVIOANCDATAFRAME_VITC;
			m_ancData[0].fields |= NVVIOANCDATAFRAME_AUDIO_GROUP_1;
			NvVIOANCAPI_SendANCData(NULL, m_ancData);
		}

		glPresentFrameKeyedNV(1, 0, NULL, NULL,
			GL_FRAME_NV,
			GL_TEXTURE_RECTANGLE_NV, m_videoTextures[0], 0,
			GL_NONE, 0, 0);
		l_eVal = glGetError();
		if (l_eVal != GL_NO_ERROR) {
			fprintf(stderr, "glPresentFameKeyedNV returned error\n");
		}
	}

	//
	// Draw texture contents to graphics window.
	//

	// Reset view parameters
	glViewport(0, 0, m_windowWidth, m_windowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw contents of each video texture
	glEnable(GL_TEXTURE_RECTANGLE_NV);  

	// Set draw color
	glColor3f(1.0f, 1.0f, 1.0f);

	// Draw captured streams to graphics window.
	switch(m_SDIin.GetNumStreams()) {
		case 1:
			drawOne();
			break;
		case 2:
			drawTwo();
			break;
		case 3:
			drawThree();
			break;
		case 4:
			drawFour();
			break;
		default:
			drawOne();
	};

	SwapBuffers(m_hDC);

	glDisable(GL_TEXTURE_RECTANGLE_NV);
	return GL_TRUE;
}


//
// Calculate the graphics window size.
//
void C_passthru::CalcWindowSize()
{  
	switch(m_SDIin.GetSignalFormat()) {
  case NVVIOSIGNALFORMAT_487I_59_94_SMPTE259_NTSC:
  case NVVIOSIGNALFORMAT_576I_50_00_SMPTE259_PAL:
	  if (m_SDIin.GetNumStreams() == 1) {
		  m_windowWidth = m_videoWidth; m_windowHeight = m_videoHeight;
	  } else if (m_SDIin.GetNumStreams() == 2) {
		  m_windowWidth = m_videoWidth; m_windowHeight = m_videoHeight<<1;
	  } else {
		  m_windowWidth = m_videoWidth<<1; m_windowHeight = m_videoHeight<<1;
	  }
	  break;

  case NVVIOSIGNALFORMAT_720P_59_94_SMPTE296:
  case NVVIOSIGNALFORMAT_720P_60_00_SMPTE296:
  case NVVIOSIGNALFORMAT_720P_50_00_SMPTE296:
  case NVVIOSIGNALFORMAT_720P_30_00_SMPTE296:
  case NVVIOSIGNALFORMAT_720P_29_97_SMPTE296:
  case NVVIOSIGNALFORMAT_720P_25_00_SMPTE296:
  case NVVIOSIGNALFORMAT_720P_24_00_SMPTE296:
  case NVVIOSIGNALFORMAT_720P_23_98_SMPTE296:
	  if (m_SDIin.GetNumStreams() == 1) {
		  m_windowWidth = m_videoWidth>>2; m_windowHeight = m_videoHeight>>2;
	  } else if (m_SDIin.GetNumStreams() == 2) {
		  m_windowWidth = m_videoWidth>>2; m_windowHeight = m_videoHeight>>1;
	  } else {
		  m_windowWidth = m_videoWidth>>1; m_windowHeight = m_videoHeight>>1;
	  }
	  break;

  case NVVIOSIGNALFORMAT_1035I_59_94_SMPTE260:
  case NVVIOSIGNALFORMAT_1035I_60_00_SMPTE260:
  case NVVIOSIGNALFORMAT_1080I_50_00_SMPTE295:
  case NVVIOSIGNALFORMAT_1080I_50_00_SMPTE274:
  case NVVIOSIGNALFORMAT_1080I_59_94_SMPTE274:
  case NVVIOSIGNALFORMAT_1080I_60_00_SMPTE274:
  case NVVIOSIGNALFORMAT_1080P_23_976_SMPTE274:
  case NVVIOSIGNALFORMAT_1080P_24_00_SMPTE274:
  case NVVIOSIGNALFORMAT_1080P_25_00_SMPTE274:
  case NVVIOSIGNALFORMAT_1080P_29_97_SMPTE274:
  case NVVIOSIGNALFORMAT_1080P_30_00_SMPTE274:
  case NVVIOSIGNALFORMAT_1080I_48_00_SMPTE274:
  case NVVIOSIGNALFORMAT_1080I_47_96_SMPTE274:
  case NVVIOSIGNALFORMAT_1080PSF_25_00_SMPTE274:
  case NVVIOSIGNALFORMAT_1080PSF_29_97_SMPTE274:
  case NVVIOSIGNALFORMAT_1080PSF_30_00_SMPTE274:
  case NVVIOSIGNALFORMAT_1080PSF_24_00_SMPTE274:
  case NVVIOSIGNALFORMAT_1080PSF_23_98_SMPTE274:
	  if (m_SDIin.GetNumStreams() == 1) {
		  m_windowWidth = m_videoWidth>>2; m_windowHeight = m_videoHeight>>2;
	  } else if (m_SDIin.GetNumStreams() == 2) {
		  m_windowWidth = m_videoWidth>>2; m_windowHeight = m_videoHeight>>1;
	  } else {
		  m_windowWidth = m_videoWidth>>1; m_windowHeight = m_videoHeight>>1;
	  }
	  break;

  case NVVIOSIGNALFORMAT_2048P_30_00_SMPTE372:
  case NVVIOSIGNALFORMAT_2048P_29_97_SMPTE372:
  case NVVIOSIGNALFORMAT_2048I_60_00_SMPTE372:
  case NVVIOSIGNALFORMAT_2048I_59_94_SMPTE372:
  case NVVIOSIGNALFORMAT_2048P_25_00_SMPTE372:
  case NVVIOSIGNALFORMAT_2048I_50_00_SMPTE372:
  case NVVIOSIGNALFORMAT_2048P_24_00_SMPTE372:
  case NVVIOSIGNALFORMAT_2048P_23_98_SMPTE372:
  case NVVIOSIGNALFORMAT_2048I_48_00_SMPTE372:
  case NVVIOSIGNALFORMAT_2048I_47_96_SMPTE372:
	  if (m_SDIin.GetNumStreams() == 1) {
		  m_windowWidth = m_videoWidth>>2; m_windowHeight = m_videoHeight>>2;
	  } else if (m_SDIin.GetNumStreams() == 2) {
		  m_windowWidth = m_videoWidth>>2; m_windowHeight = m_videoHeight>>1;
	  } else {
		  m_windowWidth = m_videoWidth>>1; m_windowHeight = m_videoHeight>>1;
	  }
	  break;

  default:
	  m_windowWidth = 500;
	  m_windowHeight = 500;
	}
}

//
// Handle window resize
//
void C_passthru::ResizeWindow(int width, int height)
{
	m_windowWidth = width;
	m_windowHeight = height;
}

//
// Main Window Callback Function 
//
LONG WINAPI MainWndProc ( 
						 HWND    hWnd, 
						 UINT    uMsg, 
						 WPARAM  wParam, 
						 LPARAM  lParam) 
{ 
	LONG    lRet = 1; 
	PAINTSTRUCT    ps; 
	RECT rect; 

	switch (uMsg) { 

	case WM_CREATE: 
		break; 

	case WM_PAINT: 
		BeginPaint(hWnd, &ps); 
		EndPaint(hWnd, &ps); 
		break; 

	case WM_SIZE: 
		GetClientRect(hWnd, &rect); 
		passthru.ResizeWindow(rect.right - rect.left, rect.bottom - rect.top);
		break; 

	case WM_CLOSE: 
		passthru.Shutdown();			
		PostQuitMessage (0);
		DestroyWindow (hWnd); 
		break; 

	case WM_DESTROY: 
		//		PostQuitMessage (0); 
		break; 

	case WM_KEYDOWN: 
		switch (wParam) { 
	case VK_LEFT: 
		break; 
	case VK_RIGHT: 
		break; 
	case VK_UP: 
		break; 
	case VK_DOWN: 
		break; 
	case VK_SPACE:
		break;
	case VK_ESCAPE:
		passthru.Shutdown();			
		PostQuitMessage (0);
		break;
		} 

	default: 
		lRet = (LONG)DefWindowProc (hWnd, uMsg, wParam, lParam); 
		break; 
	} 

	return lRet; 
} 
//
//Cleanup
//
HRESULT C_passthru::CleanupSDIDevices() 
{
	cleanupAncIn();

	cleanupAncOut();

	return S_OK;
}
void C_passthru::Shutdown()
{
	StopSDIPipeline();	
	CleanupGL();			
	CleanupSDIDevices();	
}

//
// Create window.
//
HWND C_passthru::SetupWindow(HINSTANCE hInstance, int x, int y, 
							 char *title)
{
	WNDCLASS   wndclass; 
	HWND	   hWnd;
	HDC	  hDC;								// Device context

	BOOL bStatus;
	unsigned int uiNumFormats;
	CHAR szAppName[]="OpenGL SDI Demo";

	int pixelformat; 	

	// Register the frame class.
	wndclass.style         = 0; 
	wndclass.lpfnWndProc   = (WNDPROC)MainWndProc; 
	wndclass.cbClsExtra    = 0; 
	wndclass.cbWndExtra    = 0; 
	wndclass.hInstance     = hInstance; 
	wndclass.hIcon         = LoadIcon (hInstance, szAppName); 
	wndclass.hCursor       = LoadCursor (NULL,IDC_ARROW); 
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); 
	wndclass.lpszMenuName  = szAppName; 
	wndclass.lpszClassName = szAppName; 

	if (!RegisterClass (&wndclass) ) 
		return NULL; 

	// Create initial window frame.
	m_hWnd = CreateWindow ( szAppName, title, 
		WS_CAPTION | WS_BORDER |  WS_SIZEBOX | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
		x, 
		y, 
		m_windowWidth, 
		m_windowHeight, 
		NULL, 
		NULL, 
		hInstance, 
		NULL ); 

	// Exit on error.
	if (!m_hWnd) 
		return NULL; 

	// Get device context for new window.
	hDC = GetDC(m_hWnd);

	PIXELFORMATDESCRIPTOR pfd =							// pfd Tells Windows How We Want Things To Be
	{
		sizeof (PIXELFORMATDESCRIPTOR),					// Size Of This Pixel Format Descriptor
		1,												// Version Number
		PFD_DRAW_TO_WINDOW |							// Format Must Support Window
		PFD_SUPPORT_OPENGL |							// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,								// Must Support Double Buffering
		PFD_TYPE_RGBA,									// Request An RGBA Format
		24,												// Select Our Color Depth
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

	// Choose pixel format.
	if ( (pixelformat = ChoosePixelFormat(hDC, &pfd)) == 0 ) 
	{ 
		MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK); 
		return FALSE; 
	} 

	// Set pixel format.
	if (SetPixelFormat(hDC, pixelformat, &pfd) == FALSE) 
	{ 
		MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK); 
		return FALSE; 
	} 


	// Release device context.
	ReleaseDC(m_hWnd, hDC);

	// Return window handle.
	return(m_hWnd);

}

//
// Initialize ANC data capture
//
HRESULT
C_passthru::initializeAncIn(void)
{
	if (m_bAncInInitialized) 
		return S_OK;

	// Initialize ANC API
	if (NvVIOANCAPI_InitializeGVI(m_SDIin.GetVioHandle()) != NVAPI_OK) {
		return E_FAIL;
	}

	// Get frame rate and initialize timecode counter
	float frameRate;
	if (m_SDIin.GetFrameRate(&frameRate) != S_OK) {
		return E_FAIL;
	}
	float l_fFrameRate = 0.0f;
	m_TimeCode.init((UINT)l_fFrameRate);

	// Initialize ancillary data structure for VITC and custom packet capture.
	for (int i = 0; i < 4; i++) {
		m_ancData[i].version = NVVIOANCDATAFRAME_VERSION;
		m_ancData[i].fields = NVVIOANCDATAFRAME_VITC | NVVIOANCDATAFRAME_CUSTOM;
	}

	// Set desired number of custom packets to capture and 
	// allocate space for the custom data per stream.
	for (int i = 0; i < 4; i++) {
		m_ancData[i].numCustomPackets = 10000;
		m_ancData[i].CustomPackets = (NVVIOANCDATAPACKET *)calloc(m_ancData[i].numCustomPackets, sizeof(NVVIOANCDATAPACKET));
	}

	// Determine the length of the audio sample sequence.
	NvVIOANCAPI_NumAudioSamples(m_SDIin.GetVioHandle(), 
		NVVIOANCAUDIO_SAMPLING_RATE_48_0,
		(NvU32 *)&m_SequenceLength, 
		NULL);

	// Allocate/reallocated required memory for the array to hold the number 
	// of  audio samples for each frame in a sequence.
	if (m_NumAudioSamples)
		free(m_NumAudioSamples);
	m_NumAudioSamples = (NvU32 *)calloc((size_t)m_SequenceLength, sizeof(NvU32));

	// Determine number of audio samples based on signal format
	// and audio sampling rate.
	NvVIOANCAPI_NumAudioSamples(m_SDIin.GetVioHandle(), 
		NVVIOANCAUDIO_SAMPLING_RATE_48_0,
		(NvU32 *)&m_SequenceLength, 
		(NvU32 *)m_NumAudioSamples);

	// Determine the maximum number of audio sample for any given frame.
	// Use this value when allocating space to store audio samples
	NvU32 maxNumAudioSamples = 0;
	for (unsigned int i = 0; i < m_SequenceLength; i++) {
		if (m_NumAudioSamples[i] > maxNumAudioSamples) {
			maxNumAudioSamples = m_NumAudioSamples[i];
		}
	}

	// Allocate space required for audio data packets for each audio group, four channels per group.  
	// Space required depends upon signal format and audio rate.  Set bit field to indicate 
	// desired audio channels to capture.
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			m_ancData[i].AudioGroup1.numAudioSamples = maxNumAudioSamples;
			m_ancData[i].AudioGroup1.audioData[j] = (NvU32 *)calloc(maxNumAudioSamples, sizeof(NvU32));
			m_ancData[i].fields |= NVVIOANCDATAFRAME_AUDIO_GROUP_1;

			m_ancData[i].AudioGroup2.numAudioSamples = maxNumAudioSamples;
			m_ancData[i].AudioGroup2.audioData[j] = (NvU32 *)calloc(maxNumAudioSamples, sizeof(NvU32));
			m_ancData[i].fields |= NVVIOANCDATAFRAME_AUDIO_GROUP_2;

			m_ancData[i].AudioGroup3.numAudioSamples = maxNumAudioSamples;
			m_ancData[i].AudioGroup3.audioData[j] = (NvU32 *)calloc(maxNumAudioSamples, sizeof(NvU32));
			m_ancData[i].fields |= NVVIOANCDATAFRAME_AUDIO_GROUP_3;

			m_ancData[i].AudioGroup4.numAudioSamples = maxNumAudioSamples;
			m_ancData[i].AudioGroup4.audioData[j] = (NvU32 *)calloc(maxNumAudioSamples, sizeof(NvU32));
			m_ancData[i].fields |= NVVIOANCDATAFRAME_AUDIO_GROUP_4;
		}
	}

	m_bAncInInitialized = TRUE;

	return S_OK;
}

HRESULT
C_passthru::initializeAncOut(void)
{
	if (!m_bAncOutInitialized) {

		// Initialize ANC API
		if (NvVIOANCAPI_InitializeGVO(m_SDIout.getHandle()) != NVAPI_OK) {
			return E_FAIL;
		}
	}

	m_bAncOutInitialized = TRUE;

	return S_OK;
}

HRESULT
C_passthru::SetupAnc(void)
{
	if (initializeAncIn() == E_FAIL) {
		return E_FAIL;
	}

	if (m_bSDIout) {
		initializeAncOut();
	}

	return S_OK;
}

//
// Cleanup ANC data capture
//
HRESULT
C_passthru::cleanupAncIn(void)
{
	if (m_bAncInInitialized) {

		m_bAncInInitialized = FALSE;

		if (m_NumAudioSamples) {
			free(m_NumAudioSamples);
			m_NumAudioSamples = NULL;
			m_SequenceLength = 0;
		}

		// Free custom data space
		for(int i = 0; i < 4; i++) {
			free(m_ancData[i].CustomPackets);
		}

		// Release ANC API
		return (NvVIOANCAPI_ReleaseGVI(m_SDIin.GetVioHandle()));
	} else {
		return S_OK;
	}
}

HRESULT
C_passthru::cleanupAncOut(void)
{
	if (m_bAncOutInitialized) {

		m_bAncOutInitialized = FALSE;

		return (NvVIOANCAPI_ReleaseGVO(m_SDIout.getHandle()));

	}

	return S_OK;
}

HRESULT
C_passthru::CleanupAnc(void)
{
	cleanupAncIn();

	cleanupAncOut();

	return S_OK;
}


//
// Capture frame of ANC data.
//
void 
C_passthru::captureAnc()
{
	static int counter = 0;
	int frameTens;
	int frameUnits;
	int secondTens;
	int secondUnits;
	int minuteTens;  
	int minuteUnits;
	int hourTens;
	int hourUnits;

	// Capture ANC data	
	m_ancData->numCustomPackets = 10000;

	// Free custom data space
	for(int i = 0; i < 4; i++) {
		memset(m_ancData[i].CustomPackets, 0, m_ancData->numCustomPackets * sizeof(NVVIOANCDATAPACKET));
	}

	m_ancData[0].fields = NVVIOANCDATAFRAME_VITC | NVVIOANCDATAFRAME_AUDIO_GROUP_1;
	m_ancData[0].VITCTimecode = 0;

	NvVIOANCAPI_CaptureANCData(m_SDIin.GetVioHandle(), m_ancData);

	//printf("\nSTART FRAME\n");

	// Print timecode.  If captured timecode is not 0, use it, 
	// otherwise use internal local timecode.
	if (m_ancData[0].VITCTimecode != 0) {

		// Draw timecode for first video stream
		frameTens = (m_ancData[0].VITCTimecode >> 4) & 0x3;
		frameUnits = m_ancData[0].VITCTimecode & 0xf;
		secondTens = (m_ancData[0].VITCTimecode >> 12) & 0x7;
		secondUnits = (m_ancData[0].VITCTimecode >> 8) & 0xf;
		minuteTens = (m_ancData[0].VITCTimecode >> 20) & 0x7;  
		minuteUnits = (m_ancData[0].VITCTimecode >> 16) & 0xf;
		hourTens = (m_ancData[0].VITCTimecode >> 28)& 0x3;
		hourUnits = (m_ancData[0].VITCTimecode >> 24) & 0xf;

		//printf("%d%d:%d%d:%d%d:%d%d\n", hourTens, hourUnits, minuteTens, minuteUnits, 
		//		secondTens, secondUnits, frameTens, frameUnits); 

	} else {

		frameTens = m_TimeCode.frame() / 10;
		frameUnits = m_TimeCode.frame() % 10;
		secondTens = m_TimeCode.second() / 10;
		secondUnits = m_TimeCode.second() % 10;
		minuteTens = m_TimeCode.minute() / 10;
		minuteUnits = m_TimeCode.minute() % 10;
		hourTens = m_TimeCode.hour() / 10;
		hourUnits = m_TimeCode.hour() % 10;

		// Print.  Asterisk denotes timecode is generated.
		//printf("%d%d:%d%d:%d%d:%d%d*\n", hourTens, hourUnits, minuteTens, minuteUnits, 
		//		secondTens, secondUnits, frameTens, frameUnits);  

		// Set relevant bits here
		short dropFrame = 1;
		short colorFrame = 1;
		short fieldPhase = 0;
		short bg0 = 1;
		short bg1 = 0;
		short bg2 = 1;

		m_ancData[0].VITCTimecode = hourTens << 28 | hourUnits << 24 |  minuteTens << 20 |
			minuteUnits << 16 | secondTens << 12 | secondUnits << 8 | 
			frameTens << 4 | frameUnits;
		m_ancData[0].VITCTimecode |= dropFrame << 6;
		m_ancData[0].VITCTimecode |= colorFrame << 7;
		m_ancData[0].VITCTimecode |= fieldPhase << 15;
		m_ancData[0].VITCTimecode |= bg0 << 23;
		m_ancData[0].VITCTimecode |= bg1 << 30;
		m_ancData[0].VITCTimecode |= bg2 << 31;

		m_ancData[0].VITCUserBytes = counter;
		counter++;

		// Increment the generated timecode.
		m_TimeCode.increment();


	}

#ifdef DUMP_CUSTOM

	printf("Num Custom Packets: %d\n", m_ancData[0].numCustomPackets);
	for (unsigned int i = 0; i < m_ancData[0].numCustomPackets; i++) {
		for (unsigned int j = 0; j < m_ancData[0].CustomPackets[i].DC; j++) {

			// Get rid of 0s.
			//			if (m_ancData[0].CustomPackets[i].data[j])
			printf("%d", m_ancData[0].CustomPackets[i].data[j]);
		}
		printf("\n");
	}

#endif

#define DUMP_AUDIO
#ifdef DUMP_AUDIO

	NvU32 *a1 = m_ancData[0].AudioGroup1.audioData[0];
	NvU32 *a2 = m_ancData[0].AudioGroup1.audioData[1];
	NvU32 *a3 = m_ancData[0].AudioGroup1.audioData[2];
	NvU32 *a4 = m_ancData[0].AudioGroup1.audioData[3];

	printf("Number of audio samples: %d\n", m_ancData[0].AudioGroup1.numAudioSamples);

#if 0
	for (unsigned int i = 0; i < m_ancData[0].AudioGroup1.numAudioSamples; i++) {
#if 0
		printf(" %0x %0x %0x %0x", *a1++, *a2++, *a3++, *a4++);
#else
		// SMPTE 299
		//printf(" %0x ", (*a1++ & 0xffff0));
		// SMPTE 272
		printf(" %0x ", (*a1++ & 0x7ffff8));
#endif
	}
#endif

#endif

	//
}

//
// Draw single stream in graphics window.
//
GLvoid C_passthru::drawOne()
{
	// Enable texture
	glEnable(GL_TEXTURE_RECTANGLE_NV);  

	// Bind texture object for first video stream
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_videoTextures[0]);  

	// Draw textured quad in lower left quadrant of graphics window.
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(-1, -1); 
	glTexCoord2f(0.0, (GLfloat)m_videoHeight); glVertex2f(-1, 1); 
	glTexCoord2f((GLfloat)m_videoWidth, (GLfloat)m_videoHeight); glVertex2f(1, 1); 
	glTexCoord2f((GLfloat)m_videoWidth, 0.0); glVertex2f(1, -1); 
	glEnd();

	assert(glGetError() == GL_NO_ERROR);

	// Disable texture
	glDisable(GL_TEXTURE_RECTANGLE_NV);

	// Draw timecode for first video stream
	int frameTens = (m_ancData[0].VITCTimecode >> 4) & 0x3;
	int frameUnits = m_ancData[0].VITCTimecode & 0xf;
	int secondTens = (m_ancData[0].VITCTimecode >> 12) & 0x7;
	int secondUnits = (m_ancData[0].VITCTimecode >> 8) & 0xf;
	int minuteTens = (m_ancData[0].VITCTimecode >> 20) & 0x7;  
	int minuteUnits = (m_ancData[0].VITCTimecode >> 16) & 0xf;
	int hourTens = (m_ancData[0].VITCTimecode >> 28)& 0x3;
	int hourUnits = (m_ancData[0].VITCTimecode >> 24) & 0xf;

	size_t len;
	char buf[32];
	sprintf(buf, "%d%d:%d%d:%d%d:%d%d", hourTens, hourUnits, minuteTens, minuteUnits, 
		secondTens, secondUnits, frameTens, frameUnits);  
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(-1.0f, -1.0f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);

	// Draw custom packet count for first video stream
	sprintf(buf, "Custom Packet Count:  %d", m_ancData[0].numCustomPackets);
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(-1.0f, 0.9f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);
}

//
// Draw two streams in graphics window.
// Video streams are stacked on atop the other.
//
GLvoid C_passthru::drawTwo()
{
	// Enable texture
	glEnable(GL_TEXTURE_RECTANGLE_NV);  

	// Bind texture object for first video stream
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_videoTextures[1]);

	assert(glGetError() == GL_NO_ERROR);

	// Draw textured quad in lower left quadrant of graphics window.
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(-1, -1); 
	glTexCoord2f(0.0, (GLfloat)m_videoHeight); glVertex2f(-1, 0); 
	glTexCoord2f((GLfloat)m_videoWidth, (GLfloat)m_videoHeight); glVertex2f(1, 0); 
	glTexCoord2f((GLfloat)m_videoWidth, 0.0); glVertex2f(1, -1); 
	glEnd();

	assert(glGetError() == GL_NO_ERROR);

	// Bind texture object for second video stream
	glBindTexture(GL_TEXTURE_RECTANGLE_NV,m_videoTextures[0]);

	assert(glGetError() == GL_NO_ERROR);

	// Draw textured quad in lower right quadrant of graphics window.
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(-1, 0); 
	glTexCoord2f(0.0, (GLfloat)m_videoHeight); glVertex2f(-1, 1); 
	glTexCoord2f((GLfloat)m_videoWidth, (GLfloat)m_videoHeight); glVertex2f(1, 1); 
	glTexCoord2f((GLfloat)m_videoWidth, 0.0); glVertex2f(1, 0); 
	glEnd();

	assert(glGetError() == GL_NO_ERROR);

	// Disable texture
	glDisable(GL_TEXTURE_RECTANGLE_NV);

	// Draw timecode for first video stream
	int frameTens = (m_ancData[0].VITCTimecode >> 4) & 0x3;
	int frameUnits = m_ancData[0].VITCTimecode & 0xf;
	int secondTens = (m_ancData[0].VITCTimecode >> 12) & 0x7;
	int secondUnits = (m_ancData[0].VITCTimecode >> 8) & 0xf;
	int minuteTens = (m_ancData[0].VITCTimecode >> 20) & 0x7;  
	int minuteUnits = (m_ancData[0].VITCTimecode >> 16) & 0xf;
	int hourTens = (m_ancData[0].VITCTimecode >> 28)& 0x3;
	int hourUnits = (m_ancData[0].VITCTimecode >> 24) & 0xf;

	size_t len;
	char buf[32];
	sprintf(buf, "%d%d:%d%d:%d%d:%d%d", hourTens, hourUnits, minuteTens, minuteUnits, 
		secondTens, secondUnits, frameTens, frameUnits);  
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(-1.0f, 0.0f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);

	// Draw custom packet count for first video stream
	sprintf(buf, "Custom Packet Count:  %d", m_ancData[0].numCustomPackets);
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(-1.0f, 0.9f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);

	// Draw timecode for second video stream
	frameTens = (m_ancData[1].VITCTimecode >> 4) & 0x3;
	frameUnits = m_ancData[1].VITCTimecode & 0xf;
	secondTens = (m_ancData[1].VITCTimecode >> 12) & 0x7;
	secondUnits = (m_ancData[1].VITCTimecode >> 8) & 0xf;
	minuteTens = (m_ancData[1].VITCTimecode >> 20) & 0x7;  
	minuteUnits = (m_ancData[1].VITCTimecode >> 16) & 0xf;
	hourTens = (m_ancData[1].VITCTimecode >> 28)& 0x3;
	hourUnits = (m_ancData[1].VITCTimecode >> 24) & 0xf;

	sprintf(buf, "%d%d:%d%d:%d%d:%d%d", hourTens, hourUnits, minuteTens, minuteUnits, 
		secondTens, secondUnits, frameTens, frameUnits);  
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(-1.0f, -1.0f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);

	// Draw custom packet count for second video stream
	sprintf(buf, "Custom Packet Count:  %d", m_ancData[1].numCustomPackets);
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(-1.0f, -0.1f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);
}

//
// Draw three streams in graphics window.
// Use 3 quadrants with the remaining quadrant black
//
GLvoid C_passthru::drawThree()
{
	// Enable texture
	glEnable(GL_TEXTURE_RECTANGLE_NV); 

	// Bind texture object for first video stream
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_videoTextures[2]);

	// Draw textured quad in lower left quadrant of graphics window.
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(-1, -1); 
	glTexCoord2f(0.0, (GLfloat)m_videoHeight); glVertex2f(-1, 0); 
	glTexCoord2f((GLfloat)m_videoWidth, (GLfloat)m_videoHeight); glVertex2f(0, 0); 
	glTexCoord2f((GLfloat)m_videoWidth, 0.0); glVertex2f(0, -1); 
	glEnd();

	assert(glGetError() == GL_NO_ERROR);

	// Bind texture object for second video stream
	glBindTexture(GL_TEXTURE_RECTANGLE_NV,m_videoTextures[1]);

	// Draw textured quad in lower right quadrant of graphics window.
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(0, -1); 
	glTexCoord2f(0.0, (GLfloat)m_videoHeight); glVertex2f(0, 0); 
	glTexCoord2f((GLfloat)m_videoWidth, (GLfloat)m_videoHeight); glVertex2f(1, 0); 
	glTexCoord2f((GLfloat)m_videoWidth, 0.0); glVertex2f(1, -1); 
	glEnd();

	assert(glGetError() == GL_NO_ERROR);

	// Bind texture object for third video stream
	glBindTexture(GL_TEXTURE_RECTANGLE_NV,m_videoTextures[0]);

	// Draw textured quad in upper right quadrant of graphics window.
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(-1, 0); 
	glTexCoord2f(0.0, (GLfloat)m_videoHeight); glVertex2f(-1, 1); 
	glTexCoord2f((GLfloat)m_videoWidth, (GLfloat)m_videoHeight); glVertex2f(0, 1); 
	glTexCoord2f((GLfloat)m_videoWidth, 0.0); glVertex2f(0, 0); 
	glEnd();

	assert(glGetError() == GL_NO_ERROR);

	glBindTexture(GL_TEXTURE_RECTANGLE_NV, NULL);

	// Disable texture
	glDisable(GL_TEXTURE_RECTANGLE_NV);

	// Disable texture
	glDisable(GL_TEXTURE_RECTANGLE_NV);

	// Draw timecode for first video stream
	int frameTens = (m_ancData[0].VITCTimecode >> 4) & 0x3;
	int frameUnits = m_ancData[0].VITCTimecode & 0xf;
	int secondTens = (m_ancData[0].VITCTimecode >> 12) & 0x7;
	int secondUnits = (m_ancData[0].VITCTimecode >> 8) & 0xf;
	int minuteTens = (m_ancData[0].VITCTimecode >> 20) & 0x7;  
	int minuteUnits = (m_ancData[0].VITCTimecode >> 16) & 0xf;
	int hourTens = (m_ancData[0].VITCTimecode >> 28)& 0x3;
	int hourUnits = (m_ancData[0].VITCTimecode >> 24) & 0xf;

	size_t len;
	char buf[32];
	sprintf(buf, "%d%d:%d%d:%d%d:%d%d", hourTens, hourUnits, minuteTens, minuteUnits, 
		secondTens, secondUnits, frameTens, frameUnits);  
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(-1.0f, 0.0f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);

	// Draw custom packet count for first video stream
	sprintf(buf, "Custom Packet Count:  %d", m_ancData[0].numCustomPackets);
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(-1.0f, 0.9f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);

	// Draw timecode for second video stream
	frameTens = (m_ancData[1].VITCTimecode >> 4) & 0x3;
	frameUnits = m_ancData[1].VITCTimecode & 0xf;
	secondTens = (m_ancData[1].VITCTimecode >> 12) & 0x7;
	secondUnits = (m_ancData[1].VITCTimecode >> 8) & 0xf;
	minuteTens = (m_ancData[1].VITCTimecode >> 20) & 0x7;  
	minuteUnits = (m_ancData[1].VITCTimecode >> 16) & 0xf;
	hourTens = (m_ancData[1].VITCTimecode >> 28)& 0x3;
	hourUnits = (m_ancData[1].VITCTimecode >> 24) & 0xf;

	sprintf(buf, "%d%d:%d%d:%d%d:%d%d", hourTens, hourUnits, minuteTens, minuteUnits, 
		secondTens, secondUnits, frameTens, frameUnits);  
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(-1.0f, -1.0f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);

	// Draw custom packet count for second video stream
	sprintf(buf, "Custom Packet Count:  %d", m_ancData[1].numCustomPackets);
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(-1.0f, -0.1f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);

	// Draw timecode for third video stream
	frameTens = (m_ancData[2].VITCTimecode >> 4) & 0x3;
	frameUnits = m_ancData[2].VITCTimecode & 0xf;
	secondTens = (m_ancData[2].VITCTimecode >> 12) & 0x7;
	secondUnits = (m_ancData[2].VITCTimecode >> 8) & 0xf;
	minuteTens = (m_ancData[2].VITCTimecode >> 20) & 0x7;  
	minuteUnits = (m_ancData[2].VITCTimecode >> 16) & 0xf;
	hourTens = (m_ancData[2].VITCTimecode >> 28)& 0x3;
	hourUnits = (m_ancData[2].VITCTimecode >> 24) & 0xf;

	sprintf(buf, "%d%d:%d%d:%d%d:%d%d", hourTens, hourUnits, minuteTens, minuteUnits, 
		secondTens, secondUnits, frameTens, frameUnits);  
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(0.0f, 0.0f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);

	// Draw custom packet count for third video stream
	sprintf(buf, "Custom Packet Count:  %d", m_ancData[2].numCustomPackets);
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(0.0f, 0.9f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);
}

//
// Draw four streams tiled in graphics window.
// One stream is drawn in each quadrant.
//
GLvoid C_passthru::drawFour()
{
	// Enable texture
	glEnable(GL_TEXTURE_RECTANGLE_NV); 

	// Bind texture object for first video stream
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_videoTextures[1]);

	// Draw textured quad in lower left quadrant of graphics window.
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(-1, -1); 
	glTexCoord2f(0.0, (GLfloat)m_videoHeight); glVertex2f(-1, 0); 
	glTexCoord2f((GLfloat)m_videoWidth, (GLfloat)m_videoHeight); glVertex2f(0, 0); 
	glTexCoord2f((GLfloat)m_videoWidth, 0.0); glVertex2f(0, -1); 
	glEnd();

	assert(glGetError() == GL_NO_ERROR);

	// Bind texture object for second video stream
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_videoTextures[3]);

	// Draw textured quad in lower right quadrant of graphics window.
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(0, -1); 
	glTexCoord2f(0.0, (GLfloat)m_videoHeight); glVertex2f(0, 0); 
	glTexCoord2f((GLfloat)m_videoWidth, (GLfloat)m_videoHeight); glVertex2f(1, 0); 
	glTexCoord2f((GLfloat)m_videoWidth, 0.0); glVertex2f(1, -1); 
	glEnd();

	assert(glGetError() == GL_NO_ERROR);

	// Bind texture object for third video stream
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_videoTextures[0]);

	// Draw textured quad in upper right quadrant of graphics window.
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(-1, 0); 
	glTexCoord2f(0.0, (GLfloat)m_videoHeight); glVertex2f(-1, 1); 
	glTexCoord2f((GLfloat)m_videoWidth, (GLfloat)m_videoHeight); glVertex2f(0, 1); 
	glTexCoord2f((GLfloat)m_videoWidth, 0.0); glVertex2f(0, 0); 
	glEnd();

	assert(glGetError() == GL_NO_ERROR);

	// Bind texture object for fourth video stream
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_videoTextures[2]);

	//glColor3f(1.0, 1.0, 0.0);

	// Draw textured quad in upper right quadrant of graphics window.
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(0, 0); 
	glTexCoord2f(0.0, (GLfloat)m_videoHeight); glVertex2f(0, 1); 
	glTexCoord2f((GLfloat)m_videoWidth, (GLfloat)m_videoHeight); glVertex2f(1, 1); 
	glTexCoord2f((GLfloat)m_videoWidth, 0.0); glVertex2f(1, 0); 
	glEnd();

	assert(glGetError() == GL_NO_ERROR);

	glBindTexture(GL_TEXTURE_RECTANGLE_NV, NULL);

	// Disable texture
	glDisable(GL_TEXTURE_RECTANGLE_NV);

	// Disable texture
	glDisable(GL_TEXTURE_RECTANGLE_NV);

	// Draw timecode for first video stream
	int frameTens = (m_ancData[0].VITCTimecode >> 4) & 0x3;
	int frameUnits = m_ancData[0].VITCTimecode & 0xf;
	int secondTens = (m_ancData[0].VITCTimecode >> 12) & 0x7;
	int secondUnits = (m_ancData[0].VITCTimecode >> 8) & 0xf;
	int minuteTens = (m_ancData[0].VITCTimecode >> 20) & 0x7;  
	int minuteUnits = (m_ancData[0].VITCTimecode >> 16) & 0xf;
	int hourTens = (m_ancData[0].VITCTimecode >> 28)& 0x3;
	int hourUnits = (m_ancData[0].VITCTimecode >> 24) & 0xf;

	size_t len;
	char buf[32];
	sprintf(buf, "%d%d:%d%d:%d%d:%d%d", hourTens, hourUnits, minuteTens, minuteUnits, 
		secondTens, secondUnits, frameTens, frameUnits);  
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(-1.0f, 0.0f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);

	// Draw custom packet count for first video stream
	sprintf(buf, "Custom Packet Count:  %d", m_ancData[0].numCustomPackets);
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(-1.0f, 0.9f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);

	// Draw timecode for second video stream
	frameTens = (m_ancData[1].VITCTimecode >> 4) & 0x3;
	frameUnits = m_ancData[1].VITCTimecode & 0xf;
	secondTens = (m_ancData[1].VITCTimecode >> 12) & 0x7;
	secondUnits = (m_ancData[1].VITCTimecode >> 8) & 0xf;
	minuteTens = (m_ancData[1].VITCTimecode >> 20) & 0x7;  
	minuteUnits = (m_ancData[1].VITCTimecode >> 16) & 0xf;
	hourTens = (m_ancData[1].VITCTimecode >> 28)& 0x3;
	hourUnits = (m_ancData[1].VITCTimecode >> 24) & 0xf;

	sprintf(buf, "%d%d:%d%d:%d%d:%d%d", hourTens, hourUnits, minuteTens, minuteUnits, 
		secondTens, secondUnits, frameTens, frameUnits);  
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(-1.0f, -1.0f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);

	// Draw custom packet count for second video stream
	sprintf(buf, "Custom Packet Count:  %d", m_ancData[1].numCustomPackets);
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(-1.0f, -0.1f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);

	// Draw timecode for third video stream
	frameTens = (m_ancData[2].VITCTimecode >> 4) & 0x3;
	frameUnits = m_ancData[2].VITCTimecode & 0xf;
	secondTens = (m_ancData[2].VITCTimecode >> 12) & 0x7;
	secondUnits = (m_ancData[2].VITCTimecode >> 8) & 0xf;
	minuteTens = (m_ancData[2].VITCTimecode >> 20) & 0x7;  
	minuteUnits = (m_ancData[2].VITCTimecode >> 16) & 0xf;
	hourTens = (m_ancData[2].VITCTimecode >> 28)& 0x3;
	hourUnits = (m_ancData[2].VITCTimecode >> 24) & 0xf;

	sprintf(buf, "%d%d:%d%d:%d%d:%d%d", hourTens, hourUnits, minuteTens, minuteUnits, 
		secondTens, secondUnits, frameTens, frameUnits);  
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(0.0f, 0.0f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);

	// Draw custom packet count for third video stream
	sprintf(buf, "Custom Packet Count:  %d", m_ancData[2].numCustomPackets);
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(0.0f, 0.9f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);

	// Draw timecode for fourth video stream
	frameTens = (m_ancData[3].VITCTimecode >> 4) & 0x3;
	frameUnits = m_ancData[3].VITCTimecode & 0xf;
	secondTens = (m_ancData[3].VITCTimecode >> 12) & 0x7;
	secondUnits = (m_ancData[3].VITCTimecode >> 8) & 0xf;
	minuteTens = (m_ancData[3].VITCTimecode >> 20) & 0x7;  
	minuteUnits = (m_ancData[3].VITCTimecode >> 16) & 0xf;
	hourTens = (m_ancData[3].VITCTimecode >> 28)& 0x3;
	hourUnits = (m_ancData[3].VITCTimecode >> 24) & 0xf;

	sprintf(buf, "%d%d:%d%d:%d%d:%d%d", hourTens, hourUnits, minuteTens, minuteUnits, 
		secondTens, secondUnits, frameTens, frameUnits);  
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(0.0f, -1.0f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);

	// Draw custom packet count for fourth video stream
	sprintf(buf, "Custom Packet Count:  %d", m_ancData[3].numCustomPackets);
	len = strlen(buf);
	glListBase (1000);
	glColor3f(1.0f, 1.0f, 0.0f);
	glRasterPos2f(0.0f, -0.1f);
	glCallLists ((GLsizei)len, GL_UNSIGNED_BYTE, buf);
}




#if 0

//
// Main function.
//
//
// WinMain
//
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	MSG        msg; 
	HWND g_hWnd;

	// Debug console.
#ifdef _DEBUG
	SetupConsole();
#endif

	if(passthru.Configure(&lpCmdLine) == E_FAIL)
		return FALSE;

	if(passthru.SetupSDIDevices() == E_FAIL)
		return FALSE;

	// Calculate the window size based on the incoming and outgoing video signals
	//passthru.CalcWindowSize();
	passthru.m_windowWidth = 1920;
	passthru.m_windowHeight = 1080;

	// Create window.  Use video dimensions of video initialized above.
	g_hWnd = passthru.SetupWindow(hInstance, 0, 0, "NVIDIA Quadro SDI Capture to memory");

	// Exit on error.
	if (!g_hWnd) 
		return FALSE; 

	passthru.SetupGL();

	// 	if (passthru.SetupAnc() == E_FAIL)
	// 		return FALSE;

	if(passthru.StartSDIPipeline() == E_FAIL)
		return FALSE;

	// Show window.
	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	// Animation loop.
	while (1) {

		//
		//  Process all pending messages 
		// 
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) == TRUE) { 
			if (GetMessage(&msg, NULL, 0, 0) ) { 
				TranslateMessage(&msg); 
				DispatchMessage(&msg);
			} else { 
				return TRUE; 
			} 
		} 
		if(passthru.CaptureVideo() != GL_FAILURE_NV)
		{
			passthru.DisplayVideo();
		}
	} 
}
#endif
