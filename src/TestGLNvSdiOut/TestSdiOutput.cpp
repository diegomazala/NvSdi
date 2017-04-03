
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


	int win_width = 960;
	int win_height = 540;

	WinApp		lApp;
	CreationParameters lCreationParams;
	lCreationParams.Title = "Sdi Out Window";
	lCreationParams.WindowSize.Width = win_width;
	lCreationParams.WindowSize.Height = win_height;

	
		
	unsigned int frame_count = 0;
	try
	{
		SdiOutWindow sdiWindow;
		if (!sdiWindow.Create(lCreationParams, &lApp))        // Create Our Window
		{
			std::cerr << "ERROR: Cannot create the window application. Abort. " << std::endl;
			return EXIT_FAILURE;							// Quit If Window Was Not Created
		}


		sdiWindow.MakeCurrent();
		SdiSetExternalDC(sdiWindow.GetDC());
		SdiSetExternalGLRC(sdiWindow.GetGLRC());


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
		//
		// make sdi state similar to window state
		//
		SdiOutputMakeCurrent();
		sdiWindow.SetOpenGLState();	 

		sdiWindow.MakeCurrent();
		lApp.InitSetup();
		while (lApp.ProcessMainLoop())
		{
			SdiOutputMakeCurrent();

			GLfloat diffuse[] = { 0.04136, 0.54136, 0.31424 };
			glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);

			for (int i = 0; i<2; ++i)
			{
				SdiOutputBeginRender(0, i);
				sdiWindow.Draw(win_width * 2, win_height * 2);
				SdiOutputEndRender(0, i);
			}

			SdiOutRenderEventFunc()(static_cast<int>(SdiRenderEvent::PresentFrame));

			if (SdiOutputDuplicatedFrames() > 0)
				std::cout
				<< "Duplicated: " << SdiOutputDuplicatedFrames()
				<< "\tTotal: " << SdiOutputDuplicatedFramesCount()
				<< '\t' << current_time_to_string();

			if (++frame_count > max_frames && max_frames > 0)
				sdiWindow.Close();
		}

		//
		// Shutdown sdi
		//
		SdiOutRenderEventFunc()(static_cast<int>(SdiRenderEvent::Shutdown));
		
		sdiWindow.Destroy();
	}
	catch(const std::exception& e)
	{
		MessageBox(NULL, e.what(), "GLNvSdi Exception", MB_ICONERROR | MB_OK);
	}
	

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


