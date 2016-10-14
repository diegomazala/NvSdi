#include "GLNvSdi.h"
#include "SdiIOAsyncWindow.h"




#include <time.h>
time_t start,stop;


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	bool captureFields = true;
	SdiIOAsyncWindow sdiWindow;

	//
	// Get the pointer to the funtion which manager the render events
	// This is an attempt to reproduce the same mechanism used by Unity
	//
	UnityRenderingEvent(*DvpRenderEventFunc)(void);
	DvpRenderEventFunc = &GetDvpRenderEventFunc;

	UnityRenderingEvent(*SdiOutputRenderEventFunc)(void);
	SdiOutputRenderEventFunc = &GetSdiOutputRenderEventFunc;
	


	//
	// Check if sdi is available
	//
	{
		HWND hWnd;
		HGLRC hGLRC;
		if (CreateDummyGLWindow(&hWnd, &hGLRC) != false)
		{
			DvpRenderEventFunc()(SdiRenderEvent::Initialize);

			if (!SdiOutputInitialize())
				return EXIT_FAILURE;

			if (!DvpIsOk())
				return EXIT_FAILURE;

			
			// We can kill the dummy window now
			DestroyGLWindow(&hWnd, &hGLRC);
		}
	}

	WinApp lApp;
	CreationParameters lCreationParams;
	lCreationParams.Title = "Sdi Input Window";
	lCreationParams.WindowSize.Width = 960;
	lCreationParams.WindowSize.Height = 540;
	lCreationParams.ColorDepthBits = 32;
	lCreationParams.DepthBufferBits = 24;

	

	if (!sdiWindow.Create(lCreationParams, &lApp))        // Create Our Window
	{
		std::cerr << "ERROR: Cannot create the window application. Abort. " << std::endl;
		return EXIT_FAILURE;							// Quit If Window Was Not Created
	}

	

	DvpRenderEventFunc()(SdiRenderEvent::PreSetup);
	
	//allocate the textures for display
	if (!DvpCreateDisplayTextures(DvpWidth(), DvpHeight()))
		return false;

	//for (int i = 0; i < DvpActiveDeviceCount(); ++i)
	//{
	//	for (int j = 0; j < DvpStreamsPerFrame(i); ++j)
	//	{
	//		gl::Texture2D* tex = DvpDisplayTexture(i, j);
	//		DvpSetDisplayTexture(tex->Id(), tex->Type(), i, j);
	//	}
	//}

	DvpRenderEventFunc()(SdiRenderEvent::Setup);
	DvpRenderEventFunc()(SdiRenderEvent::StartCapture);

	if (!DvpIsOk())
		return EXIT_FAILURE;


	const int ringBufferSizeInFrames = 2;
	SdiOutputSetGlobalOptions();
	// Output video format matches input video format.
	SdiGlobalOptions().videoFormat = DvpSignalFormat();


	if (!SdiOutputSetupGL())
	{
		std::cerr << "ERROR: Cannot setup opengl for output SDI. Abort. " << std::endl;
		return EXIT_FAILURE;
	}

	if (!SdiOutputBindVideo())
	{
		std::cerr << "ERROR: Cannot bind opengl video for output SDI. Abort. " << std::endl;
		return EXIT_FAILURE;
	}


	if (captureFields)
	{
		SdiOutputSetTexture(0, DvpDisplayTexture(0, 0)->Id());
		SdiOutputSetTexture(1, DvpDisplayTexture(0, 1)->Id());
	}
	else
	{
		SdiOutputSetTexture(0, DvpDisplayTexture(0, 0)->Id());
		SdiOutputSetTexture(1, DvpDisplayTexture(0, 0)->Id());
	}

	SdiAncSetupOutput();

	if (!SdiOutputStart())
	{
		std::cerr << "Error: Could not start to present video." << std::endl;
		return EXIT_FAILURE;
	}

	lApp.InitSetup();
	while (lApp.ProcessMainLoop())
	{
		DvpRenderEventFunc()(SdiRenderEvent::CaptureFrame);
		SdiOutputPresentFrame();
	}

	
	DvpRenderEventFunc()(SdiRenderEvent::StopCapture);
	DvpRenderEventFunc()(SdiRenderEvent::Shutdown);

	SdiOutputCleanupGL();
	SdiAncCleanupOutput();
	SdiOutputCleanupDevices();

	return EXIT_SUCCESS;
}

