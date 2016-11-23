#include "GLNvSdi.h"
#include "SdiWindow.h"
#include <ctime>
#include <chrono>

int main(int argc, char* argv[])
{
	std::chrono::time_point<std::chrono::system_clock> start_timer, end_timer;
	start_timer = std::chrono::system_clock::now();

	std::time_t timer = std::time(nullptr);

	const int ringBufferSizeInFrames = ((argc > 1) ? atoi(argv[1]) : 2);
	const bool captureFields = (bool)((argc > 2) ? atoi(argv[2]) : true);
	
	
	HWND hWnd;
	HGLRC hGLRC;
	if (CreateDummyGLWindow(&hWnd, &hGLRC) == false)
		return false;


	SdiWindow passthru;
	
	if (!SdiInputInitialize() || !SdiOutputInitialize())
		return EXIT_FAILURE;
	
	// We can kill the dummy window now
	if (DestroyGLWindow(&hWnd, &hGLRC) == false)
		return false;
	

	SdiInputSetGlobalOptions(ringBufferSizeInFrames, captureFields);
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
	if (!SdiInputSetupGL())
	{
		std::cerr << "ERROR: Cannot setup opengl for input SDI. Abort. " << std::endl;
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
		GLenum status = SdiInputCaptureVideo();
		if (status != GL_SUCCESS_NV)
		{
			std::cout << "Capture fail : " << ((status == GL_FAILURE_NV) ? "GL_FAILURE_NV" : "GL_PARTIAL_SUCCESS_NV") << std::endl;
		}


		SdiAncCapture();
		passthru.DisplayVideo(SdiInputWidth(), SdiInputHeight());
		SdiAncPresent();
		SdiOutputPresentFrame();

		if (SdiInputDroppedFrames() > 0)
			std::cout 
			<< "Frame:   " << SdiInputFrameNumber() 
			<< "\tDropped: " << SdiInputDroppedFrames() 
			<< "\tTotal:   " << SdiInputDroppedFramesCount() 
			<< "   " << std::asctime(std::localtime(&timer));
	}

	unsigned int frame_count = SdiInputFrameNumber();
	unsigned int drop_frame_count = SdiInputDroppedFramesCount();

	SdiInputStop();
	
	SdiInputUnbindVideoTextureFrame();
	
	SdiInputCleanupGL();
	SdiOutputCleanupGL();

	SdiAncCleanupInput();
	SdiAncCleanupOutput();

	SdiOutputCleanupDevices();
	SdiInputCleanupDevices();
	
	passthru.Destroy();


	end_timer = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end_timer - start_timer;

	std::time_t start_time_t = std::chrono::system_clock::to_time_t(start_timer);
	std::time_t end_time_t = std::chrono::system_clock::to_time_t(end_timer);

	std::cout
		<< std::endl
		<< "---" << std::endl 
		<< "--- Program finished properly" << std::endl
		<< "--- Start at          : " << std::ctime(&start_time_t) 
		<< "--- Stop  at          : " << std::ctime(&end_time_t) 
		<< "--- Total time        : " << elapsed_seconds.count() << "sec" << std::endl
		<< "--- Frames displayed  : " << frame_count << std::endl
		<< "--- Frames dropped    : " << drop_frame_count << std::endl
		<< "--- Performance loss  : " << (float)drop_frame_count / (float)frame_count * 100.f << "%" << std::endl
		<< "--------------------------------------------------------" << std::endl;

	return EXIT_SUCCESS;
}

