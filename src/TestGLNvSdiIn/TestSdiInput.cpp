#include "GLNvSdi.h"
#include "SdiInWindow.h"



int main(int argc, char* argv[])
{
	const int ringBufferSizeInFrames = ((argc > 1) ? atoi(argv[1]) : 2);
	const bool captureFields = (bool)((argc > 2) ? atoi(argv[2]) : true);

	
	
	SdiInWindow passthru;

	HWND hWnd;
	HGLRC hGLRC;
	if(CreateDummyGLWindow(&hWnd,&hGLRC) == false)
		return false;

	if (!SdiInputInitialize())
		return EXIT_FAILURE;

	// We can kill the dummy window now
	if(DestroyGLWindow(&hWnd,&hGLRC) == false)
		return false;

	
	SdiInputSetGlobalOptions(ringBufferSizeInFrames, captureFields);

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

	if (SdiGlobalOptions().captureFields)
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


	if (SdiGlobalOptions().captureFields)
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
		GLenum status = SdiInputCaptureVideo();
		if (status != GL_SUCCESS_NV)
		{
			std::cout << "Capture fail : " << ((status == GL_FAILURE_NV) ? "GL_FAILURE_NV" : "GL_PARTIAL_SUCCESS_NV") << std::endl;
		}


		if (SdiInputDroppedFrames() > 0)
			std::cout << "Frame:   " << SdiInputFrameNumber() << "    Dropped: " << SdiInputDroppedFrames() << "     Total:   " << SdiInputDroppedFramesCount() << std::endl;

	}


	SdiInputStop();

	SdiInputUnbindVideoTextureFrame();

	SdiInputCleanupGL();

	SdiAncCleanupInput();

	SdiInputCleanupDevices();

	passthru.Destroy();

	return EXIT_SUCCESS;
}
