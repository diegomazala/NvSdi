#ifndef AUDIOOUT_H
#define AUDIOOUT_H

#include <windows.h>
#define  DIRECTSOUND_VERSION 0x0800

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

#define NUM_REC_NOTIFICATIONS  16

#define MAX(a,b)        ( (a) > (b) ? (a) : (b) )

typedef class CAudioOut
{
private:
	// PCM audio with more than 2 channels, greater than 16 bits per
	// sample and more than 44,100 samples per second must utilize the 
	// WAVEFORMATEXTENSIBLE structure.
	WAVEFORMATEXTENSIBLE       m_wfxOutput;

	LPDIRECTSOUNDNOTIFY        m_pDSNotify;
	DWORD                      m_dwNextPlayOffset;
	DWORD                      m_dwPlayBufferSize;
	LPDIRECTSOUND8             m_pDS;
	LPDIRECTSOUNDBUFFER8       m_pDSBPlay;
	DWORD                      m_dwAudioBufferSize; 
	DWORD                      m_dwNotifySize;
	DSBPOSITIONNOTIFY          m_aPosNotify[ NUM_REC_NOTIFICATIONS + 1 ]; 

	DWORD                      m_dwNextWriteOffset;

	HRESULT InitNotifications( HANDLE hNotifyEvent );
	HRESULT CreatePlayBuffer( WAVEFORMATEXTENSIBLE* pwfxOutput, HANDLE hNotifyEvent );

public:
	CAudioOut();
	~CAudioOut();

	HRESULT Init( HWND hWnd, GUID* pDeviceGuid );
	HRESULT Free();

	HRESULT StartOrStopPlay( BOOL bStartPlay, HANDLE hNotifyEvent);

	BOOL SendSoundData( LPBYTE lpbSoundData, DWORD dwSoundBytes ); 

} CAudioOut;

#endif AUDIOOUT_H