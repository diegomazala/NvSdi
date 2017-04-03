#include "GLNvSdi.h"
#include "SdiInWindow.h"
#include <ctime>
#include <chrono>



std::string current_time_to_string()
{
	std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(p);
	std::stringstream ss;
	ss << std::ctime(&t);
	return ss.str();
}


int main(int argc, char* argv[])
{
	const int ringBufferSizeInFrames = ((argc > 1) ? atoi(argv[1]) : 2);
	const bool captureFields = (bool)((argc > 2) ? atoi(argv[2]) : true);
	const unsigned int max_frames = (argc > 3 ? atoi(argv[3]) : 0);

	std::chrono::time_point<std::chrono::system_clock> start_timer, end_timer;
	start_timer = std::chrono::system_clock::now();
	std::string start_time_str = current_time_to_string();

	int win_width = 960;
	int win_height = 540;
	
	WinApp lApp;
	SdiInWindow sdiWindow;
	CreationParameters lCreationParams;
	lCreationParams.Title = "Sdi Input Window";
	lCreationParams.WindowSize.Width = win_width;
	lCreationParams.WindowSize.Height = win_height;

	if (!sdiWindow.Create(lCreationParams, &lApp))        // Create Our Window
	{
		std::cerr << "ERROR: Cannot create the window application. Abort. " << std::endl;
		return EXIT_FAILURE;							// Quit If Window Was Not Created
	}
	sdiWindow.MakeCurrent();





	//
	// Get the pointer to the funtion which manager the render events
	// This is an attempt to reproduce the same mechanism used by Unity
	//
	UnityRenderingEvent(*SdiInRenderEventFunc)(void);
	SdiInRenderEventFunc = &GetSdiInputRenderEventFunc;

	//
	// Set input parameters
	//
	SdiInputSetBufferSize(ringBufferSizeInFrames);
	SdiInputSetCaptureFields(captureFields);

	//
	// Initialize sdi
	//
	SdiInRenderEventFunc()(static_cast<int>(SdiRenderEvent::Initialize));
	//
	// Setup
	//
	SdiInRenderEventFunc()(static_cast<int>(SdiRenderEvent::Setup));
	//
	// Start Capture
	//
	SdiInRenderEventFunc()(static_cast<int>(SdiRenderEvent::StartCapture));
	
	
	lApp.InitSetup();
	while(lApp.ProcessMainLoop())
	{

		//
		// Capture frame
		//
		SdiInRenderEventFunc()(static_cast<int>(SdiRenderEvent::CaptureFrame));


		if (SdiInputDroppedFrames() > 0)
		{
			std::cout
				<< "Frame:   " << SdiInputFrameNumber()
				<< "\tDropped: " << SdiInputDroppedFrames()
				<< "\tTotal:   " << SdiInputDroppedFramesCount()
				<< '\t' << current_time_to_string();
		}

		if (SdiInputFrameNumber() > max_frames && max_frames > 0)
			sdiWindow.Close();

		
		sdiWindow.DisplayVideo(win_width, win_height);
	}

	std::cout
		<< "Last :   " << SdiInputFrameNumber()
		<< "\tDropped: " << SdiInputDroppedFrames()
		<< "\tTotal:   " << SdiInputDroppedFramesCount()
		<< '\t' << current_time_to_string();

	unsigned int frame_count = SdiInputFrameNumber();
	unsigned int drop_frame_count = SdiInputDroppedFramesCount();


	//
	// Cleanup
	//
	SdiInRenderEventFunc()(static_cast<int>(SdiRenderEvent::StopCapture));
	SdiInRenderEventFunc()(static_cast<int>(SdiRenderEvent::Shutdown));

	sdiWindow.Destroy();


	end_timer = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end_timer - start_timer;
	std::time_t end_time_t = std::chrono::system_clock::to_time_t(end_timer);

	std::cout
		<< std::endl
		<< "---" << std::endl
		<< "--- Program finished!" << std::endl
		<< "--- Start at          : " << start_time_str
		<< "--- Stop  at          : " << std::ctime(&end_time_t)
		<< "--- Total time        : " << elapsed_seconds.count() << " seconds" << std::endl
		<< "--- Frames displayed  : " << frame_count << std::endl
		<< "--- Frames dropped    : " << drop_frame_count << std::endl
		<< "--- Performance loss  : " << (float)drop_frame_count / (float)frame_count * 100.f << " %" << std::endl
		<< "--------------------------------------------------------" << std::endl;

	return EXIT_SUCCESS;
}
