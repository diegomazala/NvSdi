
#include "GLNvSdi.h"
#include "glExtensions.h"
#include "timecode.h"
#include "ANCapi.h"

#include <fcntl.h>
#include <io.h>

extern "C"
{
	std::stringstream gSdiLog;
	
	static HWND		gSdiWnd = NULL;
	static HGLRC	gSdiGLRC = NULL;
	static HDC		gSdiDC = NULL;
	static HDC		gSdiAffinityDC = NULL;
	static HGLRC	gUtyGLRC = NULL;
	static HDC		gUtyDC = NULL;
	

	static SdiOptions gOptions;

	

	namespace attr
	{
		bool m_bAncInInitialized		= false;
		bool m_bAncOutInitialized		= false;
		NVVIOANCDATAFRAME m_ancData[4];				// Per-stream ancillary data
		NvU32  m_SequenceLength			= 0;		// Audio sample sequence length
		NvU32 *m_NumAudioSamples		= NULL;	    // Number of audio samples
		CTimeCode m_TimeCode;						// Timecode

		Logger	gSdiLogger;
	}


	GLNVSDI_API int SdiError()
	{
		return sdiError;
	}
	
	GLNVSDI_API std::stringstream& SdiLog()
	{
		return gSdiLog;
	}
	

	GLNVSDI_API void SdiClearLog()
	{
		gSdiLog.clear();
	}



	GLNVSDI_API void SdiGetLog(void* log_str, int max_length)
	{
		if( !log_str )
			return;

		char* logStr = reinterpret_cast<char*>(log_str);

		std::string str = gSdiLog.str();

		if (str.length() > max_length)
		{
			int index = str.length() - max_length;
			str = str.substr(index, max_length);
		}

		strcpy(logStr, str.c_str());
	}


	GLNVSDI_API void SdiSetGLRC(HGLRC hglrc)
	{
		if (hglrc == NULL)
			gSdiGLRC = wglGetCurrentContext();
		else
			gSdiGLRC = hglrc;
	}

	GLNVSDI_API void SdiSetCurrentGLRC()
	{
		gSdiGLRC = wglGetCurrentContext();
	}

	GLNVSDI_API HGLRC SdiGetGLRC()
	{
		return gSdiGLRC;
	}

	GLNVSDI_API void SdiCreateGLRC(HDC hdc)
	{
		//Create affinity-rendering context from affinity-DC
		gSdiGLRC = wglCreateContext(hdc);
	}

	GLNVSDI_API void SdiSetDC(HDC hdc)
	{
		if (hdc == NULL)
			gSdiDC = wglGetCurrentDC();
		else
			gSdiDC = hdc;
	}

	GLNVSDI_API void SdiSetCurrentDC()
	{
		gSdiDC = wglGetCurrentDC();
	}

	GLNVSDI_API HDC SdiGetDC()
	{
		return gSdiDC;
	}

	GLNVSDI_API void SdiSetAffinityDC(HDC hdc)
	{
		gSdiAffinityDC = hdc;
	}

	GLNVSDI_API HDC SdiGetAffinityDC()
	{
		return gSdiAffinityDC;
	}


	GLNVSDI_API HDC SdiCreateAffinityDC()
	{
		HGPUNV  gpuMask[2];
		gpuMask[0] = CNvGpuTopology::instance().getPrimaryGpu()->getAffinityHandle();
		gpuMask[1] = NULL;

		gSdiAffinityDC = wglCreateAffinityDCNV(gpuMask);
		if (gSdiAffinityDC != NULL)
		{
			PIXELFORMATDESCRIPTOR pfd =							// pfd Tells Windows How We Want Things To Be
			{
				sizeof (PIXELFORMATDESCRIPTOR),					// Size Of This Pixel Format Descriptor
				1,												// Version Number
				PFD_DRAW_TO_WINDOW |							// Format Must Support Window
				PFD_SUPPORT_OPENGL |							// Format Must Support OpenGL
				PFD_DOUBLEBUFFER,								// Must Support Double Buffering
				PFD_TYPE_RGBA,									// Request An RGBA Format
				32,												// Select Our Color Depth
				0, 0, 0, 0, 0, 0,								// Color Bits Ignored
				1,												// Alpha Buffer
				0,												// Shift Bit Ignored
				0,												// No Accumulation Buffer
				0, 0, 0, 0,										// Accumulation Bits Ignored
				24,												// 24 Bit Z-Buffer (Depth Buffer)  
				8,												// 8 Bit Stencil Buffer
				0,												// No Auxiliary Buffer
				PFD_MAIN_PLANE,									// Main Drawing Layer
				0,												// Reserved
				0, 0, 0											// Layer Masks Ignored
			};

			GLuint pf = ChoosePixelFormat(gSdiAffinityDC, &pfd);
			HRESULT rslt = ::SetPixelFormat(gSdiAffinityDC, pf, &pfd);

			return gSdiAffinityDC;
		}
		else
		{
			std::cout << "Unable to create GPU affinity DC" << std::endl;
			return NULL;
		}
	}

	GLNVSDI_API bool SdiMakeCurrent()
	{
		return wglMakeCurrent(SdiGetDC(), SdiGetGLRC()); 	
	}


	GLNVSDI_API void SdiSetUtyDC()
	{
		gUtyDC = wglGetCurrentDC();
	}

	GLNVSDI_API void SdiSetUtyGLRC()
	{
		gUtyGLRC = wglGetCurrentContext();
	}

	GLNVSDI_API bool SdiMakeUtyCurrent()
	{
		return wglMakeCurrent(gUtyDC, gUtyGLRC); 	
	}

	





	GLNVSDI_API SdiOptions& SdiGlobalOptions()
	{
		return gOptions;
	}

	GLNVSDI_API int SdiGpuCount()
	{
		return CNvGpuTopology::instance().getNumGpu();
	}
		
	GLNVSDI_API int SdiInputGpuCount()
	{
		return CNvSDIinTopology::instance().getNumDevice();
	}

	GLNVSDI_API int SdiOutputGpuCount()
	{
		return CNvSDIoutGpuTopology::instance().getNumGpu();
	}


	GLNVSDI_API int SdiSetupPixelFormat(HDC hDC)
	{
		int pf;
		PIXELFORMATDESCRIPTOR pfd;

		// fill in the pixel format descriptor 
		pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion     = 1;		    // version (should be 1) 
		pfd.dwFlags      =	PFD_DRAW_TO_WINDOW | // draw to window (not bitmap) 
			PFD_SUPPORT_OPENGL | // draw using opengl 
			PFD_DOUBLEBUFFER;
		pfd.iPixelType   = PFD_TYPE_RGBA;                // PFD_TYPE_RGBA or COLORINDEX 
		pfd.cColorBits   = 24;
		pfd.cDepthBits	 = 32;

		// get the appropriate pixel format 
		pf = ::ChoosePixelFormat(hDC, &pfd);
		if (pf == 0) {
			printf("ChoosePixelFormat() failed:  Cannot find format specified."); 
			return 0;
		} 

		// set the pixel format 
		if (::SetPixelFormat(hDC, pf, &pfd) == FALSE) {
			printf("SetPixelFormat() failed:  Cannot set format specified.");
			return 0;
		} 

		return pf;
	}    

	GLNVSDI_API void SdiSetupLogFile()
	{
		attr::gSdiLogger.ToFile("GLNvSdi");
	}


	GLNVSDI_API void SdiSetupLogConsole()
	{
#if 1
		attr::gSdiLogger.ToScreen();
#else

		int hCrt;
		FILE *hf;
		static int initialized = 0;

		if(initialized == 1) {
			return;
		}

		::AllocConsole();

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
#endif
	}







	//
	// Initialize ANC data capture
	//
	GLNVSDI_API bool SdiAncSetupInput()
	{
		if (SdiInput() == NULL)
			return false;

		if (attr::m_bAncInInitialized) 
			return true;

		// Initialize ANC API
		if (NvVIOANCAPI_InitializeGVI(SdiInput()->GetVioHandle()) != NVAPI_OK) 
		{
			return false;
		}

		// Get frame rate and initialize timecode counter
		float frameRate;
		if (SdiInput()->GetFrameRate(&frameRate) != S_OK) 
		{
			return false;
		}

		float l_fFrameRate = 0.0f;
		attr::m_TimeCode.init((UINT)l_fFrameRate);

		// Initialize ancillary data structure for VITC and custom packet capture.
		for (int i = 0; i < 4; i++) 
		{
			attr::m_ancData[i].version = NVVIOANCDATAFRAME_VERSION;
			attr::m_ancData[i].fields = NVVIOANCDATAFRAME_VITC | NVVIOANCDATAFRAME_CUSTOM;
		}

		// Set desired number of custom packets to capture and 
		// allocate space for the custom data per stream.
		for (int i = 0; i < 4; i++) 
		{
			attr::m_ancData[i].numCustomPackets = 10000;
			attr::m_ancData[i].CustomPackets = (NVVIOANCDATAPACKET *)calloc(attr::m_ancData[i].numCustomPackets, sizeof(NVVIOANCDATAPACKET));
		}

		// Determine the length of the audio sample sequence.
		NvVIOANCAPI_NumAudioSamples(SdiInput()->GetVioHandle(), 
			NVVIOANCAUDIO_SAMPLING_RATE_48_0,
			(NvU32 *)&attr::m_SequenceLength, 
			NULL);

		// Allocate/reallocated required memory for the array to hold the number 
		// of  audio samples for each frame in a sequence.
		if (attr::m_NumAudioSamples)
			free(attr::m_NumAudioSamples);
		attr::m_NumAudioSamples = (NvU32 *)calloc((size_t)attr::m_SequenceLength, sizeof(NvU32));

		// Determine number of audio samples based on signal format
		// and audio sampling rate.
		NvVIOANCAPI_NumAudioSamples(SdiInput()->GetVioHandle(), 
			NVVIOANCAUDIO_SAMPLING_RATE_48_0,
			(NvU32 *)&attr::m_SequenceLength, 
			(NvU32 *)attr::m_NumAudioSamples);

		// Determine the maximum number of audio sample for any given frame.
		// Use this value when allocating space to store audio samples
		NvU32 maxNumAudioSamples = 0;
		for (unsigned int i = 0; i < attr::m_SequenceLength; i++) 
		{
			if (attr::m_NumAudioSamples[i] > maxNumAudioSamples) 
			{
				maxNumAudioSamples = attr::m_NumAudioSamples[i];
			}
		}

		// Allocate space required for audio data packets for each audio group, four channels per group.  
		// Space required depends upon signal format and audio rate.  Set bit field to indicate 
		// desired audio channels to capture.
		for (int i = 0; i < 4; i++) 
		{
			for (int j = 0; j < 4; j++) 
			{
				attr::m_ancData[i].AudioGroup1.numAudioSamples = maxNumAudioSamples;
				attr::m_ancData[i].AudioGroup1.audioData[j] = (NvU32 *)calloc(maxNumAudioSamples, sizeof(NvU32));
				attr::m_ancData[i].fields |= NVVIOANCDATAFRAME_AUDIO_GROUP_1;

				attr::m_ancData[i].AudioGroup2.numAudioSamples = maxNumAudioSamples;
				attr::m_ancData[i].AudioGroup2.audioData[j] = (NvU32 *)calloc(maxNumAudioSamples, sizeof(NvU32));
				attr::m_ancData[i].fields |= NVVIOANCDATAFRAME_AUDIO_GROUP_2;

				attr::m_ancData[i].AudioGroup3.numAudioSamples = maxNumAudioSamples;
				attr::m_ancData[i].AudioGroup3.audioData[j] = (NvU32 *)calloc(maxNumAudioSamples, sizeof(NvU32));
				attr::m_ancData[i].fields |= NVVIOANCDATAFRAME_AUDIO_GROUP_3;

				attr::m_ancData[i].AudioGroup4.numAudioSamples = maxNumAudioSamples;
				attr::m_ancData[i].AudioGroup4.audioData[j] = (NvU32 *)calloc(maxNumAudioSamples, sizeof(NvU32));
				attr::m_ancData[i].fields |= NVVIOANCDATAFRAME_AUDIO_GROUP_4;
			}
		}

		attr::m_bAncInInitialized = true;

		return true;
	}






	//
	// Cleanup ANC data capture
	//
	GLNVSDI_API void SdiAncCleanupInput()
	{
		if (attr::m_bAncInInitialized) 
		{
			attr::m_bAncInInitialized = false;

			if (attr::m_NumAudioSamples) 
			{
				free(attr::m_NumAudioSamples);
				attr::m_NumAudioSamples = NULL;
				attr::m_SequenceLength = 0;
			}

			// Free custom data space
			for(int i = 0; i < 4; i++) 
			{
				free(attr::m_ancData[i].CustomPackets);
			}

			// Release ANC API
			NvVIOANCAPI_ReleaseGVI(SdiInput()->GetVioHandle());
		} 
	}	

	//
	// Capture frame of ANC data.
	//
	GLNVSDI_API bool SdiAncCapture()
	{
		if (!attr::m_bAncInInitialized)
			return false;

		static int counter = 0;
		int frameTens;
		int frameUnits;
		int secondTens;
		int secondUnits;
		int minuteTens;  
		int minuteUnits;
		int hourTens;
		int hourUnits;

		// Capture ANC data	
		attr::m_ancData->numCustomPackets = 10000;

		// Free custom data space
		for(int i = 0; i < 4; i++) {
			memset(attr::m_ancData[i].CustomPackets, 0, attr::m_ancData->numCustomPackets * sizeof(NVVIOANCDATAPACKET));
		}

		attr::m_ancData[0].fields = NVVIOANCDATAFRAME_VITC | NVVIOANCDATAFRAME_AUDIO_GROUP_1;
		attr::m_ancData[0].VITCTimecode = 0;

		if (NvVIOANCAPI_CaptureANCData(SdiInput()->GetVioHandle(), attr::m_ancData) != NVAPI_OK)
			return false;

		//printf("\nSTART FRAME\n");

		// Print timecode.  If captured timecode is not 0, use it, 
		// otherwise use internal local timecode.
		if (attr::m_ancData[0].VITCTimecode != 0) 
		{
			// Draw timecode for first video stream
			frameTens = (attr::m_ancData[0].VITCTimecode >> 4) & 0x3;
			frameUnits = attr::m_ancData[0].VITCTimecode & 0xf;
			secondTens = (attr::m_ancData[0].VITCTimecode >> 12) & 0x7;
			secondUnits = (attr::m_ancData[0].VITCTimecode >> 8) & 0xf;
			minuteTens = (attr::m_ancData[0].VITCTimecode >> 20) & 0x7;  
			minuteUnits = (attr::m_ancData[0].VITCTimecode >> 16) & 0xf;
			hourTens = (attr::m_ancData[0].VITCTimecode >> 28)& 0x3;
			hourUnits = (attr::m_ancData[0].VITCTimecode >> 24) & 0xf;

			//std::cout << hourTens << hourUnits << ':' << minuteTens << minuteUnits << ':' << secondTens << secondUnits << ':' << frameTens << frameUnits << std::endl;
			
			//printf("%d%d:%d%d:%d%d:%d%d\n", hourTens, hourUnits, minuteTens, minuteUnits, 
			//		secondTens, secondUnits, frameTens, frameUnits); 

		} 
		else 
		{

			frameTens = attr::m_TimeCode.frame() / 10;
			frameUnits = attr::m_TimeCode.frame() % 10;
			secondTens = attr::m_TimeCode.second() / 10;
			secondUnits = attr::m_TimeCode.second() % 10;
			minuteTens = attr::m_TimeCode.minute() / 10;
			minuteUnits = attr::m_TimeCode.minute() % 10;
			hourTens = attr::m_TimeCode.hour() / 10;
			hourUnits = attr::m_TimeCode.hour() % 10;

			//std::cout << hourTens << hourUnits << ':' << minuteTens << minuteUnits << ':' << secondTens << secondUnits << ':' << frameTens << frameUnits << '*' << std::endl;
			
			// Print.  Asterisk denotes timecode is generated.
			//printf("%d%d:%d%d:%d%d:%d%d*\n", hourTens, hourUnits, minuteTens, minuteUnits, 
			//		secondTens, secondUnits, frameTens, frameUnits);  

			// Set relevant bits here
			short dropFrame = 1;
			short colorFrame = 1;
			short fieldPhase = 0;
			short bg0 = 1;
			short bg1 = 0;
			short bg2 = 1;

			attr::m_ancData[0].VITCTimecode = hourTens << 28 | hourUnits << 24 |  minuteTens << 20 |
				minuteUnits << 16 | secondTens << 12 | secondUnits << 8 | 
				frameTens << 4 | frameUnits;
			attr::m_ancData[0].VITCTimecode |= dropFrame << 6;
			attr::m_ancData[0].VITCTimecode |= colorFrame << 7;
			attr::m_ancData[0].VITCTimecode |= fieldPhase << 15;
			attr::m_ancData[0].VITCTimecode |= bg0 << 23;
			attr::m_ancData[0].VITCTimecode |= bg1 << 30;
			attr::m_ancData[0].VITCTimecode |= bg2 << 31;

			attr::m_ancData[0].VITCUserBytes = counter;
			counter++;

			// Increment the generated timecode.
			attr::m_TimeCode.increment();


		}

#ifdef DUMP_CUSTOM

		printf("Num Custom Packets: %d\n", m_ancData[0].numCustomPackets);
		for (unsigned int i = 0; i < m_ancData[0].numCustomPackets; i++) {
			for (unsigned int j = 0; j < m_ancData[0].CustomPackets[i].DC; j++) {

				// Get rid of 0s.
				//			if (m_ancData[0].CustomPackets[i].data[j])
				printf("%d", m_ancData[0].CustomPackets[i].data[j]);
			}
			printf("\n");
		}

#endif

#define DUMP_AUDIO
#ifdef DUMP_AUDIO

		NvU32 *a1 = attr::m_ancData[0].AudioGroup1.audioData[0];
		NvU32 *a2 = attr::m_ancData[0].AudioGroup1.audioData[1];
		NvU32 *a3 = attr::m_ancData[0].AudioGroup1.audioData[2];
		NvU32 *a4 = attr::m_ancData[0].AudioGroup1.audioData[3];

		//printf("Number of audio samples: %d\n", attr::m_ancData[0].AudioGroup1.numAudioSamples);

#if 0
		for (unsigned int i = 0; i < m_ancData[0].AudioGroup1.numAudioSamples; i++) {
#if 0
			printf(" %0x %0x %0x %0x", *a1++, *a2++, *a3++, *a4++);
#else
			// SMPTE 299
			//printf(" %0x ", (*a1++ & 0xffff0));
			// SMPTE 272
			printf(" %0x ", (*a1++ & 0x7ffff8));
#endif
		}
#endif

#endif

		return true;
		//
	}


	GLNVSDI_API bool SdiAncSetupOutput()
	{
		if (!attr::m_bAncOutInitialized) 
		{
			// Initialize ANC API
			if (NvVIOANCAPI_InitializeGVO(SdiOutput()->getHandle()) != NVAPI_OK) 
			{
				return false;
			}
		}

		return attr::m_bAncOutInitialized = true;
	}


	GLNVSDI_API void SdiAncCleanupOutput()
	{	
		if (attr::m_bAncOutInitialized) 
		{
			attr::m_bAncOutInitialized = false;
			NvVIOANCAPI_ReleaseGVO(SdiOutput()->getHandle());
		}
	}


	GLNVSDI_API void SdiAncPresent()
	{
		// Send ANC data
		if (attr::m_bAncOutInitialized) 
		{
			attr::m_ancData[0].fields = NVVIOANCDATAFRAME_VITC;
			attr::m_ancData[0].fields |= NVVIOANCDATAFRAME_AUDIO_GROUP_1;
			NvVIOANCAPI_SendANCData(NULL, attr::m_ancData);
		}
	}


	



	GLNVSDI_API bool SdiAncGetTimeCode(void* timecode_int8, int video_index)
	{	
		// safeguard - array must be not null
		if( !timecode_int8 )
			return false;

		int* pTimeCodeData = reinterpret_cast<int*>(timecode_int8);

		if( !pTimeCodeData )
			return false;


		// Draw timecode for first video stream
		int frameTens = (attr::m_ancData[video_index].VITCTimecode >> 4) & 0x3;
		int frameUnits = attr::m_ancData[video_index].VITCTimecode & 0xf;
		int secondTens = (attr::m_ancData[video_index].VITCTimecode >> 12) & 0x7;
		int secondUnits = (attr::m_ancData[video_index].VITCTimecode >> 8) & 0xf;
		int minuteTens = (attr::m_ancData[video_index].VITCTimecode >> 20) & 0x7;  
		int minuteUnits = (attr::m_ancData[video_index].VITCTimecode >> 16) & 0xf;
		int hourTens = (attr::m_ancData[video_index].VITCTimecode >> 28)& 0x3;
		int hourUnits = (attr::m_ancData[video_index].VITCTimecode >> 24) & 0xf;


		(*(pTimeCodeData + 0))  = hourTens;
		(*(pTimeCodeData + 1))  = hourUnits;
		(*(pTimeCodeData + 2))  = minuteTens;
		(*(pTimeCodeData + 3))  = minuteUnits;
		(*(pTimeCodeData + 4))  = secondTens;
		(*(pTimeCodeData + 5))  = secondUnits;
		(*(pTimeCodeData + 6))  = frameTens;
		(*(pTimeCodeData + 7))  = frameUnits;

		return true;
	}

};	//extern "C"
