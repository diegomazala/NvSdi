#ifndef AUDIOIN_H
#define AUDIOIN_H

#include <windows.h>

// DirectSound includes
#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>
#include <ks.h>
#include <ksmedia.h>

//-----------------------------------------------------------------------------
// Defines and constants
//-----------------------------------------------------------------------------
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

#ifndef TRACE_ERR_MSGBOX
#define TRACE_ERR_MSGBOX(str) {MessageBox(NULL, str, "Error", MB_OK); }
#endif

#define NUM_REC_NOTIFICATIONS  16

#define MAX(a,b)        ( (a) > (b) ? (a) : (b) )

typedef void (*AUDIOCALLBACK)(
    DWORD, 
	void *,
	void *
    );

typedef class CAudioIn
{
private:
	// PCM audio with more than 2 channels, greater than 16 bits per
	// sample and more than 44,100 samples per second must utilize the 
	// WAVEFORMATEXTENSIBLE structure.  Need to figure out how to use
	// WAVEFORMATEXTENSIBLE to specify a DirectSound8 capture buffer.

#if 0
	WAVEFORMATEXTENSIBLE       m_wfxInput;
#else
	WAVEFORMATEX			   m_wfxInput;
#endif

	LPDIRECTSOUNDNOTIFY        m_pDSNotify;

	LPDIRECTSOUNDCAPTURE       m_pDSCapture;
	LPDIRECTSOUNDCAPTUREBUFFER m_pDSBCapture;
	DWORD                      m_dwCaptureBufferSize;
	DWORD                      m_dwNextCaptureOffset;

	DWORD                      m_dwAudioBufferSize; 
	DWORD                      m_dwNotifySize;
	DSBPOSITIONNOTIFY          m_aPosNotify[ NUM_REC_NOTIFICATIONS + 1 ]; 

	DWORD                      m_dwNextWriteOffset;

	AUDIOCALLBACK              m_func;

	HRESULT InitNotifications( HANDLE hNotifyEvent );
	HRESULT CreateCaptureBuffer( WAVEFORMATEX* pwfxInput, HANDLE hNotifyEvent );

public:
	CAudioIn();
	~CAudioIn();

	HRESULT Init( HWND hWnd, GUID* pDeviceGuid );
	HRESULT Free();

	inline DWORD GetCaptureBufferSize() { return m_dwCaptureBufferSize; };

	HRESULT StartOrStopRecord( BOOL bStartRecording, HANDLE hNotifyEvent, AUDIOCALLBACK func);

	HRESULT GrabCapturedData(void *obj);

} CAudioIn;

#endif AUDIOIN_H