#define INITGUID 
#include "AudioOut.h"

//-----------------------------------------------------------------------------
// Name: CAudioOut()
// Desc: Constructor
//-----------------------------------------------------------------------------
CAudioOut::CAudioOut()
{
	m_pDSNotify = NULL;
	m_pDS = NULL;
	m_pDSBPlay = NULL;

	// Default audio play out configuration
	memset(&m_wfxOutput, 0, sizeof(WAVEFORMATEXTENSIBLE));
	m_wfxOutput.Format.cbSize = sizeof (WAVEFORMATEXTENSIBLE);
	m_wfxOutput.Format.nChannels = 2;
	m_wfxOutput.Format.nSamplesPerSec = 48000;
	m_wfxOutput.Format.wBitsPerSample = 16; // Byte-aligned, must be multiple of 8.
	m_wfxOutput.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	m_wfxOutput.Format.nBlockAlign = 4;  // nChannels * nBitsPerSample / 8
	m_wfxOutput.Format.nAvgBytesPerSec = 192000; //nSamplesPerSecond * nBlockAlign
	m_wfxOutput.Samples.wValidBitsPerSample = m_wfxOutput.Format.wBitsPerSample;
	m_wfxOutput.Samples.wSamplesPerBlock = 0;
	m_wfxOutput.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
	m_wfxOutput.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

	// Initialize write position in playout buffer to 0.
	m_dwNextWriteOffset = 0;
}

//-----------------------------------------------------------------------------
// Name: ~CAudioOut()
// Desc: Destructor
//-----------------------------------------------------------------------------
CAudioOut::~CAudioOut()
{
}

