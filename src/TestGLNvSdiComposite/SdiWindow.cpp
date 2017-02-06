

#include "SdiWindow.h"
#include "nvSDIutil.h"
#include "glExtensions.h"
#include "GLNvSdi.h"
#include "GLFont.h"
#include <ctime>

GLFont gFont;

#define BLACK 0, 0, 0
#define WHITE 1, 1, 1
#define GRAY 0.3f, 0.3f, 0.3f

static int timecode[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

SdiWindow::SdiWindow() : GLWindow(), m_ProccessingEnabled(true), DroppedFrames(0), m_CurrentInputIndex(0)
{
}

SdiWindow::~SdiWindow()
{


}


bool SdiWindow::InitGL()
{
	this->MakeCurrent();
	this->VSync(0);


	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);

	glDisable(GL_DEPTH_TEST);

	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);

	if (!loadSwapIntervalExtension())
	{
		MessageBox(NULL, "Couldn't load required OpenGL extensions.", "Error", MB_OK);
		return false;
	}

	// Don't sync graphics drawing to the vblank.m  This permits
	// drawing to be synchronized to the SDI scanout.  Otherwise,
	// duplicate frames on the SDI output will result as drawing
	// of the next frame is blocked until the SwapBuffer call
	// returns.
	wglSwapIntervalEXT(0);



	if (!loadCaptureVideoExtension() || !loadBufferObjectExtension())
	{
		printf("Could not load the required OpenGL extensions\n");
		return false;
	}

	if (!loadPresentVideoExtension() || !loadFramebufferObjectExtension())
	{
		MessageBox(NULL, "Couldn't load required OpenGL extensions.", "Error", MB_OK);
		return false;
	}

	gFont.Create(-48, "Arial");

	return true;
}


bool SdiWindow::InitFbo()
{
	mOutputTex.Create();
	mOutputTex.Bind();
	mOutputTex.SetMipmap(false);
	mOutputTex.SetParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	mOutputTex.SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	mOutputTex.SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP);
	mOutputTex.SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP);
	mOutputTex.BuildNull(SdiInputWidth(), SdiInputHeight());
	fbo.Initialize(SdiInputWidth(), SdiInputHeight(), 8, 1, GL_TRUE, GL_TRUE, &mOutputTex);

	return true;
}






void SdiWindow::RenderToSdi(int vid_w, int vid_h)
{
	for (int f = 0; f < 2; ++f)
	{
		SdiOutputBeginRender(0, f);
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//SdiInputGetTexture(m_CurrentInputIndex)->Enable();
			//SdiInputGetTexture(m_CurrentInputIndex)->Bind();
			//SdiInputGetTexture(m_CurrentInputIndex)->Plot(vid_w, vid_h, vid_w, vid_h);

			glColor3f(WHITE);
			gFont.Plot(-0.95f, -0.70f, "GPU Time: %f. GVI Time: %f", SdiInputGpuTime(), SdiInputGviTime());
			gFont.Plot(-0.95f, -0.80f, "Drops In(%d) Out(%d) ", SdiInputDroppedFramesCount(), SdiOutputDuplicatedFramesCount());
			gFont.Plot(-0.95f, -0.90f, "%d%d:%d%d:%d%d:%d%d",
				timecode[0], timecode[1], timecode[2], timecode[3],
				timecode[4], timecode[5], timecode[6], timecode[7]);

			//SdiInputGetTexture(m_CurrentInputIndex)->Unbind();
			//SdiInputGetTexture(m_CurrentInputIndex)->Disable();
		}
		SdiOutputEndRender(0, f);
	}
}

