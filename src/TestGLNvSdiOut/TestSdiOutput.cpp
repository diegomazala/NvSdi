
#include "GLNvSdi.h"
#include "SdiOutWindow.h"
#include "WinAppGL/WinApp.h"




int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	WinApp		lApp;
	CreationParameters lCreationParams;
	lCreationParams.Title = "Sdi Out Window";
	lCreationParams.WindowSize.Width  = 960;
	lCreationParams.WindowSize.Height = 540;

	SdiOutWindow lWindow;
		
	try
	{
		if(!lWindow.Create(lCreationParams, &lApp))        // Create Our Window
		{
			std::cerr << "ERROR: Cannot create the window application. Abort. " << std::endl;
			return EXIT_FAILURE;							// Quit If Window Was Not Created
		}

		if (lWindow.SetupSdi())
		{
			lApp.Run();
			lWindow.CleanupSdi();
		}
		
	}
	catch(const std::exception& e)
	{
		MessageBox(NULL, e.what(), "GLNvSdi Exception", MB_ICONERROR | MB_OK);
	}
	

	lWindow.Destroy();

	return EXIT_SUCCESS;
}