//-----------------------------------------------------------------------------
// Name: Init()
// Desc: Initilizes DirectSound
//-----------------------------------------------------------------------------
HRESULT
CAudioOut::Init( HWND hWnd, GUID* pDeviceGuid )
{
    HRESULT hr;

    ZeroMemory( &m_aPosNotify, sizeof(DSBPOSITIONNOTIFY) * 
                               (NUM_REC_NOTIFICATIONS + 1) );
    m_dwAudioBufferSize = 0;
    m_dwNotifySize      = 0;

    // Initialize COM
	if( FAILED( hr = CoInitialize(NULL))) {
		MessageBox(GetDesktopWindow(), TEXT("CoInitialize Failed."), NULL, MB_OK);
		return hr;
	}

    // Create IDirectSound using the preferred audio device
	if( FAILED( hr = DirectSoundCreate8( pDeviceGuid, &m_pDS, NULL ) ) ) {
		MessageBox(GetDesktopWindow(), TEXT("DirectSoundCreate Failed."), NULL, MB_OK);
		return hr;
	}

	// Set cooperative level.
	m_pDS->SetCooperativeLevel(hWnd, DSSCL_NORMAL);

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Free()
// Desc: Releases DirectSound 
//-----------------------------------------------------------------------------
HRESULT 
CAudioOut::Free()
{

    // Release DirectSound interfaces
	SAFE_RELEASE( m_pDSBPlay );
    SAFE_RELEASE( m_pDSNotify );
	SAFE_RELEASE( m_pDS );

    // Release COM
    CoUninitialize();

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: InitNotifications()
// Desc: Inits the notifications on the capture buffer which are handled
//       in WinMain()
//-----------------------------------------------------------------------------
HRESULT 
CAudioOut::InitNotifications(HANDLE hNotifyEvent)
{
    HRESULT hr; 

    if( NULL == m_pDSBPlay )
        return E_FAIL;

    // Create a notification event, for when the sound stops playing
    if( FAILED( hr = m_pDSBPlay->QueryInterface( IID_IDirectSoundNotify, 
		(VOID**)&m_pDSNotify ) ) ) {
		MessageBox(GetDesktopWindow(), TEXT("QueryInterface Failed."), NULL, MB_OK);
		return hr;
	}

    // Setup the notification positions
    for( INT i = 0; i < NUM_REC_NOTIFICATIONS; i++ )
    {
        m_aPosNotify[i].dwOffset = (m_dwNotifySize * i) + m_dwNotifySize - 1;
        m_aPosNotify[i].hEventNotify = hNotifyEvent;             
    }
    
    // Tell DirectSound when to notify us. the notification will come in the from 
    // of signaled events that are handled in WinMain()
    if( FAILED( hr = m_pDSNotify->SetNotificationPositions( NUM_REC_NOTIFICATIONS, 
		m_aPosNotify ) ) ) {
		MessageBox(GetDesktopWindow(), TEXT("SetNotificationPositions Failed."), NULL, MB_OK);
		return hr;
	}

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CreatePlayBuffer()
// Desc: Creates a play buffer for provided format and sets up notifications 
//-----------------------------------------------------------------------------
HRESULT 
CAudioOut::CreatePlayBuffer( WAVEFORMATEXTENSIBLE* pwfxOutput,
							 HANDLE hNotifyEvent )
{
    HRESULT hr;
	LPDIRECTSOUNDBUFFER pDSB = NULL;
    DSBUFFERDESC dsbdesc;

    SAFE_RELEASE( m_pDSNotify );
    SAFE_RELEASE( m_pDSBPlay );

    // Set the notification size
    m_dwNotifySize = MAX( 1024, pwfxOutput->Format.nAvgBytesPerSec / 8 );
    m_dwNotifySize -= m_dwNotifySize % pwfxOutput->Format.nBlockAlign;   

    // Set the buffer sizes 
    m_dwPlayBufferSize = m_dwNotifySize * NUM_REC_NOTIFICATIONS;

	// printf("g_dwPlayBufferSize = %d\n", g_dwPlayBufferSize);

    SAFE_RELEASE( m_pDSNotify );
    SAFE_RELEASE( m_pDSBPlay );

    // Create the playout buffer
    ZeroMemory( &dsbdesc, sizeof(dsbdesc) );
    dsbdesc.dwSize        = sizeof(dsbdesc);
    dsbdesc.dwBufferBytes = m_dwPlayBufferSize;
	dsbdesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | 
		              DSBCAPS_STICKYFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
    dsbdesc.lpwfxFormat   = &(pwfxOutput->Format); // Set the format during creation

    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbdesc, 
                                               &pDSB, 
                                               NULL ) ) ) {
		MessageBox(GetDesktopWindow(), TEXT("CreatePlayBuffer Failed."), NULL, MB_OK);
		return hr;
	}

	// Get IDirectSoundBuffer8 interface.
    if( FAILED( hr = pDSB->QueryInterface( IID_IDirectSoundBuffer8, 
		(VOID**)&m_pDSBPlay ) ) ) {
		MessageBox(GetDesktopWindow(), TEXT("QueryInterface Failed."), NULL, MB_OK);
		return hr;
	}

	// With IDirectSoundBuffer8 interface, release original buffer pointer
	pDSB->Release();

	DWORD dwStatus = 0;
	m_pDSBPlay->GetStatus(&dwStatus);

	// Set starting play position
    m_dwNextPlayOffset = 0;
	m_pDSBPlay->SetCurrentPosition( m_dwNextPlayOffset );

	// Initialize the buffer notification points.
	if( FAILED( hr = InitNotifications( hNotifyEvent ) ) ) {
		MessageBox(GetDesktopWindow(), TEXT("InitNotifications Failed."), NULL, MB_OK);
		return hr;
	}

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: StartOrStopPlay()
// Desc: Starts or stops the play buffer from recording
//-----------------------------------------------------------------------------
HRESULT 
CAudioOut::StartOrStopPlay( BOOL bStartPlay, HANDLE hNotifyEvent )
{
    HRESULT hr;

    if( bStartPlay )
    {
        // Create a sound buffer, and tell the sound 
        // buffer to start playing   
		if( FAILED( hr = CreatePlayBuffer( &m_wfxOutput, hNotifyEvent ) ) ) {
			MessageBox(GetDesktopWindow(), TEXT("CreatePlayBuffer Failed."), NULL, MB_OK);
			return hr;
		}	

		if( FAILED( hr = m_pDSBPlay->Play( 0, 0, DSCBSTART_LOOPING ) ) ) {
			MessageBox(GetDesktopWindow(), TEXT("Start Failed."), NULL, MB_OK);
			return hr;
		}
    }
    else
    {
        // Stop the play out
        if( NULL == m_pDSBPlay )
            return S_OK;

        // Stop the buffer,
		if( FAILED( hr = m_pDSBPlay->Stop() ) ) {
			MessageBox(GetDesktopWindow(), TEXT("Stop Failed."), NULL, MB_OK);
			return hr;
		}
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: SendSoundData()
// Desc: Write PCM audio data to portion of direct sound buffer for playout
//-----------------------------------------------------------------------------
BOOL 
CAudioOut::SendSoundData( LPBYTE lpbSoundData,     // Sound data
						  DWORD dwSoundBytes )     // Size of block to copy.
{ 
	LPVOID  lpvPtr1; 
	DWORD dwBytes1; 
    LPVOID  lpvPtr2; 
    DWORD dwBytes2; 
    HRESULT hr; 
     
    // Obtain memory address of write block. This will be in two parts
    // if the block wraps around. 
    hr = m_pDSBPlay->Lock(m_dwNextWriteOffset, dwSoundBytes, &lpvPtr1, 
					      &dwBytes1, &lpvPtr2, &dwBytes2, 0); 
     
    // If the buffer was lost, restore and retry lock. 
    if (DSERR_BUFFERLOST == hr) { 
		m_pDSBPlay->Restore(); 
        hr = m_pDSBPlay->Lock(m_dwNextWriteOffset, dwSoundBytes, &lpvPtr1, 
			                  &dwBytes1, &lpvPtr2, &dwBytes2, 0); 
    }

    if (SUCCEEDED(hr)) {

        // Write to pointers. 
        CopyMemory(lpvPtr1, lpbSoundData, dwBytes1); 
        if (NULL != lpvPtr2) { 
          CopyMemory(lpvPtr2, lpbSoundData+dwBytes1, dwBytes2); 
        } 

		// Calculate next write offset in playout buffer
		m_dwNextWriteOffset += (dwBytes1 + dwBytes2); 
        m_dwNextWriteOffset %= m_dwPlayBufferSize; // Circular buffer

        // Release the buffer back to DirectSound.   
        hr = m_pDSBPlay->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2); 
        if (SUCCEEDED(hr)) { 
          // Success. 
          return TRUE; 
        } 
    } 
     
    // Lock, Unlock, or Restore failed. 
    return FALSE; 
}