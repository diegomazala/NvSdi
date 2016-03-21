#include "AudioIn.h"

//-----------------------------------------------------------------------------
// Name: CAudioIn()
// Desc: Constructor
//-----------------------------------------------------------------------------
CAudioIn::CAudioIn()
{
	m_pDSNotify = NULL;
	m_pDSCapture = NULL;
	m_pDSBCapture = NULL;

	// Default audio play out configuration
#if 0
	memset(&m_wfxInput, 0, sizeof(WAVEFORMATEXTENSIBLE));
	m_wfxInput.Format.cbSize = sizeof (WAVEFORMATEXTENSIBLE);
	m_wfxInput.Format.nChannels = 2;
	m_wfxInput.Format.nSamplesPerSec = 48000;
	m_wfxInput.Format.wBitsPerSample = 16; // Byte-aligned, must be multiple of 8.
	m_wfxInput.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	m_wfxInput.Format.nBlockAlign = 4;  // nChannels * nBitsPerSample / 8
	m_wfxInput.Format.nAvgBytesPerSec = 192000; //nSamplesPerSecond * nBlockAlign
	m_wfxInput.Samples.wValidBitsPerSample = m_wfxInput.Format.wBitsPerSample;
	m_wfxInput.Samples.wSamplesPerBlock = 0;
	m_wfxInput.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
	m_wfxInput.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
#else
	memset(&m_wfxInput, 0, sizeof(WAVEFORMATEX));
	m_wfxInput.cbSize = sizeof (WAVEFORMATEX);
	m_wfxInput.nChannels = 2;
	m_wfxInput.nSamplesPerSec = 48000;
	m_wfxInput.wBitsPerSample = 16; // Byte-aligned, must be multiple of 8.
	m_wfxInput.wFormatTag = WAVE_FORMAT_PCM;
	m_wfxInput.nBlockAlign = 4;  // nChannels * nBitsPerSample / 8
	m_wfxInput.nAvgBytesPerSec = 192000; //nSamplesPerSecond * nBlockAlign
#endif

	// Initialize write position in playout buffer to 0.
	m_dwNextWriteOffset = 0;
}

//-----------------------------------------------------------------------------
// Name: ~CAudioIn()
// Desc: Destructor
//-----------------------------------------------------------------------------
CAudioIn::~CAudioIn()
{
}

