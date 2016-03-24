#include "GLNvSdi.h"
#include "SdiInWindow.h"


#include <fcntl.h>
#include <io.h>
static void SetupConsole()
{
	int hCrt;
	FILE *hf;
	static int initialized = 0;

	if(initialized == 1) {
		return;
	}

	AllocConsole();

	// Setup stdout
	hCrt = _open_osfhandle( (long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT );
	hf = _fdopen(hCrt, "w" );
	*stdout = *hf;
	setvbuf(stdout, NULL, _IONBF, 0);

	// Setup stderr
	hCrt = _open_osfhandle( (long)GetStdHandle(STD_ERROR_HANDLE), _O_TEXT );
	hf = _fdopen(hCrt, "w" );
	*stderr = *hf;
	setvbuf(stderr, NULL, _IONBF, 0);

	initialized = 1;
}



#include <time.h>
time_t start,stop;



int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{

	// Debug console.
#ifdef _DEBUG
	//SetupConsole();
	SdiSetupLogConsole();
#endif

	SdiSetupLogConsole();

	bool captureFields = true;
	
	SdiInWindow passthru;


	if (!SdiInputInitialize())
		return EXIT_FAILURE;

	const int ringBufferSizeInFrames = 2;
	SdiInputSetGlobalOptions(ringBufferSizeInFrames);

	if (!SdiInputSetupDevices())
		return EXIT_FAILURE;


	int window_w = SdiInputWidth(), 
		window_h = SdiInputHeight();

	WinApp lApp;
	CreationParameters lCreationParams;
	lCreationParams.Title = "Sdi Input Window";
	lCreationParams.WindowSize.Width  = window_w / 2;
	lCreationParams.WindowSize.Height = window_h / 2;
	

	passthru.ShowConsoleWindow(true);

	if(!passthru.Create(lCreationParams, &lApp))        // Create Our Window
	{
		std::cerr << "ERROR: Cannot create the window application. Abort. " << std::endl;
		return EXIT_FAILURE;							// Quit If Window Was Not Created
	}

	if (captureFields)
	{
		if (!SdiInputCreateTextures(MAX_VIDEO_STREAMS * 2, SdiInputWidth(), SdiInputHeight() / 2))
		{
			std::cerr << "ERROR: Could not creat opengl textures for SDI input. Abort. " << std::endl;
			return EXIT_FAILURE;
		}
	}
	else
	{
		if (!SdiInputCreateTextures(MAX_VIDEO_STREAMS, SdiInputWidth(), SdiInputHeight()))
		{
			std::cerr << "ERROR: Could not creat opengl textures for SDI input. Abort. " << std::endl;
			return EXIT_FAILURE;
		}
	}


	SdiSetDC(passthru.GetDC());
	SdiSetGLRC(passthru.GetGLRC());
	SdiMakeCurrent();

	if (!SdiInputSetupGL())
	{
		std::cerr << "ERROR: Cannot setup opengl for SDI. Abort. " << std::endl;
		return EXIT_FAILURE;
	}


	if (captureFields)
	{
		if (!SdiInputBindVideoTextureField())
		{
			std::cerr << "ERROR: Cannot bind textures for SDI. Abort. " << std::endl;
			return EXIT_FAILURE;
		}
	}
	else
	{
		if (!SdiInputBindVideoTextureFrame())
		{
			std::cerr << "ERROR: Cannot bind textures for SDI. Abort. " << std::endl;
			return EXIT_FAILURE;
		}
	}


	if (!SdiAncSetupInput())
	{
		std::cerr << "ERROR: Cannot setup audio capture for SDI. Abort. " << std::endl;
		return EXIT_FAILURE;
	}
	

	if (!SdiInputStart())
	{
		std::cerr << "Error: Could not start video capture." << std::endl;
		return EXIT_FAILURE;
	}


	lApp.InitSetup();
	while(lApp.ProcessMainLoop())
	{
	}


	SdiInputStop();

	SdiInputUnbindVideoTextureFrame();

	SdiInputCleanupGL();

	SdiAncCleanupInput();

	SdiInputCleanupDevices();

	passthru.Destroy();

	return EXIT_SUCCESS;
}