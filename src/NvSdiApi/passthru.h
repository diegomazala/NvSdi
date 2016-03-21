//*******************************************************************************
// passthru: captures video/ancillary data from a single capture card into texture 
//objects on a Primary GPU and data buffers in system memory.
//The sample displays the video and some of the captured ancillary data
//The capture card can be selected.
//All the incoming video signals are assumed to be genlocked.
//*******************************************************************************

#ifndef passthru_H
#define passthru_H

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <assert.h>
#include <stdio.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/wglext.h>

#include "nvSDIin.h"
#include "nvSDIout.h"

#include "timecode.h"

#include "ANCapi.h"

typedef class C_passthru
{
protected:
	CNvSDIoutGpu *m_gpu;

	GLuint m_videoTextures[MAX_VIDEO_STREAMS];
	Options options;	

	int m_videoWidth;
	int m_videoHeight;
	void drawOne();
	void drawTwo();
	void drawThree();
	void drawFour();
	CNvSDIin m_SDIin;
	CNvSDIout m_SDIout;
	HRESULT setupSDIinGL();
	HRESULT setupSDIoutGL();
	HRESULT setupSDIinDevices();
	HRESULT setupSDIoutDevice();
	HRESULT cleanupSDIinGL();
	HRESULT cleanupSDIoutGL();

	void captureAnc();
	HRESULT initializeAncIn(void);
	HRESULT initializeAncOut(void);
	HRESULT cleanupAncIn(void);
	HRESULT cleanupAncOut(void);

	bool m_bAncInInitialized;
	bool m_bAncOutInitialized;

	bool m_bSDIout;

	HWND m_hWnd;							// Window handle
	HDC	m_hDC;								// Device context
	HGLRC m_hRC;							// OpenGL rendering context
	HDC m_hAffinityDC;
public:

	GLuint m_windowWidth;                   // Window width
	GLuint m_windowHeight;                  // Window height

	NVVIOANCDATAFRAME m_ancData[4];         // Per-stream ancillary data
	NvU32  m_SequenceLength;                // Audio sample sequence length
	NvU32 *m_NumAudioSamples;		        // Number of audio samples
	CTimeCode m_TimeCode;				    // Timecode
	C_passthru();
	~C_passthru();
	HRESULT Configure(char *szCmdLine[]);
	HRESULT SetupSDIDevices();
	HRESULT SetupCapture();
	GLboolean  SetupGL();	
	GLboolean CleanupGL();
	HRESULT CleanupSDIDevices();
	HRESULT StartSDIPipeline();
	HRESULT StopSDIPipeline();
	GLenum  CaptureVideo();
	GLboolean DisplayVideo();
	void Shutdown();
	void CalcWindowSize();
	HWND SetupWindow(HINSTANCE hInstance, int x, int y, char *title);
	void ResizeWindow(int width, int height);
	HRESULT SetupAnc();
	HRESULT CleanupAnc();

}C_passthru;


//
// Function prototypes
//

// From Console.cpp
void SetupConsole();

#endif passthru_H