void SdiWindow::DisplayVideo(int vid_w, int vid_h)
{
	if (m_ProccessingEnabled)
	{

		fbo.BeginRender(0);

		glColor3f(GRAY);

		// Enable texture
		SdiInputGetTexture(m_CurrentInputIndex)->Enable();

		// Bind texture object for first video stream
		SdiInputGetTexture(m_CurrentInputIndex)->Bind();

		SdiInputGetTexture(m_CurrentInputIndex)->Plot(SdiOutputWidth(), SdiOutputHeight(), vid_w, vid_h);

		assert(glGetError() == GL_NO_ERROR);

		// Disable texture
		SdiInputGetTexture(m_CurrentInputIndex)->Unbind();
		SdiInputGetTexture(m_CurrentInputIndex)->Disable();


		glColor3f(WHITE);
		gFont.Plot(-0.95f, 0.90f, "GPU Time: %f. GVI Time: %f", SdiInputGpuTime(), SdiInputGviTime());
		gFont.Plot(-0.95f, 0.80f, "Drops In(%d) Out(%d) ", SdiInputDroppedFramesCount(), SdiOutputDuplicatedFramesCount());



		if (SdiAncGetTimeCode(timecode, m_CurrentInputIndex))
		{
			gFont.Plot(-0.95f, 0.70f, "%d%d:%d%d:%d%d:%d%d",
				timecode[0], timecode[1], timecode[2], timecode[3],
				timecode[4], timecode[5], timecode[6], timecode[7]);
		}
		else
		{
			gFont.Plot(-0.95f, 0.70f, "No TimeCode");
		}

		fbo.EndRender(0);

		//
		// Draw texture contents to graphics window.
		//

		// Reset view parameters
		glViewport(0, 0, Width(), Height());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// Set draw color
		glColor3f(1.0f, 1.0f, 1.0f);


		// Enable texture
		mOutputTex.Enable();

		// Bind texture object for first video stream
		mOutputTex.Bind();

		mOutputTex.Plot(Width(), Height(), vid_w, vid_h);

		assert(glGetError() == GL_NO_ERROR);

		// Disable texture
		mOutputTex.Unbind();
		mOutputTex.Disable();
	}
	else
	{
		//
		// Draw texture contents to graphics window.
		//

		// Reset view parameters
		glViewport(0, 0, Width(), Height());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// Set draw color
		glColor3f(1.0f, 1.0f, 1.0f);


		// Enable texture
		SdiInputGetTexture(m_CurrentInputIndex)->Enable();

		// Bind texture object for first video stream
		SdiInputGetTexture(m_CurrentInputIndex)->Bind();

		SdiInputGetTexture(m_CurrentInputIndex)->Plot(Width(), Height(), vid_w, vid_h);

		assert(glGetError() == GL_NO_ERROR);

		// Disable texture
		SdiInputGetTexture(m_CurrentInputIndex)->Unbind();
		SdiInputGetTexture(m_CurrentInputIndex)->Disable();
	}
}

//
// Calculate the graphics window size.
//
void SdiWindow::CalcWindowSize(int vid_w, int vid_h, int stream_count, NVVIOSIGNALFORMAT signal_format, int& win_w, int& win_h)
{
	switch (signal_format)
	{
	case NVVIOSIGNALFORMAT_487I_59_94_SMPTE259_NTSC:
	case NVVIOSIGNALFORMAT_576I_50_00_SMPTE259_PAL:
		if (stream_count == 1) {
			win_w = vid_w; win_h = vid_h;
		}
		else if (stream_count == 2) {
			win_w = vid_w; win_h = vid_h << 1;
		}
		else {
			win_w = vid_w << 1; win_h = vid_h << 1;
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
		if (stream_count == 1) {
			win_w = vid_w >> 2; win_h = vid_h >> 2;
		}
		else if (stream_count == 2) {
			win_w = vid_w >> 2; win_h = vid_h >> 1;
		}
		else {
			win_w = vid_w >> 1; win_h = vid_h >> 1;
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
		if (stream_count == 1) {
			win_w = vid_w >> 2; win_h = vid_h >> 2;
		}
		else if (stream_count == 2) {
			win_w = vid_w >> 2; win_h = vid_h >> 1;
		}
		else {
			win_w = vid_w >> 1; win_h = vid_h >> 1;
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
		if (stream_count == 1) {
			win_w = vid_w >> 2; win_h = vid_h >> 2;
		}
		else if (stream_count == 2) {
			win_w = vid_w >> 2; win_h = vid_h >> 1;
		}
		else {
			win_w = vid_w >> 1; win_h = vid_h >> 1;
		}
		break;

	default:
		win_w = 500;
		win_h = 500;
	}
}



void SdiWindow::Render()
{
}



void SdiWindow::OnKeyEvent(const KeyEvent* pEvent)
{
	if (!pEvent->PressedDown)
		return;

	if (pEvent->Key == KEY_ESCAPE)
	{
		this->Close();
	}
	else if (pEvent->Key == KEY_SPACE)
	{
		m_ProccessingEnabled = !m_ProccessingEnabled;
	}
	else if (pEvent->Key == KEY_UP)
	{
		if (m_CurrentInputIndex < MAX_VIDEO_STREAMS - 1)
			m_CurrentInputIndex++;
		else
			m_CurrentInputIndex = 0;
	}
	else if (pEvent->Key == KEY_DOWN)
	{
		if (m_CurrentInputIndex > 0)
			m_CurrentInputIndex--;
		else
			m_CurrentInputIndex = MAX_VIDEO_STREAMS - 1;
	}
	else if (pEvent->Key == KEY_NUMPAD0)
	{
		m_CurrentInputIndex = 0;
	}
	else if (pEvent->Key == KEY_NUMPAD1)
	{
		m_CurrentInputIndex = 1;
	}
	else if (pEvent->Key == KEY_NUMPAD2)
	{
		m_CurrentInputIndex = 2;
	}
	else if (pEvent->Key == KEY_NUMPAD3)
	{
		m_CurrentInputIndex = 3;
	}
	else if (pEvent->Key == KEY_KEY_R)
	{
		SdiInputResetDroppedFramesCount();
		SdiOutputResetDuplicatedFramesCount();
	}
}







