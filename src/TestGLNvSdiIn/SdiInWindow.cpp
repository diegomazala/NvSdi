

#include "SdiInWindow.h"
#include "nvSDIutil.h"
#include "glExtensions.h"
#include "GLNvSdi.h"
#include "GLFont.h"

GLFont gFont;

#define BLACK 0, 0, 0
#define WHITE 1, 1, 1
#define GRAY 0.3f, 0.3f, 0.3f


SdiInWindow::SdiInWindow() : GLWindow(), DroppedFrames(0)
{
}

SdiInWindow::~SdiInWindow()
{


}


bool SdiInWindow::InitGL()
{
	this->MakeCurrent();
	this->VSync(0);


	glClearColor( 0.0, 0.0, 0.0, 0.0); 
	glClearDepth( 1.0 ); 

	glDisable(GL_DEPTH_TEST); 

	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);


	gFont.Create(-18, "Arial");

	return true;
}



void SdiInWindow::Render()
{
	
}


void SdiInWindow::DisplayVideo(int vid_w, int vid_h)
{	
	int timecode[8];
	int videoIndex;
	
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

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	

	
	glColor3f(GRAY);
	videoIndex = 0;
	SdiInputGetTexture(videoIndex)->Draw(-1, 0, 0, 1, 0, 0, 1, 1);
	glColor3f(WHITE);
	gFont.Plot(-0.9f, 0.8f, "Input %d", videoIndex);
	if (SdiAncGetTimeCode(timecode, videoIndex))
	{
		gFont.Plot(-0.9f, 0.7f, "%d%d:%d%d:%d%d:%d%d", 
			timecode[0], timecode[1], timecode[2], timecode[3],
			timecode[4], timecode[5], timecode[6], timecode[7]);
	}


	glColor3f(GRAY);
	videoIndex = 1;
	SdiInputGetTexture(videoIndex)->Draw(0, 0, 1, 1, 0, 0, 1, 1);
	glColor3f(WHITE);
	gFont.Plot(0.1f, 0.8f, "Input %d", videoIndex);
	if (SdiAncGetTimeCode(timecode, videoIndex))
	{
		gFont.Plot(0.1f, 0.7f, "%d%d:%d%d:%d%d:%d%d", 
			timecode[0], timecode[1], timecode[2], timecode[3],
			timecode[4], timecode[5], timecode[6], timecode[7]);
	}
	

	glColor3f(GRAY);
	videoIndex = 2;
	SdiInputGetTexture(videoIndex)->Draw(-1, -1, 0, 0, 0, 0, 1, 1);
	glColor3f(WHITE);
	gFont.Plot(-0.9f, -0.2f, "Input %d", videoIndex);
	if (SdiAncGetTimeCode(timecode, videoIndex))
	{
		gFont.Plot(-0.9f, -0.3f, "%d%d:%d%d:%d%d:%d%d", 
			timecode[0], timecode[1], timecode[2], timecode[3],
			timecode[4], timecode[5], timecode[6], timecode[7]);
	}

	
	glColor3f(GRAY);
	videoIndex = 3;
 	SdiInputGetTexture(videoIndex)->Draw(0, -1, 1, 0, 0, 0, 1, 1);
	glColor3f(WHITE);
	gFont.Plot(0.1f, -0.2f, "Input %d", videoIndex);
	if (SdiAncGetTimeCode(timecode, videoIndex))
	{
		gFont.Plot(0.1f, -0.3f, "%d%d:%d%d:%d%d:%d%d", 
			timecode[0], timecode[1], timecode[2], timecode[3],
			timecode[4], timecode[5], timecode[6], timecode[7]);
	}

	gFont.Plot(-0.9f, 0.5f, "Dropped %d", DroppedFrames);
	gFont.Plot(-0.9f, 0.42f, "GPU Time: %f", SdiInputGpuTime());
	gFont.Plot(-0.9f, 0.34f, "GVI Time: %f", SdiInputGviTime());

	// Draw border
	glColor3f(BLACK);
	glLineWidth(10);
	glBegin(GL_LINES);
	{
		glVertex2f(0, -1);
		glVertex2f(0,  1);
		glVertex2f(-1, 0);
		glVertex2f( 1, 0);
	}
	glEnd();

}



//
// Calculate the graphics window size.
//
void SdiInWindow::CalcWindowSize(int vid_w, int vid_h, int stream_count, NVVIOSIGNALFORMAT signal_format, int& win_w, int& win_h)
{  
	switch(signal_format) 
	{
	case NVVIOSIGNALFORMAT_487I_59_94_SMPTE259_NTSC:
	case NVVIOSIGNALFORMAT_576I_50_00_SMPTE259_PAL:
		if (stream_count == 1) {
			win_w = vid_w; win_h = vid_h;
		} else if (stream_count == 2) {
			win_w = vid_w; win_h = vid_h<<1;
		} else {
			win_w = vid_w<<1; win_h = vid_h<<1;
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
			win_w = vid_w>>2; win_h = vid_h>>2;
		} else if (stream_count == 2) {
			win_w = vid_w>>2; win_h = vid_h>>1;
		} else {
			win_w = vid_w>>1; win_h = vid_h>>1;
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
			win_w = vid_w>>2; win_h = vid_h>>2;
		} else if (stream_count == 2) {
			win_w = vid_w>>2; win_h = vid_h>>1;
		} else {
			win_w = vid_w>>1; win_h = vid_h>>1;
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
			win_w = vid_w>>2; win_h = vid_h>>2;
		} else if (stream_count == 2) {
			win_w = vid_w>>2; win_h = vid_h>>1;
		} else {
			win_w = vid_w>>1; win_h = vid_h>>1;
		}
		break;

	default:
		win_w = 500;
		win_h = 500;
	}
}




void SdiInWindow::OnKeyEvent(const KeyEvent* pEvent)
{
	if(!pEvent->PressedDown)
		return;

	if(pEvent->Key==KEY_ESCAPE)
	{
		this->Close();
	}
}





