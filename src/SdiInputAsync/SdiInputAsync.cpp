#if 1
#include "GLNvSdi.h"
#include "SdiInAsyncWindow.h"




#include <time.h>
time_t start,stop;


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	SdiInAsyncWindow multiCaptureAsync;
	if (!multiCaptureAsync.IsSdiAvailable())
		return EXIT_FAILURE;

	WinApp lApp;
	CreationParameters lCreationParams;
	lCreationParams.Title = "Sdi Input Window";
	lCreationParams.WindowSize.Width = 960;
	lCreationParams.WindowSize.Height = 540;
	lCreationParams.ColorDepthBits = 32;
	lCreationParams.DepthBufferBits = 24;


	if (!multiCaptureAsync.Create(lCreationParams, &lApp))        // Create Our Window
	{
		std::cerr << "ERROR: Cannot create the window application. Abort. " << std::endl;
		return EXIT_FAILURE;							// Quit If Window Was Not Created
	}


	if (multiCaptureAsync.SetupSDIPipeline() == E_FAIL)
		return FALSE;

	multiCaptureAsync.SetupGLPipe();

	if (multiCaptureAsync.StartSDIPipeline() == E_FAIL)
		return FALSE;


	lApp.InitSetup();
	while (lApp.ProcessMainLoop())
	{
		multiCaptureAsync.DisplayVideo();
	}

	std::cout << "exit" << std::endl;

	return EXIT_SUCCESS;
}

#endif
