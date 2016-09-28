

#include "SdiInAsyncWindow.h"
#include "nvSDIutil.h"
#include "glExtensions.h"
#include "GLNvSdi.h"
#include "GLFont.h"

GLFont gFont;

#define BLACK 0, 0, 0
#define WHITE 1, 1, 1
#define GRAY 0.3f, 0.3f, 0.3f

//
// Calculate scaled video dimensions that preserve aspect ratio
//
static void ComputeScaledVideoDimensions(unsigned int ww, unsigned int wh, unsigned int vw, unsigned int vh, float *svw, float *svh)
{
	float fww = ww;
	float fwh = wh;
	float fvw = vw;
	float fvh = vh;

	// Set the scale video width to the window width.
	// Scale the video height by the aspect ratio.
	// If the resulting height is greater than the
	// window height, the set the video height to 
	// the window height and scale the width by the
	// video aspect ratio.
	*svw = fww;
	*svh = (fvh / fvw) * *svw;
	if (*svh > wh) {
		*svh = fwh;
		*svw = (fvw / fvh) * *svh;
	}

	// Normalize
	*svh /= fwh;
	*svw /= fww;
}


SdiInAsyncWindow::SdiInAsyncWindow() : GLWindow()
{
}

SdiInAsyncWindow::~SdiInAsyncWindow()
{


}


bool SdiInAsyncWindow::InitGL()
{
	MakeCurrent();
	SdiSetDC(GetDC());

	// Create window device context and rendering context.
	m_ShowStatistics = true;

	// Creating Affinity DC 

	HDC m_AffinityDC = SdiCreateAffinityDC();
	//HDC m_AffinityDC = this->GetDC();
	SdiCreateGLRC(m_AffinityDC);

	// Make window rendering context current.
	//wglMakeCurrent(hDC, l_hRC);
	SdiMakeCurrent();


	//load the required OpenGL extensions:
	if (!loadTimerQueryExtension() ||
		!loadBufferObjectExtension() ||
		!loadShaderObjectsExtension() ||
		!loadFramebufferObjectExtension() ||
		!loadCopyImageExtension() ||
		!loadSwapIntervalExtension())
	{
		std::cout << "Could not load the required OpenGL extensions" << std::endl;
		return false;
	}

	if (wglSwapIntervalEXT)
		wglSwapIntervalEXT(0);

	// Create bitmap font display list
	SelectObject(SdiGetDC(), GetStockObject(SYSTEM_FONT));
	SetDCBrushColor(SdiGetDC(), 0x0000ffff);  // 0x00bbggrr
	glColor3f(1.0f, 0.0f, 0.0);
	wglUseFontBitmaps(SdiGetDC(), 0, 255, 1000);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);

	glDisable(GL_DEPTH_TEST);

	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);

	glGenQueries(1, &m_drawTimeQuery);

	return true;
}




void SdiInAsyncWindow::Render()
{
}


