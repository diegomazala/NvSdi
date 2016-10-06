#include "GLNvSdi.h"
#include "SdiInAsyncWindow.h"




#include <time.h>
time_t start,stop;


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	SdiInAsyncWindow sdiWindow;

	//
	// Get the pointer to the funtion which manager the render events
	// This is an attempt to reproduce the same mechanism used by Unity
	//
	UnityRenderingEvent(*DvpRenderEventFunc)(void);
	DvpRenderEventFunc = &GetDvpRenderEventFunc;
	

	//
	// Check if sdi is available
	//
	{
		HWND hWnd;
		HGLRC hGLRC;
		if (CreateDummyGLWindow(&hWnd, &hGLRC) == false)
		{
			DvpRenderEventFunc()(SdiRenderEvent::Initialize);

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
	DvpRenderEventFunc()(SdiRenderEvent::Setup);
	DvpRenderEventFunc()(SdiRenderEvent::StartCapture);

	if (!DvpIsOk())
		return EXIT_FAILURE;


	lApp.InitSetup();
	while (lApp.ProcessMainLoop())
	{
		DvpRenderEventFunc()(SdiRenderEvent::CaptureFrame);
	}

	
	DvpRenderEventFunc()(SdiRenderEvent::StopCapture);
	//DvpRenderEventFunc()(SdiRenderEvent::Shutdown);

	return EXIT_SUCCESS;
}

