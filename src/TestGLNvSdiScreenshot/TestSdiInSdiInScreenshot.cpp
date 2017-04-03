#include "GLNvSdi.h"
#include "SdiInScreenshot.h"



int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	

	bool captureFields = false;
	
	SdiInScreenshot passthru;
	

	HWND hWnd;
	HGLRC hGLRC;
	if (CreateDummyGLWindow(&hWnd, &hGLRC) == false)
		return false;

	if (!SdiInputInitialize())
		return EXIT_FAILURE;

	// We can kill the dummy window now
	if (DestroyGLWindow(&hWnd, &hGLRC) == false)
		return false;

	const int ringBufferSizeInFrames = 2;
	SdiInputSetBufferSize(ringBufferSizeInFrames);

	if (!SdiInputSetupDevices())
		return EXIT_FAILURE;


	int window_w = SdiInputWidth(), 
		window_h = SdiInputHeight();

	WinApp lApp;
	CreationParameters lCreationParams;
	lCreationParams.Title = "Sdi Input Window";
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


	SdiSetExternalDC(passthru.GetDC());
	SdiSetExternalGLRC(passthru.GetGLRC());

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
		if (SdiInputCaptureVideo() != GL_FAILURE_NV)
		{
			SdiAncCapture();
			passthru.DisplayVideo();
		}
		else
		{

		}
	}


	SdiInputStop();

	SdiInputUnbindVideoTextureFrame();

	SdiInputCleanupGL();

	SdiAncCleanupInput();

	SdiInputCleanupDevices();

	passthru.Destroy();

	return EXIT_SUCCESS;
}
