
#ifndef _SDI_IN_ASYNC_WINDOW_H_
#define _SDI_IN_ASYNC_WINDOW_H_

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <assert.h>
#include <stdio.h>

#include "OpenGL.h"

#include "nvSDIin.h"


#include "timecode.h"

#include "ANCapi.h"


#include "WinAppGL/GLWindow.h"

#include "DVP.h"

class SdiInAsyncWindow : public GLWindow
{
public:

	SdiInAsyncWindow();
	~SdiInAsyncWindow();

	void DisplayVideo(int vid_w, int vid_h);

	void CalcWindowSize(int vid_w, int vid_h, int stream_count, NVVIOSIGNALFORMAT signal_format, int& win_w, int& win_h);


	void Render();
	void OnKeyEvent(const KeyEvent* pEvent);



	bool IsSdiAvailable();
	HRESULT SetupSDIPipeline();
	GLboolean SetupGLPipe();
	HRESULT StartSDIPipeline();
	HRESULT StopSDIPipeline();
	GLboolean DisplayVideo();
	GLboolean CleanupGL();
	HRESULT CleanupSDIPipeline();
	void Shutdown();
	void ToggleShowStatistics(){ m_ShowStatistics = !m_ShowStatistics; }


	HDC GetAffinityDC() { return m_AffinityDC; }

protected:

	bool InitGL();


	C_DVP m_pipe;
	GLuint m_drawTimeQuery;
	HDC m_AffinityDC;
	bool m_ShowStatistics;
};


#endif // _SDI_IN_ASYNC_WINDOW_H_
