#include "GLNvSdi.h"
#include "SdiWindow.h"
#include <ctime>
#include <chrono>
#include "GLFont.h"


std::string current_time_to_string()
{
	std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(p);
	std::stringstream ss;
	ss << std::ctime(&t);
	return ss.str();
}

static void frame_count_to_timecode(uint64_t frame_number, uint32_t& hours, uint32_t& minutes, uint32_t& seconds, uint32_t& frames)
{
	frames = frame_number % 30;
	seconds = (frame_number / 30) % 60;
	minutes = ((frame_number / 30) / 60) % 60;
	hours = (((frame_number / 30) / 60) / 60) % 24;
}


int main(int argc, char* argv[])
{
	const int ringBufferSizeInFrames = ((argc > 1) ? atoi(argv[1]) : 2);
	const int ringBufferSizeOutFrames = ((argc > 2) ? atoi(argv[2]) : 2);
	const bool outputDelay = ((argc > 3) ? atof(argv[3]) : 0.f);
	const bool captureFields = (bool)((argc > 4) ? atoi(argv[4]) : true);
	const unsigned int sync_type = (argc > 5 ? atoi(argv[5]) : 2);
	const unsigned int max_frames = (argc > 6 ? atoi(argv[6]) : 0);

	std::chrono::time_point<std::chrono::system_clock> start_timer, end_timer;
	start_timer = std::chrono::system_clock::now();
	std::string start_time_str = current_time_to_string();


	HWND hWnd;
	HGLRC hGLRC;
	if (CreateDummyGLWindow(&hWnd, &hGLRC) == false)
		return false;

	GLFont gFont;
	gFont.Create(-48, "Arial");
	static int timecode[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	SdiSetDC();
	SdiSetGLRC(hGLRC);

	if (!SdiInputInitialize() || !SdiOutputInitialize())
		return EXIT_FAILURE;


	SdiInputSetGlobalOptions(ringBufferSizeInFrames, captureFields);
	SdiOutputSetGlobalOptions();
	SdiOutputSetVideoFormat(HD_1080I_59_94, (SdiSyncSource)sync_type, outputDelay, 0, 0, false, ringBufferSizeOutFrames);
	//SdiOutputSetVideoFormat(HD_1080I_59_94, COMP_SYNC, outputDelay, 788, 513, false, 2);
	//SdiOutputSetVideoFormat(HD_1080I_59_94, NONE, 4.5f, 0, 0, false, 2);
	//SdiOutputSetVideoFormat(SD_487I_59_94, COMP_SYNC, 4.5f, 788, 513, false, 2);

	SdiOutputPrintStats(false);

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
	lCreationParams.WindowSize.Width = window_w / 2;
	lCreationParams.WindowSize.Height = window_h / 2;


	SdiWindow passthru;
	if (!passthru.Create(lCreationParams, &lApp))        // Create Our Window
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

#if 0
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
#else
	SdiOutputCreateTextures();
	SdiOutputInitializeFbo();
#endif

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


	SdiInputResetDroppedFramesCount();
	SdiOutputResetDuplicatedFramesCount();

	uint64_t last_drop = 0;
	uint64_t out_frame_count = 0;
	std::chrono::duration<double> loop_elapsed_seconds;

	lApp.InitSetup();
	while (lApp.ProcessMainLoop())
	{
		auto begin_loop_timer = std::chrono::system_clock::now();

		GLenum status = SdiInputCaptureVideo();
		if (status != GL_SUCCESS_NV)
		{
			std::cout << "Capture fail : " << ((status == GL_FAILURE_NV) ? "GL_FAILURE_NV" : "GL_PARTIAL_SUCCESS_NV") << std::endl;
		}


		SdiAncCapture();
		SdiAncGetTimeCode(timecode, 0);
		//passthru.RenderToSdi(SdiInputWidth(), SdiInputHeight());

		for (int f = 0; f < 2; ++f)
		{
			SdiOutputBeginRender(0, f);
			{
				glClearColor(1 - f, f, 0.0, 0.0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				SdiInputGetTexture(0)->Enable();
				SdiInputGetTexture(0)->Bind();
				SdiInputGetTexture(0)->Plot(SdiInputWidth(), SdiInputHeight(), SdiInputWidth(), SdiInputHeight());

				glColor3f(1, 1, 1);
				gFont.Plot(-0.95f, -0.60f, "GPU Time: %f. GVI Time: %f", SdiInputGpuTime(), SdiInputGviTime());
				gFont.Plot(-0.95f, -0.70f, "In(%d) Out(%d) ", SdiInputDroppedFramesCount(), SdiOutputDuplicatedFramesCount());
				uint32_t hours, minutes, seconds, frame_number;
				frame_count_to_timecode(out_frame_count, hours, minutes, seconds, frame_number);
				//gFont.Plot(-0.95f, -0.80f, "%05d - %02d:%02d:%02d:%02d", out_frame_count, hours, minutes, seconds, frame_number);
				gFont.Plot(-0.95f, -0.80f, "%d %d - %d%d:%d%d:%d%d:%d%d", SdiInputFrameNumber(), SdiInputFrameNumber() - last_drop,
					timecode[0], timecode[1], timecode[2], timecode[3],
					timecode[4], timecode[5], timecode[6], timecode[7]);
				//gFont.Plot(-0.95f, -0.70f, "Out (%d) ", SdiOutputDuplicatedFramesCount());
				gFont.Plot(-0.95f, -0.90f, "Loop Time (%f) ", loop_elapsed_seconds.count());

				SdiInputGetTexture(0)->Unbind();
				SdiInputGetTexture(0)->Disable();
			}
			SdiOutputEndRender(0, f);
		}

					
		SdiAncPresent();

		double frame_rate_ns = 1000000000.0 / SdiInputFrameRate();
		const uint64_t minPresentTime = SdiInputCaptureTime() + outputDelay * frame_rate_ns;
		SdiOutputPresentFrame(minPresentTime);
		

		if (SdiInputDroppedFrames() > 0 || SdiOutputDuplicatedFrames() > 0)
		{
			std::cout
				<< "Frame: " << SdiInputFrameNumber() << ' ' << SdiInputFrameNumber() - last_drop
				<< "\tDrop IO : " << SdiInputDroppedFrames() << ", " << SdiOutputDuplicatedFrames()
				<< "\t- " << SdiInputDroppedFramesCount() << ", " << SdiOutputDuplicatedFramesCount()
				<< '\t' << SdiInputGviTime() << '\t' << current_time_to_string();

			last_drop = SdiInputFrameNumber();
		}

		auto end_loop_timer = std::chrono::system_clock::now();

		loop_elapsed_seconds = end_loop_timer - begin_loop_timer;

		int drop = SdiOutputDuplicatedFrames();

		++out_frame_count;

		passthru.DisplayVideo(SdiInputWidth(), SdiInputHeight());

		if (SdiInputFrameNumber() > max_frames && max_frames > 0)
			passthru.Close();
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


	std::cout
		<< "Last : " << SdiInputFrameNumber()
		<< "\tDrop IO : " << SdiInputDroppedFrames() << ", " << SdiOutputDuplicatedFrames()
		<< "\t- " << SdiInputDroppedFramesCount() << ", " << SdiOutputDuplicatedFramesCount()
		<< '\t' << SdiInputGviTime() << '\t' << current_time_to_string();


	unsigned int frame_count = SdiInputFrameNumber();
	unsigned int drop_frame_count = SdiInputDroppedFramesCount();
	unsigned int duplicate_frame_count = SdiOutputDuplicatedFramesCount();


	end_timer = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end_timer - start_timer;
	std::time_t end_time_t = std::chrono::system_clock::to_time_t(end_timer);

	std::cout
		<< std::endl
		<< "---" << std::endl
		<< "--- Program finished!" << std::endl
		<< "--- Start at                 : " << start_time_str
		<< "--- Stop  at                 : " << std::ctime(&end_time_t)
		<< "--- Total time               : " << elapsed_seconds.count() << " seconds" << std::endl
		<< "--- Frames displayed         : " << frame_count << std::endl
		<< "--- Input Frames dropped     : " << drop_frame_count << std::endl
		<< "--- Input Performance loss   : " << (float)drop_frame_count / (float)frame_count * 100.f << " %" << std::endl
		<< "--- Output Frames dropped    : " << duplicate_frame_count << std::endl
		<< "--- Output Performance loss  : " << (float)duplicate_frame_count / (float)frame_count * 100.f << " %" << std::endl
		<< "--------------------------------------------------------" << std::endl;

	getchar();
	return EXIT_SUCCESS;
}