GLboolean SdiInAsyncWindow::SetupGLPipe()
{

	m_pipe.SetupSDIinGL(SdiGetDC(), SdiGetGLRC());

	SdiMakeCurrent();

	int videoWidth = m_pipe.GetVideoWidth();
	int videoHeight = m_pipe.GetVideoHeight();

	// To view the buffers we need to load an appropriate shader
	m_pipe.SetupDecodeProgram();
	int activeDeviceCount = m_pipe.GetActiveDeviceCount();
	if (activeDeviceCount == 0)
		return GL_FALSE;
	//allocate the textures for display


	for (UINT i = 0; i < activeDeviceCount; i++) 
	{
		int numStreams = m_pipe.GetNumStreamsPerFrame(i);
#ifdef USE_ALL_STREAMS
		numStreams = NUM_VIDEO_STERAMS;
#endif		
		glGenTextures(numStreams, &m_pipe.m_DisplayTextures[i][0]);

		for (UINT j = 0; j < numStreams; j++) 
		{
			glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_pipe.m_DisplayTextures[i][j]);
			assert(glGetError() == GL_NO_ERROR);
			glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			assert(glGetError() == GL_NO_ERROR);

			glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA8, videoWidth, videoHeight,
				0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			assert(glGetError() == GL_NO_ERROR);
		}
	}

	//create the textures to go with the buffers and frame buffer objects to create the display textures
	for (UINT j = 0; j < activeDeviceCount; j++) 
	{
		int numStreams = m_pipe.GetNumStreamsPerFrame(j);
#ifdef USE_ALL_STREAMS	
		numStreams = NUM_VIDEO_STERAMS;
#endif		        
		glGenTextures(numStreams, &m_pipe.m_decodeTextures[j][0]);
		assert(glGetError() == GL_NO_ERROR);
		glGenFramebuffersEXT(numStreams, &m_pipe.m_vidFbos[j][0]);
		assert(glGetError() == GL_NO_ERROR);

		for (unsigned int i = 0; i < numStreams; i++) 
		{
			glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_pipe.m_decodeTextures[j][i]);
			assert(glGetError() == GL_NO_ERROR);
			glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			assert(glGetError() == GL_NO_ERROR);


			// Allocate storage for the decode texture.
			glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA8UI,
				(GLsizei)(videoWidth*0.5), videoHeight, 0,
				GL_RGBA_INTEGER_EXT, GL_UNSIGNED_BYTE, NULL);
			assert(glGetError() == GL_NO_ERROR);
			// Configure the decode->output FBO.
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_pipe.m_vidFbos[j][i]);
			assert(glGetError() == GL_NO_ERROR);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT,
				GL_TEXTURE_RECTANGLE_NV,
				m_pipe.m_DisplayTextures[j][i],
				0);
			assert(glGetError() == GL_NO_ERROR);

			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		}

	}

	return GL_TRUE;
}

GLboolean SdiInAsyncWindow::CleanupGL()
{

	m_pipe.CleanupSDIinGL();

	SdiMakeCurrent();

	int activeDeviceCount = m_pipe.GetActiveDeviceCount();
	if(activeDeviceCount == 0)
		return GL_FALSE;
	for (UINT i = 0; i < activeDeviceCount; i++) 
	{
		int numStreams = m_pipe.GetNumStreamsPerFrame(i);
#ifdef USE_ALL_STREAMS		
		numStreams = NUM_VIDEO_STERAMS;
#endif		  
		glDeleteTextures(numStreams, &m_pipe.m_DisplayTextures[i][0]);
		glDeleteTextures(numStreams, &m_pipe.m_decodeTextures[i][0]);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glDeleteFramebuffersEXT(numStreams, &m_pipe.m_vidFbos[i][0]);
	}
	m_pipe.DestroyDecodeProgram();

	glDeleteQueries(1, &m_drawTimeQuery);
	// Delete OpenGL rendering context.
	wglMakeCurrent(NULL, NULL);
	if (SdiGetGLRC())
	{
		wglDeleteContext(SdiGetGLRC());
		SdiSetGLRC(NULL);
	}

	ReleaseDC(this->hWnd, SdiGetDC());

	wglDeleteDCNV(SdiGetAffinityDC());

	return GL_TRUE;
}


bool SdiInAsyncWindow::IsSdiAvailable()
{
	HWND hWnd;
	HGLRC hGLRC;
	if (CreateDummyGLWindow(&hWnd, &hGLRC) == false)
		return false;

	int numGPUs;
	// Note, this function enumerates GPUs which are both CUDA & GLAffinity capable (i.e. newer Quadros)  
	numGPUs = CNvGpuTopology::instance().getNumGpu();

	if (numGPUs <= 0)
	{
		MessageBox(NULL, "Unable to obtain system GPU topology", "Error", MB_OK);
		return false;
	}

	int numCaptureDevices = CNvSDIinTopology::instance().getNumDevice();

	if (numCaptureDevices <= 0)
	{
		MessageBox(NULL, "Unable to obtain system Capture topology", "Error", MB_OK);
		return false;
	}


	if (m_pipe.m_options.captureGPU >= numGPUs)
	{
		MessageBox(NULL, "Selected GPU is out of range", "Error", MB_OK);
		return false;
	}


	// We can kill the dummy window now
	if (DestroyGLWindow(&hWnd, &hGLRC) == false)
		return false;

	return true;
}

