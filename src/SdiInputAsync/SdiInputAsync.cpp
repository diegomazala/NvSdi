#include "GLNvSdi.h"
#include "SdiInAsyncWindow.h"




#include <time.h>
time_t start,stop;


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	SdiInAsyncWindow sdiWindow;

	//if (!sdiWindow.IsSdiAvailable())
	if (!DvpIsAvailable())
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


	if (!DvpSetup())
		return EXIT_FAILURE;

	if (!DvpStart())
		return EXIT_FAILURE;


	lApp.InitSetup();
	while (lApp.ProcessMainLoop())
	{
	}

	return EXIT_SUCCESS;
}

