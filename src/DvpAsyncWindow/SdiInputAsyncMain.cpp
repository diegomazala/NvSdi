#include "GLNvDvp.h"
#include "DvpAsyncWindow.h"




#include <time.h>
time_t start,stop;


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	//
	// Get the pointer to the funtion which manager the render events
	// This is an attempt to reproduce the same mechanism used by Unity
	//
	UnityRenderingEvent(*DvpRenderEventFunc)(void);
	DvpRenderEventFunc = &GetGLNvDvpRenderEventFunc;


	//
	// Verify if sdi is available
	//
	DvpRenderEventFunc()(static_cast<int>(DvpRenderEvent::CheckAvalability));
	{
		if (!DvpInputIsAvailable())
		{
			std::cerr << "Error: Sdi input is not availabl. Abort." << std::endl;
			return EXIT_FAILURE;
		}
		if (!DvpOutputIsAvailable())
		{
			std::cerr << "Error: Sdi output is not availabl. Abort." << std::endl;
			return EXIT_FAILURE;
		}
	}

	DvpOutputDisable();

	//
	// Setup window application
	//
	WinApp app;
	DvpAsyncWindow sdiWindow;
	{
		CreationParameters creationParams;
		creationParams.Title = "Sdi Input Window";
		creationParams.WindowSize.Width = 960;
		creationParams.WindowSize.Height = 540;
		creationParams.ColorDepthBits = 32;
		creationParams.DepthBufferBits = 24;

		if (!sdiWindow.Create(creationParams, &app))        // Create Our Window
		{
			std::cerr << "ERROR: Could not create the window application. Abort. " << std::endl;
			return EXIT_FAILURE;							// Quit If Window Was Not Created
		}
	}


	//
	// Initialize sdi
	//
	DvpRenderEventFunc()(static_cast<int>(DvpRenderEvent::Initialize));


	//allocate the textures for display
	if (!DvpCreateDisplayTextures(DvpInputWidth(), DvpInputHeight()))
		return false;


	DvpRenderEventFunc()(static_cast<int>(DvpRenderEvent::Setup));

	//if (!DvpIsOk())
	//	return EXIT_FAILURE;

	app.InitSetup();
	while (app.ProcessMainLoop())
	{
		DvpRenderEventFunc()(static_cast<int>(DvpRenderEvent::Update));
	}

	DvpRenderEventFunc()(static_cast<int>(DvpRenderEvent::Cleanup));
	DvpRenderEventFunc()(static_cast<int>(DvpRenderEvent::Uninitialize));

	return EXIT_SUCCESS;
}

