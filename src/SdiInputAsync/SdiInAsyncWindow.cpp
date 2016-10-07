

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

	texBlit.CreateVbo();

	return true;
}




void SdiInAsyncWindow::Render()
{
	MakeCurrent();

#if 1
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
		C_Frame *frame = DvpFrame(i);

		if (frame == nullptr)
		{
			draw_time_update = false;
		}

		if (DvpPreviousFrame(i) == nullptr)
			continue;
		else
		{
			numDroppedFrames[i] = DvpNumDroppedFrames(i);
			draw_time_update = true;
		}

		ComputeScaledVideoDimensions(rectW, rectH,
			videoWidth, videoHeight,
			&l_scaledVideoWidth,
			&l_scaledVideoHeight);


		glColor3f(1.0f, 1.0f, 1.0f);

		int numStreams = DvpStreamsPerFrame(i);

		// Draw contents of each video texture
		// Reset view parameters

		DvpDisplayTexture(i, 0)->Enable();

		for (int j = 0; j < numStreams; j++)
		{
			// Set viewport .
			glViewport(0, 0, rectW, rectH);
			
			// Bind texture object video stream i
			DvpDisplayTexture(i, j)->Bind();

			texBlit.BlitDefault(Width(), Height());

			DvpDisplayTexture(i, j)->Unbind();
			assert(glGetError() == GL_NO_ERROR);
		}

		DvpDisplayTexture(i, 0)->Disable();


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

			sprintf(buf, "OpenGL :%s", (char*)glGetString(GL_VERSION));

			len = strlen(buf);
			glListBase(1000);
			glColor3f(1.0f, 1.0f, 0.0f);
			glRasterPos2f(10, rectH*(i + 1) - 20);
			glCallLists((GLsizei)len, GL_UNSIGNED_BYTE, buf);

			sprintf(buf, "Card:%d", DvpDeviceId(i));

			len = strlen(buf);
			glListBase(1000);
			glColor3f(1.0f, 1.0f, 0.0f);
			glRasterPos2f(10, rectH*(i + 1) - 35);
			glCallLists((GLsizei)len, GL_UNSIGNED_BYTE, buf);

			// Draw video signal
			len = strlen(SignalFormatToString(signalFormat).c_str());
			glListBase(1000);
			glColor3f(1.0f, 1.0f, 0.0f);
			glRasterPos2f(10, rectH*(i + 1) - 50);
			glCallLists((GLsizei)len, GL_UNSIGNED_BYTE, SignalFormatToString(signalFormat).c_str());

			// Draw sequence number
			sprintf(buf, "total:%d", frame->sequenceNum);
			len = strlen(buf);
			glListBase(1000);
			glColor3f(1.0f, 1.0f, 0.0f);
			glRasterPos2f(10, rectH*(i + 1) - 65);
			glCallLists((GLsizei)len, GL_UNSIGNED_BYTE, buf);

			// Draw dropped frames number
			sprintf(buf, "dropped at capture:%d", frame->numDroppedFrames);
			len = strlen(buf);
			glListBase(1000);
			glColor3f(1.0f, 1.0f, 0.0f);
			glRasterPos2f(10, rectH*(i + 1) - 80);
			glCallLists((GLsizei)len, GL_UNSIGNED_BYTE, buf);

			// Draw dropped frames number
			sprintf(buf, "dropped at render time:%d", numDroppedFrames[i]);
			len = strlen(buf);
			glListBase(1000);
			glColor3f(1.0f, 1.0f, 0.0f);
			glRasterPos2f(10, rectH*(i + 1) - 95);
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
		sprintf(buf, "MultiCaptureAsync | Draw Time %.5f %.5f | Capture Time %.5f | Upload Time %.5f | Download Time %.5f ", 
			gpuDrawTime, cpuDrawTime, DvpCaptureElapsedTime(), DvpUploadElapsedTime(), DvpDownloadElapsedTime());
		SetWindowText(this->hWnd, buf);
	}

	count++;

#endif

	//glViewport(0, 0, Width(), Height());
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//quad.Render(Width(), Height());
	//assert(glGetError() == GL_NO_ERROR);
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