//-----------------------------------------------------------------------------
// Name: Init()
// Desc: Initilizes DirectSound
//-----------------------------------------------------------------------------
HRESULT 
CAudioIn::Init( HWND hDlg, GUID* pDeviceGuid )
{
    HRESULT hr;

    ZeroMemory( &m_aPosNotify, sizeof(DSBPOSITIONNOTIFY) * 
                               (NUM_REC_NOTIFICATIONS + 1) );
    m_dwCaptureBufferSize = 0;
    m_dwNotifySize        = 0;

    // Initialize COM
	if( FAILED( hr = CoInitialize(NULL) ) ) {
        TRACE_ERR_MSGBOX( TEXT("CoInitialize"));
		return hr;
	}

    // Create IDirectSoundCapture using the preferred capture device
	if( FAILED( hr = DirectSoundCaptureCreate( pDeviceGuid, &m_pDSCapture, NULL ) ) ) {
        TRACE_ERR_MSGBOX( TEXT("DirectSoundCaptureCreate"));
		return hr;
	}

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Free()
// Desc: Releases DirectSound 
//-----------------------------------------------------------------------------
HRESULT 
CAudioIn::Free()
{

    // Release DirectSound interfaces
    SAFE_RELEASE( m_pDSNotify );
    SAFE_RELEASE( m_pDSBCapture );
    SAFE_RELEASE( m_pDSCapture ); 

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
CAudioIn::InitNotifications( HANDLE hNotifyEvent )
{
    HRESULT hr; 

    if( NULL == m_pDSBCapture )
        return E_FAIL;

    // Create a notification event, for when the sound stops playing
    if( FAILED( hr = m_pDSBCapture->QueryInterface( IID_IDirectSoundNotify, 
		        (VOID**)&m_pDSNotify ) ) ) {
        TRACE_ERR_MSGBOX( TEXT("QueryInterface"));
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
        TRACE_ERR_MSGBOX( TEXT("SetNotificationPositions"));
		return hr;
	}

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: CreateCaptureBuffer()
// Desc: Creates a capture buffer and sets the format 
//-----------------------------------------------------------------------------
HRESULT 
CAudioIn::CreateCaptureBuffer( WAVEFORMATEX* pwfxInput, HANDLE hNotifyEvent )
{
    HRESULT hr;
    DSCBUFFERDESC dscbd;

    SAFE_RELEASE( m_pDSNotify );
    SAFE_RELEASE( m_pDSBCapture );

    // Set the notification size
    m_dwNotifySize = MAX( 1024, pwfxInput->nAvgBytesPerSec / 8 );
    m_dwNotifySize -= m_dwNotifySize % pwfxInput->nBlockAlign;   

    // Set the buffer sizes 
    m_dwCaptureBufferSize = m_dwNotifySize * NUM_REC_NOTIFICATIONS;

    SAFE_RELEASE( m_pDSNotify );
    SAFE_RELEASE( m_pDSBCapture );

    // Create the capture buffer
    ZeroMemory( &dscbd, sizeof(dscbd) );
    dscbd.dwSize        = sizeof(dscbd);
    dscbd.dwBufferBytes = m_dwCaptureBufferSize;
    dscbd.lpwfxFormat   = pwfxInput; // Set the format during creatation

    if( FAILED( hr = m_pDSCapture->CreateCaptureBuffer( &dscbd, 
                                                        &m_pDSBCapture, 
														NULL ) ) ) {
        TRACE_ERR_MSGBOX( TEXT("CreateCaptureBuffer"));
		return hr;
	}

    m_dwNextCaptureOffset = 0;

	if( FAILED( hr = InitNotifications( hNotifyEvent ) ) ) {
        TRACE_ERR_MSGBOX( TEXT("InitNotifications"));
		return hr;
	}

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: StartOrStopRecord()
// Desc: Starts or stops the capture buffer from recording
//-----------------------------------------------------------------------------
HRESULT 
CAudioIn::StartOrStopRecord( BOOL bStartRecording, HANDLE hNotifyEvent, AUDIOCALLBACK func)
{
    HRESULT hr;

    if( bStartRecording )
    {
        // Create a capture buffer, and tell the capture 
        // buffer to start recording 
#if 0
		if( FAILED( hr = CreateCaptureBuffer( &m_wfxInput.Format, hNotifyEvent ) ) ) {
            TRACE_ERR_MSGBOX( TEXT("CreateCaptureBuffer"));
			return hr;
		}
#else
		if( FAILED( hr = CreateCaptureBuffer( &m_wfxInput, hNotifyEvent ) ) ) {
            TRACE_ERR_MSGBOX( TEXT("CreateCaptureBuffer"));
			return hr;
		}
#endif

		if( FAILED( hr = m_pDSBCapture->Start( DSCBSTART_LOOPING ) ) ) {
            TRACE_ERR_MSGBOX( TEXT("Start"));
			return hr;
		}
    }
    else
    {
        // Stop the capture and read any data that 
        // was not caught by a notification
        if( NULL == m_pDSBCapture )
            return S_OK;

        // Stop the buffer, and read any data that was not 
        // caught by a notification
		if( FAILED( hr = m_pDSBCapture->Stop() ) ) {
            TRACE_ERR_MSGBOX( TEXT("Stop"));
			return hr;
		}

      //   if( FAILED( hr = RecordCapturedData() ) ) {
      //       TRACE_ERR_MSGBOX( TEXT("RecordCapturedData"));
      //       return hr;
	  //   }
    }

	// Set callback function
	m_func = func;

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: GrabCapturedData()
// Desc: Copies data from the capture buffer. 
//-----------------------------------------------------------------------------
HRESULT 
CAudioIn::GrabCapturedData(void *obj) 
{
    HRESULT hr;
    VOID*   pbCaptureData    = NULL;
    DWORD   dwCaptureLength;
    VOID*   pbCaptureData2   = NULL;
    DWORD   dwCaptureLength2;
    DWORD   dwReadPos;
    DWORD   dwCapturePos;
    LONG    lLockSize;

	if( NULL == m_pDSBCapture )
        return S_FALSE;

	if( FAILED( hr = m_pDSBCapture->GetCurrentPosition( &dwCapturePos, &dwReadPos ) ) ) {
        TRACE_ERR_MSGBOX( TEXT("GetCurrentPosition") );
		return hr;
	}

#if 0
	WAVEFORMATEX format;
	DWORD bytesWritten;
	if ( FAILED( hr = g_pDSBCapture->GetFormat( &format, sizeof(WAVEFORMATEX), &bytesWritten ) ) ) {
		TRACE_ERR_MSGBOX( TEXT("GetFormat") );
		return hr;
	}

	//fprintf(stderr, "capture audio\n");
#endif
    lLockSize = dwReadPos - m_dwNextCaptureOffset;
    if( lLockSize < 0 )
        lLockSize += m_dwCaptureBufferSize;

    // Block align lock size so that we are always write on a boundary
    lLockSize -= (lLockSize % m_dwNotifySize);

    if( lLockSize == 0 )
        return S_FALSE;

    // Lock the capture buffer down
    if( FAILED( hr = m_pDSBCapture->Lock( m_dwNextCaptureOffset, lLockSize, 
                                          &pbCaptureData, &dwCaptureLength, 
										  &pbCaptureData2, &dwCaptureLength2, 0L ) ) ) {
        TRACE_ERR_MSGBOX( TEXT("Lock") );
		return hr;
	}

	if (pbCaptureData != NULL) {
#if 0
		BufferAndFormatAudio(dwCaptureLength, pbCaptureData);
#else
		(m_func)(dwCaptureLength, pbCaptureData, obj);
#endif
	}

    // Move the capture offset along
    m_dwNextCaptureOffset += dwCaptureLength; 
    m_dwNextCaptureOffset %= m_dwCaptureBufferSize; // Circular buffer

	if( pbCaptureData2 != NULL ) {

        // Move the capture offset along
        m_dwNextCaptureOffset += dwCaptureLength2; 
        m_dwNextCaptureOffset %= m_dwCaptureBufferSize; // Circular buffer
    }

    // Unlock the capture buffer
    m_pDSBCapture->Unlock( pbCaptureData,  dwCaptureLength, 
                           pbCaptureData2, dwCaptureLength2 );


    return S_OK;
}