HRESULT SdiInAsyncWindow::SetupSDIPipeline()
{
	//try to setup pipelines and for each one that we succeed with, push it into activePipelines deque.
	m_pipe.SetupSDIPipeline();

	return S_OK;
}

HRESULT SdiInAsyncWindow::CleanupSDIPipeline()
{
	m_pipe.CleanupSDIPipeline();
	return S_OK;

}

/////////////////////////////////////
// Pipeline Activation
/////////////////////////////////////

HRESULT SdiInAsyncWindow::StartSDIPipeline()
{
	// Start video capture
	if (m_pipe.StartSDIPipeline() != S_OK)
	{
		MessageBox(NULL, "Error starting video capture.", "Error", MB_OK);
		return E_FAIL;
	}
	return S_OK;
}
/////////////////////////////////////
// Pipeline deactivation
/////////////////////////////////////

HRESULT SdiInAsyncWindow::StopSDIPipeline()
{
	m_pipe.StopSDIPipeline();
	return S_OK;
}



GLboolean SdiInAsyncWindow::DisplayVideo()
{
	//
	// Draw texture contents to graphics window.
	//
	size_t len;
	char buf[512];

	SdiMakeCurrent();

	assert(glGetError() == GL_NO_ERROR);
	
	// Set draw color
	glColor3f(0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	assert(glGetError() == GL_NO_ERROR);

	int activeDeviceCount = m_pipe.GetActiveDeviceCount();
	if (activeDeviceCount == 0)
		return GL_FALSE;

	// Calculate scaled video dimensions.
	GLfloat l_scaledVideoWidth;
	GLfloat l_scaledVideoHeight;
	int maxNumStreams = 1;
	for (int i = 0; i < activeDeviceCount; i++)
	{
		int numStreams = m_pipe.GetNumStreamsPerFrame(i);
		if (numStreams > maxNumStreams)
			maxNumStreams = numStreams;
	}

	float rectW = this->Width() / (float)maxNumStreams;
	float rectH = this->Height() / (float)activeDeviceCount;


	static C_Frame *prevFrame[NVAPI_MAX_VIO_DEVICES];
	static GLuint numDroppedFrames[NVAPI_MAX_VIO_DEVICES];
	GLfloat left = 0, top = 0;
	int videoWidth = m_pipe.GetVideoWidth();
	int videoHeight = m_pipe.GetVideoHeight();

	NVVIOSIGNALFORMAT signalFormat = m_pipe.GetSignalFormat();
	static int count = 0;
	static float gpuDrawTime;
	static float cpuDrawTime;

	GLuint64EXT drawTimeStart;
	GLuint64EXT drawTimeEnd;


	assert(glGetError() == GL_NO_ERROR);
	glBeginQuery(GL_TIME_ELAPSED_EXT, m_drawTimeQuery);
	assert(glGetError() == GL_NO_ERROR);
	glGetInteger64v(GL_CURRENT_TIME_NV, (GLint64 *)&drawTimeStart);
	
	int devcount = 0;
	bool draw_time_update = false;
	float captureElapsedTime = 0;
	float totalUploadTime = 0, 
		totalDownloadTime = 0;
	for (int i = 0; i < activeDeviceCount; i++)
	{
		left = 0;
		top = i*rectH;


		//
		// Capture statistics
		//
		C_Frame *frame = m_pipe.GetFrame(i);

		if (frame != NULL)
		{

			if (prevFrame[i] != NULL)
			{
				numDroppedFrames[i] += frame->sequenceNum - prevFrame[i]->sequenceNum - 1;
				//printf("Dropped frames %d\n",numDroppedFrames[i]);
				m_pipe.ReleaseUsedFrame(i, prevFrame[i]);
			}

			prevFrame[i] = frame;

			captureElapsedTime += frame->captureElapsedTime;
			totalUploadTime += frame->uploadElapsedTime;
			totalDownloadTime += frame->downloadElapsedTime;
			devcount++;
			if (devcount == activeDeviceCount)
				draw_time_update = true;

		}
		else
		{
			if (prevFrame[i] == NULL)
				continue;
			else
				frame = prevFrame[i];
			draw_time_update = false;
		}
		


		ComputeScaledVideoDimensions(rectW, rectH,
			videoWidth, videoHeight,
			&l_scaledVideoWidth,
			&l_scaledVideoHeight);



		glColor3f(1.0f, 1.0f, 1.0f);

		glEnable(GL_TEXTURE_RECTANGLE_NV);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0, videoWidth, 0.0, videoHeight, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glViewport(0, 0, videoWidth, videoHeight);
		int numStreams = m_pipe.GetNumStreamsPerFrame(i);
		// First blit the buffer object into a texture and chroma expand
		GLint rowLength = frame->getPitch() / 4;
		glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);

#ifdef USE_ALL_STREAMS
		numStreams = NUM_VIDEO_STERAMS;
#endif		

		for (int j = 0; j < numStreams; j++)
		{
			// Blit the frame to the texture.

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, frame->getDstObject(j));

			assert(glGetError() == GL_NO_ERROR);

			glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_pipe.m_decodeTextures[i][j]);

			assert(glGetError() == GL_NO_ERROR);

			/// XXX
			glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0,
				0, 0, (GLsizei)(0.5*videoWidth), videoHeight,
				GL_RGBA_INTEGER_EXT, GL_UNSIGNED_BYTE, NULL);

			assert(glGetError() == GL_NO_ERROR);

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

			assert(glGetError() == GL_NO_ERROR);

			// Draw a textured quad to the FBO to do the chroma expansion
			// and colorspace conversion.
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_pipe.m_vidFbos[i][j]);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT,
				GL_TEXTURE_RECTANGLE_NV,
				m_pipe.m_DisplayTextures[i][j],
				0);

			glClear(GL_COLOR_BUFFER_BIT);

			glUseProgram(m_pipe.decodeProgram);


			assert(glGetError() == GL_NO_ERROR);

			glBegin(GL_QUADS);
			glVertex2f(0.0f, 0.0f);
			glVertex2f(0.0f, (float)videoHeight);
			glVertex2f((float)videoWidth, (float)videoHeight);
			glVertex2f((float)videoWidth, 0.0f);
			glEnd();

			assert(glGetError() == GL_NO_ERROR);

			glUseProgram(0);

			glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		}

		// Draw contents of each video texture
		// Reset view parameters

		glViewport(0, 0, this->Width(), this->Height());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		for (int j = 0; j < numStreams; j++)
		{
			// Set viewport .
			glViewport(left, top, rectW, rectH);

			left += rectW;


			// Bind texture object  video stream i

			glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_pipe.m_DisplayTextures[i][j]);

			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex2f(-l_scaledVideoWidth, -l_scaledVideoHeight);
			glTexCoord2f(0.0, (GLfloat)videoHeight); glVertex2f(-l_scaledVideoWidth, l_scaledVideoHeight);
			glTexCoord2f((GLfloat)videoWidth, (GLfloat)videoHeight); glVertex2f(l_scaledVideoWidth, l_scaledVideoHeight);
			glTexCoord2f((GLfloat)videoWidth, 0.0); glVertex2f(l_scaledVideoWidth, -l_scaledVideoHeight);
			glEnd();

			glBindTexture(GL_TEXTURE_RECTANGLE_NV, NULL);
			assert(glGetError() == GL_NO_ERROR);
		}

		glDisable(GL_TEXTURE_RECTANGLE_NV);
		//Draw 2D stuff like #of dropped frames,sequence number and so on.	
		if (m_ShowStatistics)
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			//gluOrtho2D(0.0, rectW, 0.0, rectH );
			gluOrtho2D(0.0, this->Width(), 0.0, this->Height());
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glViewport(0, 0, this->Width(), this->Height());

			// Draw video signal		
			sprintf(buf, "Card:%d", m_pipe.GetDeviceNumber(i));

			len = strlen(buf);
			glListBase(1000);
			glColor3f(1.0f, 1.0f, 0.0f);
			glRasterPos2f(10, rectH*(i + 1) - 20);
			glCallLists((GLsizei)len, GL_UNSIGNED_BYTE, buf);

			// Draw video signal
			len = strlen(SignalFormatToString(signalFormat).c_str());
			glListBase(1000);
			glColor3f(1.0f, 1.0f, 0.0f);
			glRasterPos2f(10, rectH*(i + 1) - 35);
			glCallLists((GLsizei)len, GL_UNSIGNED_BYTE, SignalFormatToString(signalFormat).c_str());

			// Draw sequence number
			sprintf(buf, "total:%d", frame->sequenceNum);
			len = strlen(buf);
			glListBase(1000);
			glColor3f(1.0f, 1.0f, 0.0f);
			glRasterPos2f(10, rectH*(i + 1) - 50);
			glCallLists((GLsizei)len, GL_UNSIGNED_BYTE, buf);

			// Draw dropped frames number
			sprintf(buf, "dropped at capture:%d", frame->numDroppedFrames);
			len = strlen(buf);
			glListBase(1000);
			glColor3f(1.0f, 1.0f, 0.0f);
			glRasterPos2f(10, rectH*(i + 1) - 65);
			glCallLists((GLsizei)len, GL_UNSIGNED_BYTE, buf);

			// Draw dropped frames number
			sprintf(buf, "dropped at render time:%d", numDroppedFrames[i]);
			len = strlen(buf);
			glListBase(1000);
			glColor3f(1.0f, 1.0f, 0.0f);
			glRasterPos2f(10, rectH*(i + 1) - 80);
			glCallLists((GLsizei)len, GL_UNSIGNED_BYTE, buf);
		}

	}
	assert(glGetError() == GL_NO_ERROR);
	glEndQuery(GL_TIME_ELAPSED_EXT);
	assert(glGetError() == GL_NO_ERROR);
	GLuint64EXT timeElapsed;
	assert(glGetError() == GL_NO_ERROR);
	glGetQueryObjectui64vEXT(m_drawTimeQuery, GL_QUERY_RESULT, &timeElapsed);
	glGetInteger64v(GL_CURRENT_TIME_NV, (GLint64 *)&drawTimeEnd);


	//::SwapBuffers(SdiGetDC());


	if (count % 15 == 0 && draw_time_update)
	{

		gpuDrawTime = timeElapsed *0.000000001;
		cpuDrawTime = (drawTimeEnd - drawTimeStart)*0.000000001;
		sprintf(buf, "MultiCaptureAsync | Draw Time %.5f | Capture Time %.5f | Upload Time %.5f | Download Time %.5f ", gpuDrawTime, captureElapsedTime, totalUploadTime, totalDownloadTime);
		SetWindowText(this->hWnd, buf);
	}

	count++;

	return GL_TRUE;
}


void SdiInAsyncWindow::Shutdown()
{
	StopSDIPipeline();
	CleanupSDIPipeline();
	CleanupGL();
}

void SdiInAsyncWindow::OnKeyEvent(const KeyEvent* pEvent)
{
	if(!pEvent->PressedDown)
		return;

	if(pEvent->Key==KEY_ESCAPE)
	{
		this->Close();
	}
}











