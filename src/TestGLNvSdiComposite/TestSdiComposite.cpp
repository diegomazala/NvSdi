#include "GLNvSdi.h"
#include "SdiWindow.h"


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




int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	// Debug console.
#ifdef _DEBUG
	//SetupConsole();
	SdiSetupLogConsole();
#endif
	SdiSetupLogConsole();

	bool captureFields = true;

	SdiWindow passthru;


	if (!SdiInputInitialize() || !SdiOutputInitialize())
		return EXIT_FAILURE;
	

	

	const int ringBufferSizeInFrames = 2;
	SdiInputSetGlobalOptions(ringBufferSizeInFrames);
	SdiOutputSetGlobalOptions();
	
	//SdiOutputSetVideoFormat(HD_1080I_59_94, COMP_SYNC, 788, 513, false, 2);
	SdiOutputSetVideoFormat(HD_1080I_59_94, COMP_SYNC, 0, 0, false, 2);
	//SdiOutputSetVideoFormat(HD_1080I_59_94, NONE, 0, 0, false, 2);
	//SdiOutputSetVideoFormat(SD_487I_59_94, COMP_SYNC, 788, 513, false, 2);

	if (!SdiInputSetupDevices())
		return EXIT_FAILURE;


	
	// Output video format matches input video format.
	SdiGlobalOptions().videoFormat = SdiInput()->GetSignalFormat();
	

	if (!SdiOutputSetupDevices())
	{
		std::cerr << "Error: Could not setup sdi devices for output" << std::endl;
		return EXIT_FAILURE;
	}

	int window_w = SdiInputWidth(), 
		window_h = SdiInputHeight();


	WinApp lApp;
	CreationParameters lCreationParams;
	lCreationParams.Title = "Sdi Window";
	lCreationParams.WindowSize.Width  = window_w / 2;
	lCreationParams.WindowSize.Height = window_h / 2;


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
	if (!SdiInputSetupGL())
	{
		std::cerr << "ERROR: Cannot setup opengl for input SDI. Abort. " << std::endl;
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
		SdiOutputSetTexture(0, SdiInputGetTextureId(0));
		SdiOutputSetTexture(1, SdiInputGetTextureId(1));
	}
	else
	{
		SdiOutputSetTexture(0, SdiInputGetTextureId(0));
		SdiOutputSetTexture(1, SdiInputGetTextureId(0));
	}


	passthru.InitFbo();


	SdiAncSetupInput();
	SdiAncSetupOutput();


	if (!SdiInputStart())
	{
		std::cerr << "Error: Could not start video capture." << std::endl;
		return EXIT_FAILURE;
	}

	if (!SdiOutputStart())
	{
		std::cerr << "Error: Could not start to present video." << std::endl;
		return EXIT_FAILURE;
	}

	lApp.InitSetup();
	while(lApp.ProcessMainLoop())
	{
		if (SdiInputCaptureVideo() != GL_FAILURE_NV)
		{
			//SdiAncCapture();
			passthru.DisplayVideo(SdiInputWidth(), SdiInputHeight());
			//SdiAncPresent();
			SdiOutputPresentFrame();
		}
	}


	SdiInputStop();
	
	SdiInputUnbindVideoTextureFrame();
	
	SdiInputCleanupGL();
	SdiOutputCleanupGL();

	SdiAncCleanupInput();
	SdiAncCleanupOutput();

	SdiOutputCleanupDevices();
	SdiInputCleanupDevices();
	
	passthru.Destroy();

	return EXIT_SUCCESS;
}

