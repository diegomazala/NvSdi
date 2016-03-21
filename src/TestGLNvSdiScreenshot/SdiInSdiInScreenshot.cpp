

#include "SdiInScreenshot.h"
#include "nvSDIutil.h"
#include "glExtensions.h"
#include "GLNvSdi.h"
#include "GLFont.h"
#include "Image.h"

#include <cstdio>
#include <ctime>

GLFont gFont;

#define BLACK 0, 0, 0
#define WHITE 1, 1, 1
#define GRAY 0.3f, 0.3f, 0.3f

bool sdi_ok = false;


static std::string GetTimeStr()
{
	std::time_t rawtime;
    std::tm* timeinfo;
    char buffer [80];

    std::time(&rawtime);
    timeinfo = std::localtime(&rawtime);

    std::strftime(buffer,80,"%Y-%m-%d-%H-%M-%S",timeinfo);
    std::puts(buffer);

	return buffer;
}


static void SaveTexture(gl::Texture2D& tex, std::string fileName)
{
	tex.Enable();
	tex.Bind();
	const int w = tex.GetWidth();
	const int h = tex.GetHeight();
	const int ch = 4;

	unsigned char* pImageData = new unsigned char[ch*w*h];
	tex.GetData(pImageData, GL_BGRA);

	dm::Image::Save(fileName.c_str(), pImageData, w, h, ch);

	delete [] pImageData;
}


SdiInScreenshot::SdiInScreenshot() : GLWindow(), mVideoIndex(0)
{
}

SdiInScreenshot::~SdiInScreenshot()
{


}


bool SdiInScreenshot::InitGL()
{
	this->MakeCurrent();
	this->VSync(0);


	glClearColor( 0.0, 0.0, 0.0, 0.0); 
	glClearDepth( 1.0 ); 

	glDisable(GL_DEPTH_TEST); 

	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);


	if(!loadCaptureVideoExtension() || !loadBufferObjectExtension() )
	{
		printf("Could not load the required OpenGL extensions\n");
		return false;
	}

	if(!loadPresentVideoExtension() ||  !loadFramebufferObjectExtension())
	{
		MessageBox(NULL, "Couldn't load required OpenGL extensions.", "Error", MB_OK);
		return false;
	}

	gFont.Create(-12, "Arial");

	return true;
}



void SdiInScreenshot::Render()
{
	if (sdi_ok)
	{
		int dropped_frames = 0;
		if (SdiInputCaptureVideo() != GL_FAILURE_NV)
			this->DisplayVideo();
	}
}


void SdiInScreenshot::DisplayVideo()
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

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	SdiInputGetTexture(mVideoIndex)->Draw(-1, -1, 1, 1, 0, 0, 1, 1);

	gFont.Plot(-0.9f, 0.90f, "[Escape] Quit Application");
	gFont.Plot(-0.9f, 0.85f, "['B'] Save Screenshot BMP");
	gFont.Plot(-0.9f, 0.80f, "['P'] Save Screenshot PNG");
	gFont.Plot(-0.9f, 0.75f, "['1-4'] Set Video Input");
}





void SdiInScreenshot::OnKeyEvent(const KeyEvent* pEvent)
{
	if(!pEvent->PressedDown)
		return;

	switch(pEvent->Key)
	{
		case KEY_ESCAPE:
		{
			sdi_ok = false;
			this->Close();
			break;
		}

		case KEY_KEY_B:
		{
			std::string file_name = GetTimeStr() + ".bmp";
			SaveTexture(*SdiInputGetTexture(mVideoIndex), file_name); 
			break;
		}

		case KEY_KEY_P:
		{
			std::string file_name = GetTimeStr() + ".png";
			SaveTexture(*SdiInputGetTexture(mVideoIndex), file_name); 
			break;
		}

		case KEY_KEY_1:
		case KEY_NUMPAD1:
		{
			mVideoIndex = 0;
			glClear(GL_COLOR_BUFFER_BIT);
			break;
		}

		case KEY_KEY_2:
		case KEY_NUMPAD2:
		{
			glClear(GL_COLOR_BUFFER_BIT);
			mVideoIndex = 1;
			break;
		}

		case KEY_KEY_3:
		case KEY_NUMPAD3:
		{
			glClear(GL_COLOR_BUFFER_BIT);
			mVideoIndex = 2;
			break;
		}

		case KEY_KEY_4:
		case KEY_NUMPAD4:
		{
			glClear(GL_COLOR_BUFFER_BIT);
			mVideoIndex = 3;
			break;
		}
	}

}





