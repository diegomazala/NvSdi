

#include "SdiOutWindow.h"
#include "GLNvSdi.h"
#include "glExtensions.h"
#include "GLTeapot.h"
#include "GLFont.h"
#include <math.h>			// Header File For Windows Math Library

static float gAngle = 0.0;

GLTeapot gTeapot;
GLFont gFont;






SdiOutWindow::SdiOutWindow():GLWindow(),  presenting(true)
{	
}

SdiOutWindow::~SdiOutWindow()
{
	gFont.Destroy();
}


bool SdiOutWindow::SetupSdi()
{
	if (!SdiOutputInitialize())
	{
		std::cerr << "Error: Could not initialize output sdi" << std::endl;
		return false;
	}

	SdiOutputSetGlobalOptions();
	SdiOutputSetVideoFormat(HD_1080I_59_94, COMP_SYNC, 3.5f, 788, 513, false, 2);

	if (!SdiOutputSetupDevices())
	{
		std::cerr << "Error: Could not setup sdi devices for output" << std::endl;
		SdiOutputCleanupDevices();
		SdiOutputUninitialize();
		return false;
	}

	this->MakeCurrent();

	if (!SdiOutputSetupContextGL(this->GetDC(), this->GetGLRC()) || !SdiOutputSetupGL())
	{
		std::cerr << "Error: Could not setup opengl for sdi output" << std::endl;
		SdiOutputCleanupGL();
		SdiOutputCleanupDevices();
		SdiOutputUninitialize();
		return false;
	}
	else
	{
		SdiOutputCreateTextures();
		SdiOutputInitializeFbo();
	}


	if (!SdiOutputBindVideo() || !SdiOutputStart())
	{
		std::cerr << "Error: Could not start opengl sdi output" << std::endl;
		CleanupSdi();
		return false;
	}

	CHECK_OGL_ERROR;

	return true;
}




void SdiOutWindow::Draw()
{	
	glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, Width(), Height());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, float(Width())/float(Height()), 1, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 5, 7, 0, 1, 0, 0, 1, 0);
	glRotatef(gAngle++, 0, 1, 0);

	gTeapot.Render();
}




void SdiOutWindow::Render()
{
	if (presenting)
	{
		SdiOutputBeginRender();
		{
			this->Draw();

			// 2D Projection
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluOrtho2D( 0.0, 1.0, 0.0, 1.0 ); 
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			gFont.Print(0.01f, 0.99f, "SDI Text ");				// Print GL Text To The Screen
		}
		SdiOutputEndRender();

		SdiOutputGetTexture()->Plot(this->Width(), this->Height(), SdiOutputWidth(), SdiOutputHeight());
	}
	else
	{
		Draw();
	}
}



void SdiOutWindow::OnKeyEvent(const KeyEvent* pEvent)
{
	if(!pEvent->PressedDown)
		return;

	if(pEvent->Key==KEY_ESCAPE)
	{
		this->Close();
	}
	else if(pEvent->Key==KEY_F5)
	{
		this->SetupSdi();
	}
	else if(pEvent->Key==KEY_F6)
	{
		this->CleanupSdi();
	}
	else if(pEvent->Key==KEY_RETURN)
	{
		presenting = !presenting;
	}
}



bool SdiOutWindow::InitGL()
{		
	if(!loadSwapIntervalExtension() || !loadCopyImageExtension())
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

	glClearColor( 0.0, 0.0, 0.0, 0.0); 
	glClearDepth( 1.0 ); 

	glEnable(GL_DEPTH_TEST); 

	// Initialize lighting for render to texture
	GLfloat spot_ambient[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat spot_position[] = {0.0, 3.0, 3.0, 0.0};
	GLfloat local_view[] = {0.0};
	GLfloat ambient[] = {0.01175, 0.01175, 0.1745};
	GLfloat diffuse[] = {0.04136, 0.04136, 0.61424};
	GLfloat specular[] = {0.626959, 0.626959, 0.727811};

	glLightfv(GL_LIGHT0, GL_POSITION, spot_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, spot_ambient);
	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);

	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);

	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, 0.6*128.0);

	//glDisable(GL_LIGHTING);

	glColor3f(0.0, 0.0, 1.0);

	glClearColor(0.0, 0.0, 0.0, 1.0);

	gTeapot.Create(1, 16);

	gFont.Create(-24);

	return true;
}


void SdiOutWindow::CleanupSdi()
{
	SdiOutputStop();
	SdiOutputUnbindVideo();
	SdiOutputCleanupGL();
	SdiOutputCleanupDevices();
	SdiOutputUninitialize();
}
