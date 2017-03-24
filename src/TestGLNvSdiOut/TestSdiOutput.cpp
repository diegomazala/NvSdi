
#include "GLNvSdi.h"
#include "SdiOutWindow.h"
#include "WinAppGL/WinApp.h"
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
	const int flipQueueLenght = ((argc > 1) ? atoi(argv[1]) : 5);
	const unsigned int sync_type = (argc > 2 ? atoi(argv[2]) : 2);
	const unsigned int max_frames = (argc > 3 ? atoi(argv[3]) : 0);

	std::chrono::time_point<std::chrono::system_clock> start_timer, end_timer;
	start_timer = std::chrono::system_clock::now();
	std::string start_time_str = current_time_to_string();




	//
	// Get the pointer to the funtion which manager the render events
	// This is an attempt to reproduce the same mechanism used by Unity
	//
	UnityRenderingEvent(*SdiOutRenderEventFunc)(void);
	SdiOutRenderEventFunc = &GetSdiOutputRenderEventFunc;




	WinApp		lApp;
	CreationParameters lCreationParams;
	lCreationParams.Title = "Sdi Out Window";
	lCreationParams.WindowSize.Width  = 960;
	lCreationParams.WindowSize.Height = 540;

	SdiOutWindow lWindow;
		
	unsigned int frame_count = 0;
	try
	{
		if(!lWindow.Create(lCreationParams, &lApp))        // Create Our Window
		{
			std::cerr << "ERROR: Cannot create the window application. Abort. " << std::endl;
			return EXIT_FAILURE;							// Quit If Window Was Not Created
		}

		HGLRC glrc1 = lWindow.GetGLRC();

		SdiSetDC(lWindow.GetDC());
		SdiSetGLRC(lWindow.GetGLRC());

		

		//
		// Initialize sdi
		//
		SdiOutRenderEventFunc()(static_cast<int>(SdiRenderEvent::Initialize));


		SdiOutputSetGlobalOptions();
		SdiOutputSetVideoFormat(HD_1080I_59_94, (SdiSyncSource)sync_type, 3.5f, 0, 0, false, flipQueueLenght);

		//
		// Setup sdi
		//
		SdiOutRenderEventFunc()(static_cast<int>(SdiRenderEvent::Setup));

		HGLRC glrc2 = SdiGetGLRC();

		//if (lWindow.SetupSdi())
		{
			
			lApp.InitSetup();
			while (lApp.ProcessMainLoop())
			{
				SdiOutRenderEventFunc()(static_cast<int>(SdiRenderEvent::PresentFrame));

				if (SdiOutputDuplicatedFrames() > 0)
					std::cout
					<< "Duplicated: " << SdiOutputDuplicatedFrames()
					<< "\tTotal: " << SdiOutputDuplicatedFramesCount()
					<< '\t' << current_time_to_string();

				if (++frame_count > max_frames && max_frames > 0)
					lWindow.Close();
			}

			//
			// Shutdown sdi
			//
			SdiOutRenderEventFunc()(static_cast<int>(SdiRenderEvent::Shutdown));
		}
		
	}
	catch(const std::exception& e)
	{
		MessageBox(NULL, e.what(), "GLNvSdi Exception", MB_ICONERROR | MB_OK);
	}
	
	lWindow.Destroy();


	end_timer = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end_timer - start_timer;
	std::time_t end_time_t = std::chrono::system_clock::to_time_t(end_timer);

	std::cout
		<< "Duplicated: " << SdiOutputDuplicatedFrames()
		<< "\tTotal: " << SdiOutputDuplicatedFramesCount()
		<< '\t' << current_time_to_string();
	

	std::cout
		<< std::endl
		<< "---" << std::endl
		<< "--- Program finished!" << std::endl
		<< "--- Start at          : " << start_time_str
		<< "--- Stop  at          : " << std::ctime(&end_time_t)
		<< "--- Total time        : " << elapsed_seconds.count() << " seconds" << std::endl
		<< "--- Frames displayed  : " << frame_count << std::endl
		<< "--- Frames duplicated : " << SdiOutputDuplicatedFramesCount() << std::endl
		<< "--- Performance loss  : " << (float)SdiOutputDuplicatedFramesCount() / (float)frame_count * 100.f << " %" << std::endl
		<< "--------------------------------------------------------" << std::endl;

	return EXIT_SUCCESS;
}


