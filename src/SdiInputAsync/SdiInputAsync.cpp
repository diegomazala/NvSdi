#if 1
#include "GLNvSdi.h"
#include "SdiInAsyncWindow.h"




#include <time.h>
time_t start,stop;


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	SdiInAsyncWindow sdiWindow;
	if (!sdiWindow.IsSdiAvailable())
		return EXIT_FAILURE;

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


	if (sdiWindow.SetupSDIPipeline() == E_FAIL)
		return FALSE;

	sdiWindow.SetupGLPipe();

	if (sdiWindow.StartSDIPipeline() == E_FAIL)
		return FALSE;


	lApp.InitSetup();
	while (lApp.ProcessMainLoop())
	{
		sdiWindow.DisplayVideo();
	}

	return EXIT_SUCCESS;
}

#endif
