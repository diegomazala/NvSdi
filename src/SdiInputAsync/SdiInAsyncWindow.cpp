

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
	SdiSetGLRC(GetGLRC());

	// Create window device context and rendering context.
	showStatistics = true;

	// Creating Affinity DC 
	//HDC affinityDC = SdiCreateAffinityDC();
	//HDC affinityDC = this->GetDC();
	//SdiCreateGLRC(affinityDC);

	// Make window rendering context current.
	//wglMakeCurrent(hDC, l_hRC);
	//SdiMakeCurrent();


	// load the required OpenGL extensions:
	if (!loadSwapIntervalExtension())
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

	return true;
}




void SdiInAsyncWindow::Render()
{
	MakeCurrent();

	//
	// Draw texture contents to graphics window.
	//
	size_t len;
	char buf[512];

	assert(glGetError() == GL_NO_ERROR);
	
	// Set draw color
	glColor3f(0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	assert(glGetError() == GL_NO_ERROR);

	int activeDeviceCount = DvpActiveDeviceCount();
	if (activeDeviceCount == 0)
		return;

	// Calculate scaled video dimensions.
	GLfloat l_scaledVideoWidth;
	GLfloat l_scaledVideoHeight;
	int maxNumStreams = 1;
	for (int i = 0; i < activeDeviceCount; i++)
	{
		int numStreams = DvpStreamsPerFrame(i);
		if (numStreams > maxNumStreams)
			maxNumStreams = numStreams;
	}

	float rectW = this->Width() / (float)maxNumStreams;
	float rectH = this->Height() / (float)activeDeviceCount;


	static C_Frame *prevFrame[NVAPI_MAX_VIO_DEVICES];
	static GLuint numDroppedFrames[NVAPI_MAX_VIO_DEVICES];
	GLfloat left = 0, top = 0;
	int videoWidth = DvpWidth();
	int videoHeight = DvpHeight();

	NVVIOSIGNALFORMAT signalFormat = DvpSignalFormat();
	static int count = 0;
	static float gpuDrawTime;
	static float cpuDrawTime;

	DvpBeginTimeQuery();
	
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
		C_Frame *frame = DvpUpdateFrame(i);

		prevFrame[i] = DvpPreviousFrame(i);
		if (frame == nullptr)
		{
			draw_time_update = false;
		}

		if (prevFrame[i] == nullptr)
			continue;
		else
		{
			numDroppedFrames[i] = DvpNumDroppedFrames(i);
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
		int numStreams = DvpStreamsPerFrame(i);


#ifdef USE_ALL_STREAMS
		numStreams = NUM_VIDEO_STREAMS;
#endif		

		for (int j = 0; j < numStreams; j++)
		{
			assert(DvpBlitTexture(DvpDisplayTextureId(i, j), i, j));
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

			glBindTexture(GL_TEXTURE_RECTANGLE_NV, DvpDisplayTextureId(i, j));

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
		if (showStatistics)
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			//gluOrtho2D(0.0, rectW, 0.0, rectH );
			gluOrtho2D(0.0, this->Width(), 0.0, this->Height());
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glViewport(0, 0, this->Width(), this->Height());

			// Draw video signal		
			sprintf(buf, "Card:%d", DvpDeviceId(i));

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
	DvpEndTimeQuery();


	//::SwapBuffers(SdiGetDC());


	if (count % 15 == 0 && draw_time_update)
	{
		gpuDrawTime = DvpGpuTimeElapsed() * 0.000000001;
		cpuDrawTime = DvpCpuTimeElapsed() * 0.000000001;
		//gpuDrawTime = DvpGpuTimeElapsedMs();
		//cpuDrawTime = DvpCpuTimeElapsedMs();
		sprintf(buf, "MultiCaptureAsync | Draw Time %.5f %.5f | Capture Time %.5f | Upload Time %.5f | Download Time %.5f ", gpuDrawTime, cpuDrawTime, captureElapsedTime, totalUploadTime, totalDownloadTime);
		SetWindowText(this->hWnd, buf);
	}

	count++;
}



void SdiInAsyncWindow::OnKeyEvent(const KeyEvent* pEvent)
{
	if(!pEvent->PressedDown)
		return;

	if(pEvent->Key==KEY_ESCAPE)
	{
		this->Close();
	}

	if (pEvent->Key == KEY_KEY_T)
	{
		showStatistics = !showStatistics;
	}
}











