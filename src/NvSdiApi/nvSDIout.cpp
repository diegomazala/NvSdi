
#include "nvSDIutil.h"
#include "nvSDIout.h"
#include "glExtensions.h"

CNvSDIout::CNvSDIout()
{
#ifdef USE_NVAPI
	m_vioHandle = NULL;
#else
	m_hGVO = NULL;
#endif
}

CNvSDIout::~CNvSDIout()
{
}

//
// Setup the video device
//
HRESULT
CNvSDIout::Init(Options *options, CNvSDIoutGpu *SDIoutGpu)
{
#ifdef USE_NVAPI
	NvAPI_Status l_ret;


	if(SDIoutGpu == NULL || SDIoutGpu->isSDIoutput() != true)
	{
		//If the application didn't scan the system topology and selected a
		// gpu for output the the scanning has to be done here.
		// Initialize NVAPI
		if (NvAPI_Initialize() != NVAPI_OK) {
			return E_FAIL;
		}

		// Query available VIO topologies.  
		// Fail if there are no VIO topologies or devices available.
		NVVIOTOPOLOGY l_vioTopologies;
		memset(&l_vioTopologies, 0, sizeof(l_vioTopologies));
		l_vioTopologies.version = NVVIOTOPOLOGY_VER;

		if (NvAPI_VIO_QueryTopology(&l_vioTopologies) != NVAPI_OK) {
			return E_FAIL;
		}

		if (l_vioTopologies.vioTotalDeviceCount == 0) {
			return E_FAIL;
		}

		// Cycle through all SDI topologies looking for the first
		// available SDI output device topology.
		BOOL l_bFound = FALSE;
		unsigned int i = 0;
		NVVIOCAPS l_vioCaps;
		while ((i < l_vioTopologies.vioTotalDeviceCount) && (!l_bFound)) {

			// Get video I/O capabilities for current video I/O target.
			memset(&l_vioCaps, 0, sizeof(l_vioCaps));
			l_vioCaps.version = NVVIOCAPS_VER;
			if (NvAPI_VIO_GetCapabilities(l_vioTopologies.vioTarget[i].hVioHandle, 
										  &l_vioCaps) != NVAPI_OK) {
				MessageBox(NULL, "Video I/O Unsupported.", "Error", MB_OK);
				return E_FAIL;
			}

			// If video output device found, save VIO handle and set flag.
			if (l_vioCaps.adapterCaps & NVVIOCAPS_VIDOUT_SDI) {
				m_vioHandle = l_vioTopologies.vioTarget[i].hVioHandle;
				l_bFound = TRUE;
			} else {
				i++;
			}
		} // while i < vioTotalDeviceCount

		// If no video output device found, return error.
		if (!l_bFound) {
			MessageBox(NULL, "No SDI video output devices found.", "Error", MB_OK);
			return E_FAIL;
		}
	}
	else
	{
		m_vioHandle = SDIoutGpu->getVioHandle();
	}
	// Open the SDI device
	l_ret = NvAPI_VIO_Open(m_vioHandle, NVVIOCLASS_SDI, NVVIOOWNERTYPE_APPLICATION);
	if (l_ret != NVAPI_OK) {
		return E_FAIL;
	}

	// Configure the SDI GVO device
	NVVIOCONFIG_V1 l_vioConfig;
	memset(&l_vioConfig, 0, sizeof(l_vioConfig));
	l_vioConfig.version = NVVIOCONFIG_VER1;
	l_vioConfig.fields = 0;
	l_vioConfig.nvvioConfigType = NVVIOCONFIGTYPE_OUT;

	// Configure signal and data format
	l_vioConfig.fields = NVVIOCONFIG_SIGNALFORMAT;
	l_vioConfig.vioConfig.outConfig.signalFormat = options->videoFormat;
	l_vioConfig.fields |= NVVIOCONFIG_DATAFORMAT; 
	l_vioConfig.vioConfig.outConfig.dataFormat = options->dataFormat;

	// Send 8-bit colorbars as YCrCb on both channels, overriding any user settings.
	if ((options->testPattern == TEST_PATTERN_COLORBARS8_75) ||
		(options->testPattern == TEST_PATTERN_COLORBARS8_100) ||
		(options->testPattern == TEST_PATTERN_YCRCB_COLORBARS)) {
		l_vioConfig.vioConfig.outConfig.dataFormat = NVVIODATAFORMAT_DUAL_X8X8X8_TO_DUAL_422_PASSTHRU;
	}

	// Send 10-bit colorbars as 10-bit YCrCb on a single channel, overriding any user settings.
	if ((options->testPattern == TEST_PATTERN_COLORBARS10_75) ||
		(options->testPattern == TEST_PATTERN_COLORBARS10_100)) {
		l_vioConfig.vioConfig.outConfig.dataFormat = NVVIODATAFORMAT_X10X10X10_444_PASSTHRU;
	}

	// Send 8-bit ramp as 8-bit YCrCbA4224
	if (options->testPattern == TEST_PATTERN_RAMP8) {
		l_vioConfig.vioConfig.outConfig.dataFormat = NVVIODATAFORMAT_R8G8B8A8_TO_YCRCBA4224;
	}

	// Send 16-bit ramp as 16-bit YCrCb444
	if (options->testPattern == TEST_PATTERN_RAMP16) {
		l_vioConfig.vioConfig.outConfig.dataFormat = NVVIODATAFORMAT_R10G10B10_TO_YCRCB422;
	}

	// Set sync source if specified.  The sync source must be set
	// before a valid sync can be detected.
	if (options->syncEnable) {
		l_vioConfig.vioConfig.outConfig.syncEnable = options->syncEnable;
		l_vioConfig.vioConfig.outConfig.syncSource = options->syncSource;

		switch(options->syncSource) {
			case NVVIOSYNCSOURCE_SDISYNC:
				l_vioConfig.fields |= NVVIOCONFIG_SYNCSOURCEENABLE;
				break;
			case NVVIOSYNCSOURCE_COMPSYNC:
				l_vioConfig.vioConfig.outConfig.compositeSyncType = NVVIOCOMPSYNCTYPE_AUTO;
				l_vioConfig.fields |= (NVVIOCONFIG_SYNCSOURCEENABLE | NVVIOCONFIG_COMPOSITESYNCTYPE);
				break;
		} // switch
	}

	// Colorspace Conversion
	if (options->cscEnable) {
		l_vioConfig.fields |= NVVIOCONFIG_CSCOVERRIDE;
		l_vioConfig.vioConfig.outConfig.cscOverride = TRUE;
		l_vioConfig.fields |= NVVIOCONFIG_COLORCONVERSION;

		// Offset
		l_vioConfig.vioConfig.outConfig.colorConversion.colorOffset[0] = options->cscOffset[0];
		l_vioConfig.vioConfig.outConfig.colorConversion.colorOffset[1] = options->cscOffset[1];
		l_vioConfig.vioConfig.outConfig.colorConversion.colorOffset[2] = options->cscOffset[2];

		// Scale
		l_vioConfig.vioConfig.outConfig.colorConversion.colorScale[0] = options->cscScale[0];
	    l_vioConfig.vioConfig.outConfig.colorConversion.colorScale[1] = options->cscScale[1];
	    l_vioConfig.vioConfig.outConfig.colorConversion.colorScale[2] = options->cscScale[2];
		l_vioConfig.vioConfig.outConfig.colorConversion.compositeSafe = TRUE;

		// Matrix
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[0][0] = options->cscMatrix[0][0]; 
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[0][1] = options->cscMatrix[0][1]; 
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[0][2] = options->cscMatrix[0][2];
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[1][0] = options->cscMatrix[1][0];  
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[1][1] = options->cscMatrix[1][1];   
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[1][2] = options->cscMatrix[1][2];
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[2][0] = options->cscMatrix[2][0];  
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[2][1] = options->cscMatrix[2][1]; 
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[2][2] = options->cscMatrix[2][2]; 

	} else {
		l_vioConfig.fields |= NVVIOCONFIG_CSCOVERRIDE;
		l_vioConfig.vioConfig.outConfig.cscOverride = FALSE;
	} // if

	// Gamma correction
	l_vioConfig.vioConfig.outConfig.gammaCorrection.version = NVVIOGAMMACORRECTION_VER;
	l_vioConfig.vioConfig.outConfig.gammaCorrection.fGammaValueR = options->gamma[0];
	l_vioConfig.vioConfig.outConfig.gammaCorrection.fGammaValueG = options->gamma[1];
	l_vioConfig.vioConfig.outConfig.gammaCorrection.fGammaValueB = options->gamma[2];
	l_vioConfig.fields |= NVVIOCONFIG_GAMMACORRECTION;

	// Set flip queue length
	l_vioConfig.fields |= NVVIOCONFIG_FLIPQUEUELENGTH;
	l_vioConfig.vioConfig.outConfig.flipQueueLength = options->flipQueueLength;

	// Set full color range
	l_vioConfig.fields |= NVVIOCONFIG_FULL_COLOR_RANGE;
	l_vioConfig.vioConfig.outConfig.enableFullColorRange = TRUE;

	// Set configuration.
	l_ret = NvAPI_VIO_SetConfig(m_vioHandle, (NVVIOCONFIG *)&l_vioConfig);
	if (l_ret != NVAPI_OK) {
		return E_FAIL;
	}

	// Get configuration to make sure it matches what is set.
	//memset(&l_vioConfig, 0, sizeof(l_vioConfig));
	//l_vioConfig.version = NVVIOCONFIG_VER;
	//l_vioConfig.fields = NVVIOCONFIG_ALLFIELDS;
	//l_ret = NvAPI_VIO_GetConfig(m_vioHandle, &l_vioConfig);
	//if (l_ret != NVAPI_OK) {
	//	return E_FAIL;
	//}

	//
	// Setup external sync here. 
	//
    // Configure external sync parameters
 	if (options->syncEnable) {
		NvU32 l_wait;
		NVVIOSTATUS l_vioStatus;
		l_vioConfig.fields = 0;	// reset fields

		// Trigger redetection of sync format
		if (NvAPI_VIO_SyncFormatDetect(m_vioHandle, &l_wait) != NVAPI_OK) {
			return E_FAIL;
		}

		// Wait for sync detection to complete 
		Sleep(l_wait);

		// Get sync signal format
		l_vioStatus.version = NVVIOSTATUS_VER;
		if (NvAPI_VIO_Status(m_vioHandle, &l_vioStatus) != NVAPI_OK) {
			return E_FAIL;
		}

		// Verify that incoming sync signal is compatible outgoing video signal
		if (!options->frameLock) {
			if (l_vioStatus.vioStatus.outStatus.syncFormat !=
				l_vioConfig.vioConfig.outConfig.signalFormat) {
				MessageBox(NULL, "Incoming sync does not match outgoing video signal.", "Error", MB_OK);
				return E_FAIL;
			}
			l_vioConfig.vioConfig.outConfig.frameLockEnable = FALSE;	
			l_vioConfig.fields |= NVVIOCONFIG_FRAMELOCKENABLE;
		} else {  // Framelock Case
			NvU32 l_compatible;
			if (NvAPI_VIO_IsFrameLockModeCompatible(m_vioHandle,
													l_vioStatus.vioStatus.outStatus.syncFormat,
													l_vioConfig.vioConfig.outConfig.signalFormat,
													&l_compatible) != NVAPI_OK) {
				return E_FAIL;
			}

			if (l_compatible) {
				l_vioConfig.vioConfig.outConfig.frameLockEnable = TRUE;	
				l_vioConfig.fields |= NVVIOCONFIG_FRAMELOCKENABLE;
			} else {
				MessageBox(NULL, "Incoming sync not compatible with outgoing video format.", "Error", MB_OK);
				return E_FAIL;
			}
		}

		l_vioConfig.vioConfig.outConfig.syncEnable = l_vioStatus.vioStatus.outStatus.syncEnable;
		l_vioConfig.vioConfig.outConfig.syncSource = l_vioStatus.vioStatus.outStatus.syncSource;

		switch(l_vioStatus.vioStatus.outStatus.syncSource) {
			case NVVIOSYNCSOURCE_SDISYNC:
				l_vioConfig.fields |= NVVIOCONFIG_SYNCSOURCEENABLE;
				break;
			case NVVIOSYNCSOURCE_COMPSYNC:
				l_vioConfig.vioConfig.outConfig.compositeSyncType = NVVIOCOMPSYNCTYPE_AUTO;
				l_vioConfig.fields |= (NVVIOCONFIG_SYNCSOURCEENABLE | NVVIOCONFIG_COMPOSITESYNCTYPE);
				break;
		} // switch

		// Sync delay
		NVVIOSYNCDELAY l_vioSyncDelay;
		memset(&l_vioSyncDelay, 0, sizeof(l_vioSyncDelay));
		l_vioSyncDelay.version = NVVIOSYNCDELAY_VER;
		l_vioSyncDelay.horizontalDelay = options->hDelay;
		l_vioSyncDelay.verticalDelay = options->vDelay;
		l_vioConfig.fields |= NVVIOCONFIG_SYNCDELAY;
		l_vioConfig.vioConfig.outConfig.syncDelay = l_vioSyncDelay;

		// Setup external sync
		if (NvAPI_VIO_SetConfig(m_vioHandle, (NVVIOCONFIG *)&l_vioConfig) != NVAPI_OK) {
			return E_FAIL;
		}
	}

	// Get video configuration.
	NVVIOSIGNALFORMATDETAIL l_vioSignalFormatDetail;
	memset(&l_vioSignalFormatDetail, 0, sizeof(l_vioSignalFormatDetail));

	ULONG l_vioSignalFormatIndex = (ULONG)NVVIOSIGNALFORMAT_NONE; 

	// Enumerate all sigal formats until we find the one we are looking
	// for, the enueration ends, or there is an error.
	while(1) { 
		l_ret = NvAPI_VIO_EnumSignalFormats(m_vioHandle, 
		 								    l_vioSignalFormatIndex, 
										    &l_vioSignalFormatDetail); 
        if (l_ret == NVAPI_END_ENUMERATION || l_ret != NVAPI_OK)  { 
			return E_FAIL; 
		} // if

		// We found the signal format that we were looking for so break.
		if (l_vioSignalFormatDetail.signalFormat == 
			l_vioConfig.vioConfig.outConfig.signalFormat) { 
			break;
		} // if

		l_vioSignalFormatIndex++; 
	} // while

	// Set frame rate.  In the case of an interlaced signal format,
	// divide by 2.0 to calculate the frame rate.
	if (l_vioSignalFormatDetail.videoMode.interlaceMode == NVVIOINTERLACEMODE_INTERLACE) 
		m_frameRate = l_vioSignalFormatDetail.videoMode.fFrameRate / 2.0f;
	else
		m_frameRate = l_vioSignalFormatDetail.videoMode.fFrameRate;

	// Set width and height
	m_videoWidth = l_vioSignalFormatDetail.videoMode.horizontalPixels;
	m_videoHeight = l_vioSignalFormatDetail.videoMode.verticalLines;

	// Set interlace flag.
	if ((l_vioSignalFormatDetail.videoMode.interlaceMode == NVVIOINTERLACEMODE_INTERLACE)  ||
	    (l_vioSignalFormatDetail.videoMode.interlaceMode == NVVIOINTERLACEMODE_PSF)) {
		m_bInterlaced = TRUE;
	} else {
		m_bInterlaced = FALSE;
	}
#else
	const UINT nAdapterNumber = 1;
	NVRESULT l_nvResult = NV_OK;

	NVGVOCAPS l_gvoCaps = {0};
	l_gvoCaps.cbSize = sizeof(NVGVOCAPS);
	if ( NvGvoCaps(nAdapterNumber, NULL, &l_gvoCaps) != NV_OK ) {
		return E_FAIL;
	}

	// Open GVO device
	if ( NvGvoOpen(nAdapterNumber, NULL, NVGVOCLASS_SDI, NVGVO_O_WRITE_EXCLUSIVE, &m_hGVO) != NV_OK ) {
		return E_FAIL;
	}

	// Configure GVO device
	NVGVOCONFIG l_gvoConfig;
	l_gvoConfig.cbSize = sizeof(NVGVOCONFIG);
	l_gvoConfig.dwFields = 0;

	// Configure signal and data format
	l_gvoConfig.dwFields = NVGVOCONFIG_SIGNALFORMAT; 
	l_gvoConfig.signalFormat = options->videoFormat;
	l_gvoConfig.dwFields |= NVGVOCONFIG_DATAFORMAT; 
	l_gvoConfig.dataFormat = options->dataFormat;

	// Send 8-bit colorbars as YCrCb on both channels, overriding any user settings.
	if ((options->testPattern == TEST_PATTERN_COLORBARS8_75) ||
		(options->testPattern == TEST_PATTERN_COLORBARS8_100) ||
		(options->testPattern == TEST_PATTERN_YCRCB_COLORBARS)) {
		l_gvoConfig.dataFormat = NVGVODATAFORMAT_DUAL_Y8CR8CB8_TO_DUAL_YCRCB422;
	}

	// Send 10-bit colorbars as 10-bit YCrCb on a single channel, overriding any user settings.
	if ((options->testPattern == TEST_PATTERN_COLORBARS10_75) ||
		(options->testPattern == TEST_PATTERN_COLORBARS10_100)) {
		l_gvoConfig.dataFormat = NVGVODATAFORMAT_Y10CR10CB10_TO_YCRCB444;
	}

	// Send 8-bit ramp as 8-bit YCrCbA4224
	if (options->testPattern == TEST_PATTERN_RAMP8) {
		l_gvoConfig.dataFormat = NVGVODATAFORMAT_R8G8B8A8_TO_YCRCBA4224;
	}

	// Send 16-bit ramp as 16-bit YCrCb444
	if (options->testPattern == TEST_PATTERN_RAMP16) {
		l_gvoConfig.dataFormat = NVGVODATAFORMAT_R10G10B10_TO_YCRCB444;
	}

	// Set sync source if specified.  On Quadro FX SDI II, the sync source must be set
	// before a valid sync can be detected.
	if (options->syncEnable) {
		l_gvoConfig.syncEnable = options->syncEnable;
		l_gvoConfig.syncSource = options->syncSource;

		switch(options->syncSource) {
			case NVGVOSYNCSOURCE_SDISYNC:
				l_gvoConfig.dwFields |= NVGVOCONFIG_SYNCSOURCEENABLE;
				break;
			case NVGVOSYNCSOURCE_COMPSYNC:
				l_gvoConfig.compositeSyncType = NVGVOCOMPSYNCTYPE_AUTO;
				l_gvoConfig.dwFields |= (NVGVOCONFIG_SYNCSOURCEENABLE | NVGVOCONFIG_COMPOSITESYNCTYPE);
				break;
		} // switch
	}

	// Colorspace Conversion
	if (options->cscEnable) {
		l_gvoConfig.dwFields |= NVGVOCONFIG_CSCOVERRIDE;
		l_gvoConfig.bCSCOverride = TRUE;
		l_gvoConfig.dwFields |= NVGVOCONFIG_COLORCONVERSION;

		// Offset
		l_gvoConfig.colorConversion.colorOffset[0] = options->cscOffset[0];
		l_gvoConfig.colorConversion.colorOffset[1] = options->cscOffset[1];
		l_gvoConfig.colorConversion.colorOffset[2] = options->cscOffset[2];

		// Scale
		l_gvoConfig.colorConversion.colorScale[0] = options->cscScale[0];
	    l_gvoConfig.colorConversion.colorScale[1] = options->cscScale[1];
	    l_gvoConfig.colorConversion.colorScale[2] = options->cscScale[2];
		l_gvoConfig.colorConversion.bCompositeSafe = TRUE;

		// Matrix
		l_gvoConfig.colorConversion.colorMatrix[0][0] = options->cscMatrix[0][0]; 
		l_gvoConfig.colorConversion.colorMatrix[0][1] = options->cscMatrix[0][1]; 
		l_gvoConfig.colorConversion.colorMatrix[0][2] = options->cscMatrix[0][2];
		l_gvoConfig.colorConversion.colorMatrix[1][0] = options->cscMatrix[1][0];  
		l_gvoConfig.colorConversion.colorMatrix[1][1] = options->cscMatrix[1][1];   
		l_gvoConfig.colorConversion.colorMatrix[1][2] = options->cscMatrix[1][2];
		l_gvoConfig.colorConversion.colorMatrix[2][0] = options->cscMatrix[2][0];  
		l_gvoConfig.colorConversion.colorMatrix[2][1] = options->cscMatrix[2][1]; 
		l_gvoConfig.colorConversion.colorMatrix[2][2] = options->cscMatrix[2][2]; 

	} else {
		l_gvoConfig.dwFields |= NVGVOCONFIG_CSCOVERRIDE;
		l_gvoConfig.bCSCOverride = FALSE;
	} // if

	// Gamma correction
#if 0
	l_gvoConfig.gammaCorrection.gammaRamp8.cbSize = sizeof(NVGAMMARAMP10);
	l_gvoConfig.fGammaValueR = options->gamma[0];
	l_gvoConfig.fGammaValueG = options->gamma[1];
	l_gvoConfig.fGammaValueB = options->gamma[2];
	l_gvoConfig.dwFields |= NVGVOCONFIG_GAMMACORRECTION;
#endif

	// Set flip queue length
	l_gvoConfig.dwFields |= NVGVOCONFIG_FLIPQUEUELENGTH;
	l_gvoConfig.dwFlipQueueLength = options->flipQueueLength;
#if 1
	// Set full color range
	l_gvoConfig.dwFields |= NVGVOCONFIG_FULL_COLOR_RANGE;
	l_gvoConfig.bEnableFullColorRange = TRUE;
#endif
	NVGVOCHECK(NvGvoConfigSet(m_hGVO, &l_gvoConfig));

	//
	// Setup external sync here.  On Quadro FX SDI II, the sync source must
	// be set before a valid sync can be detected.
	//
    // Configure external sync parameters
	if (options->syncEnable) {
		DWORD l_dwWait;
		NVGVOSTATUS l_gvoStatus;
		l_gvoConfig.dwFields = 0;	// reset fields

		// Trigger redetection of sync format
		l_nvResult = NvGvoSyncFormatDetect(m_hGVO, &l_dwWait);

		// Wait for sync detection to complete 
		Sleep(l_dwWait);

		// Get sync signal format
		l_gvoStatus.cbSize = sizeof(NVGVOSTATUS);
		l_nvResult = NvGvoStatus(m_hGVO, &l_gvoStatus);

		// Verify that incoming sync signal is compatible outgoing video signal
		if (!options->frameLock) {
			if (l_gvoStatus.syncFormat != l_gvoConfig.signalFormat) {
				MessageBox(NULL, "Incoming sync does not match outgoing video signal.", "Error", MB_OK);
				return E_FAIL;
			}
			l_gvoConfig.frameLockEnable = FALSE;	
			l_gvoConfig.dwFields |= NVGVOCONFIG_FRAMELOCKENABLE;
		} else {  // Framelock Case
			BOOL l_bCompatible;
			l_nvResult = NvGvoIsFrameLockModeCompatible(m_hGVO,
														l_gvoStatus.syncFormat,
														l_gvoConfig.signalFormat,
														&l_bCompatible);
			if (l_bCompatible) {
				l_gvoConfig.frameLockEnable = TRUE;	
				l_gvoConfig.dwFields |= NVGVOCONFIG_FRAMELOCKENABLE;
			} else {
				MessageBox(NULL, "Incoming sync not compatible with outgoing video format.", "Error", MB_OK);
				return E_FAIL;
			}
		}

		l_gvoConfig.syncEnable = l_gvoStatus.syncEnable;
		l_gvoConfig.syncSource = l_gvoStatus.syncSource;

		switch(l_gvoStatus.syncSource) {
			case NVGVOSYNCSOURCE_SDISYNC:
				l_gvoConfig.dwFields |= NVGVOCONFIG_SYNCSOURCEENABLE;
				break;
			case NVGVOSYNCSOURCE_COMPSYNC:
				l_gvoConfig.compositeSyncType = NVGVOCOMPSYNCTYPE_AUTO;
				l_gvoConfig.dwFields |= (NVGVOCONFIG_SYNCSOURCEENABLE | NVGVOCONFIG_COMPOSITESYNCTYPE);
				break;
		} // switch

		// Sync delay
		NVGVOSYNCDELAY l_gvoSyncDelay;
		memset(&l_gvoSyncDelay, 0, sizeof(l_gvoSyncDelay));
		l_gvoSyncDelay.wHorizontalDelay = options->hDelay;
		l_gvoSyncDelay.wVerticalDelay = options->vDelay;
		l_gvoConfig.dwFields |= NVGVOCONFIG_SYNCDELAY;
		l_gvoConfig.syncDelay = l_gvoSyncDelay;

		// Setup external sync
		NVGVOCHECK(NvGvoConfigSet(m_hGVO, &l_gvoConfig));
	}

	// Get video configuration.
	NVGVOSIGNALFORMATDETAIL l_signalFormatDetail = {0};
	l_signalFormatDetail.cbSize = sizeof(l_signalFormatDetail);
	if (NvGvoEnumSignalFormats(m_hGVO, l_gvoConfig.signalFormat,
		TRUE, &l_signalFormatDetail) != NV_OK) {
		MessageBox(NULL, "Unable to enum video signal formats\n", "Error", MB_OK);
	}

	// Set width and height
	m_videoWidth = l_signalFormatDetail.videoMode.dwHorizontalPixels;
	m_videoHeight = l_signalFormatDetail.videoMode.dwVerticalLines;

	// Set interlace flag.
	if ((l_signalFormatDetail.videoMode.interlaceMode == NVINTERLACEMODE_INTERLACE)  ||
	    (l_signalFormatDetail.videoMode.interlaceMode = NVINTERLACEMODE_PSF)) {
		m_bInterlaced = TRUE;
	} else {
		m_bInterlaced = FALSE;
	}
#endif
	return S_OK;
}

//
// Start video transfers
//
HRESULT
CNvSDIout::Start()
{
	// In the case of GVO, this is a noop.  The OpenGL extension
	// functions wglBindVideoImageNV (WGL_NV_video_out) and 
	// wglBindVideoDeviceNV (GL_NV_present_video) start / stop
	// video transfers.
#ifdef USE_NVAPI
#else
#endif
	return S_OK;
}

//
// Stop video transfers
//
HRESULT
CNvSDIout::Stop()
{
	// In the case of GVO, this is a noop.  The OpenGL extension
	// functions wglBindVideoImageNV (WGL_NV_video_out) and 
	// wglBindVideoDeviceNV (GL_NV_present_video) start / stop
	// video transfers.
#ifdef USE_NVAPI
#else
#endif

	return S_OK;
}

//
// Cleanup video
//
HRESULT
CNvSDIout::Cleanup()
{

	Stop();

	if (NvAPI_VIO_Close(m_vioHandle, TRUE) != NVAPI_OK) {
		return E_FAIL;
	}

	m_vioHandle = NULL;
		
#ifdef USE_NVAPI

#else
	NVRESULT l_nvResult;

	// Turn off compositing
	NVGVOCONFIG l_gvoConfig;
	l_gvoConfig.cbSize = sizeof(NVGVOCONFIG);
	l_gvoConfig.dwFields = 0;
	l_gvoConfig.dwFields = NVGVOCONFIG_COMPOSITE;
	l_gvoConfig.bEnableComposite = FALSE;
	NVGVOCHECK(NvGvoConfigSet(g_hGVO, &l_gvoConfig));

	if ( m_hGVO ) {

		if ( NvGvoIsRunning(m_hGVO) ) {
			l_nvResult = NvGvoStop(m_hGVO);
			switch (l_nvResult) {
				case NV_ACCESSDENIED:
					cleanupGL();
					l_nvResult = NvGvoStop(m_hGVO);
					break;
				case NV_NOTRUNNING:
					MessageBox(NULL, "NvGvoStop: NV_NOTRUNNING", "Error", MB_OK);
					break;
				default:
					break;
			}
		}

		// Reset CSC matrix to default.
		if (options.cscEnable) {
			NVRESULT l_nvResult = NV_OK;
			NVGVOCONFIG l_gvoConfig;
			l_gvoConfig.cbSize = sizeof(l_gvoConfig);
			l_gvoConfig.dwFields = 0;
			l_gvoConfig.dwFields |= NVGVOCONFIG_CSCOVERRIDE;
			l_gvoConfig.bCSCOverride = FALSE;
			NVGVOCHECK(NvGvoConfigSet(m_hGVO, &l_gvoConfig));
		}

		if ( NvGvoClose(m_hGVO) != NV_OK ) {
			return E_FAIL;
		}

		// Try to open the device again.
		if ( NvGvoOpen(1, NULL, NVGVOCLASS_SDI, NVGVO_O_WRITE_EXCLUSIVE, &m_hGVO) != NV_OK ) {
			return E_FAIL;
		}

		if ( NvGvoClose(m_hGVO) != NV_OK ) {
			return E_FAIL;
		}	
	}
#endif

	return S_OK;
}

//
// Display video status
//
HRESULT
CNvSDIout::DisplayVideoStatus(GLvoid)
{
	char text[1000];
	char newtext[100];

	// Get running state
	sprintf(text, "NvAPI_VIO_IsRunning(): ");
#ifdef USE_NVAPI
	if (NvAPI_VIO_IsRunning(m_vioHandle) == NVAPI_DRIVER_RUNNING) {
		sprintf(newtext, "NVAPI_DRIVER_RUNNING\n");
	} else if (NvAPI_VIO_IsRunning(m_vioHandle) == NVAPI_DRIVER_NOTRUNNING) {
		sprintf(newtext, "NVAPI_DRIVER_NOTRUNNING\n");
	} else {
		sprintf(newtext, "undefined\n");
	}
	strcat(text, newtext);

	// Query milliseconds to wait before getting incoming sync format
	NvU32 l_wait;

	// Trigger redetection of sync format
	if (NvAPI_VIO_SyncFormatDetect(m_vioHandle, &l_wait) != NVAPI_OK) {
		return E_FAIL;
	}

	// Wait for sync detection to complete 
	Sleep(l_wait);

	// Get sync signal format
	NVVIOSTATUS l_vioStatus;
	l_vioStatus.version = NVVIOSTATUS_VER;
	if (NvAPI_VIO_Status(m_vioHandle, &l_vioStatus) != NVAPI_OK) {
		return E_FAIL;
	}

	sprintf(newtext, "Video 1 Out: ");
	strcat(text, newtext);
	switch(l_vioStatus.vioStatus.outStatus.vid1Out) {		
		case NVINPUTOUTPUTSTATUS_OFF:
			sprintf(newtext, "NVINPUTOUTPUTSTATUS_OFF\n");
			break;
		case NVINPUTOUTPUTSTATUS_ERROR:
			sprintf(newtext, "NVINPUTOUTPUTSTATUS_ERROR\n");
			break;
		case NVINPUTOUTPUTSTATUS_SDI_SD:
			sprintf(newtext, "NVINPUTOUTPUTSTATUS_SDI_SD\n");
			break;
		case NVINPUTOUTPUTSTATUS_SDI_HD:
			sprintf(newtext, "NVINPUTOUTPUTSTATUS_SDI_HD\n");
			break;
		default:
			sprintf(newtext, "undefined\n");
	}
	strcat(text, newtext);

	sprintf(newtext, "Video 2 Out: ");
	strcat(text, newtext);
	switch(l_vioStatus.vioStatus.outStatus.vid2Out) {	
		case NVINPUTOUTPUTSTATUS_OFF:
			sprintf(newtext, "NVINPUTOUTPUTSTATUS_OFF\n");
			break;
		case NVINPUTOUTPUTSTATUS_ERROR:
			sprintf(newtext, "NVINPUTOUTPUTSTATUS_ERROR\n");
			break;
		case NVINPUTOUTPUTSTATUS_SDI_SD:
			sprintf(newtext, "NVINPUTOUTPUTSTATUS_SDI_SD\n");
			break;
		case NVINPUTOUTPUTSTATUS_SDI_HD:
			sprintf(newtext, "NVINPUTOUTPUTSTATUS_SDI_HD\n");
			break;
		default:
			sprintf(newtext, "undefined\n");
	}
	strcat(text, newtext);

	if (l_vioStatus.vioStatus.outStatus.syncEnable) {
		sprintf(newtext, "Sync Source: ");
		strcat(text, newtext);
		switch(l_vioStatus.vioStatus.outStatus.syncSource) {
			case NVVIOSYNCSOURCE_SDISYNC:
				sprintf(newtext, "NVVIOSYNCSOURCE_SDISYNC\n");
				break;
			case NVVIOSYNCSOURCE_COMPSYNC:
				sprintf(newtext, "NVVIOSYNCSOURCE_COMPSYNC\n");
				break;
			default:
				sprintf(newtext, "undefined\n");
		}
		strcat(text, newtext);

		sprintf(newtext, "Sync Format: ");
		strcat(text, newtext);
		switch(l_vioStatus.vioStatus.outStatus.syncFormat) {
			case NVVIOSIGNALFORMAT_NONE:
				sprintf(newtext, "NVVIOSIGNALFORMAT_NONE\n");
				break;
    		case NVVIOSIGNALFORMAT_487I_59_94_SMPTE259_NTSC:
				sprintf(newtext, "NVVIOSIGNALFORMAT_487I_59_94_SMPTE259_NTSC\n");
				break;
			case NVVIOSIGNALFORMAT_576I_50_00_SMPTE259_PAL:
				sprintf(newtext, "NVVIOSIGNALFORMAT_576I_50_00_SMPTE259_PAL\n");
				break;
			case NVVIOSIGNALFORMAT_720P_59_94_SMPTE296:
				sprintf(newtext, "NVVIOSIGNALFORMAT_720P_59_94_SMPTE296\n");
				break;
			case NVVIOSIGNALFORMAT_720P_60_00_SMPTE296:
				sprintf(newtext, "NVVIOSIGNALFORMAT_720P_60_00_SMPTE296\n");
				break;
			case NVVIOSIGNALFORMAT_1035I_59_94_SMPTE260:
				sprintf(newtext, "NVVIOSIGNALFORMAT_1035I_59_94_SMPTE26\n");
				break;
			case NVVIOSIGNALFORMAT_1035I_60_00_SMPTE260:
				sprintf(newtext, "NVVIOSIGNALFORMAT_1035I_60_00_SMPTE260\n");
				break;
			case NVVIOSIGNALFORMAT_1080I_50_00_SMPTE295:
				sprintf(newtext, "NVVIOSIGNALFORMAT_1080I_50_00_SMPTE295\n");
				break;
			case NVVIOSIGNALFORMAT_1080I_50_00_SMPTE274:
				sprintf(newtext, "NVVIOSIGNALFORMAT_1080I_50_00_SMPTE274\n");
				break;
			case NVVIOSIGNALFORMAT_1080I_59_94_SMPTE274:
				sprintf(newtext, "NVVIOSIGNALFORMAT_1080I_59_94_SMPTE274\n");
				break;
			case NVVIOSIGNALFORMAT_1080I_60_00_SMPTE274:
				sprintf(newtext, "NVVIOSIGNALFORMAT_1080I_60_00_SMPTE274\n");
				break;
			case NVVIOSIGNALFORMAT_1080PSF_23_98_SMPTE274:
				sprintf(newtext, "NVVIOSIGNALFORMAT_1080PSF_23_98_SMPTE274\n");
				break;
			case NVVIOSIGNALFORMAT_1080PSF_24_00_SMPTE274:
				sprintf(newtext, "NVVIOSIGNALFORMAT_1080PSF_24_00_SMPTE274\n");
				break;
			case NVVIOSIGNALFORMAT_1080PSF_25_00_SMPTE274:
				sprintf(newtext, "NVVIOSIGNALFORMAT_1080PSF_25_00_SMPTE274\n");
				break;
//			case NVVIOSIGNALFORMAT_1080PSF_29_97_SMPTE274:
//				sprintf(newtext, "NVVIOSIGNALFORMAT_1080PSF_29_97_SMPTE274\n");
//				break;
			case NVVIOSIGNALFORMAT_1080P_23_976_SMPTE274:
				sprintf(newtext, "NVVIOSIGNALFORMAT_1080P_23_976_SMPTE274\n");
				break;
			case NVVIOSIGNALFORMAT_1080P_24_00_SMPTE274:
				sprintf(newtext, "NVVIOSIGNALFORMAT_1080P_24_00_SMPTE274\n");
				break;
			case NVVIOSIGNALFORMAT_1080P_25_00_SMPTE274:
				sprintf(newtext, "NVVIOSIGNALFORMAT_1080P_25_00_SMPTE274\n");
				break;
			case NVVIOSIGNALFORMAT_1080P_29_97_SMPTE274:
				sprintf(newtext, "NVVIOSIGNALFORMAT_1080P_29_97_SMPTE274\n");
				break;
			case NVVIOSIGNALFORMAT_1080P_30_00_SMPTE274:
				sprintf(newtext, "NVVIOSIGNALFORMAT_1080P_30_00_SMPTE274\n");
				break;
			default:
				sprintf(newtext, "undefined\n");
		}
		strcat(text, newtext);

		sprintf(newtext, "Composite Sync In: ");
		strcat(text, newtext);
		switch(l_vioStatus.vioStatus.outStatus.compSyncIn) {
			case NVVIOSYNCSTATUS_OFF:
				sprintf(newtext, "NVVIOSYNCSTATUS_OFF\n");
				break;
			case NVVIOSYNCSTATUS_ERROR: 
				sprintf(newtext, "NVVIOSYNCSTATUS_ERROR\n");
				break;
			case NVVIOSYNCSTATUS_SYNCLOSS:
				sprintf(newtext, "NVVIOSYNCSTATUS_SYNCLOSS\n");
				break;
			case NVVIOSYNCSTATUS_COMPOSITE:
				sprintf(newtext, "NVVIOSYNCSTATUS_COMPOSITE\n");
				break;
			case NVVIOSYNCSTATUS_SDI_SD:
				sprintf(newtext, "NVVIOSYNCSTATUS_SDI_SD\n");
				break;
			case NVVIOSYNCSTATUS_SDI_HD:
				sprintf(newtext, "NVVIOSYNCSTATUS_SDI_HD\n");
				break;
		}
		strcat(text, newtext);

		sprintf(newtext, "SDI Sync In: ");
		strcat(text, newtext);
		switch(l_vioStatus.vioStatus.outStatus.sdiSyncIn) {
			case NVVIOSYNCSTATUS_OFF:
				sprintf(newtext, "NVVIOSYNCSTATUS_OFF\n");
				break;
			case NVVIOSYNCSTATUS_ERROR: 
				sprintf(newtext, "NVVIOSYNCSTATUS_ERROR\n");
				break;
			case NVVIOSYNCSTATUS_SYNCLOSS:
				sprintf(newtext, "NVVIOSYNCSTATUS_SYNCLOSS\n");
				break;
			case NVVIOSYNCSTATUS_COMPOSITE:
				sprintf(newtext, "NVVIOSYNCSTATUS_COMPOSITE\n");
				break;
			case NVVIOSYNCSTATUS_SDI_SD:
				sprintf(newtext, "NVVIOSYNCSTATUS_SDI_SD\n");
				break;
			case NVVIOSYNCSTATUS_SDI_HD:
				sprintf(newtext, "NVVIOSYNCSTATUS_SDI_HD\n");
				break;
		}
		strcat(text, newtext);

	} else {
		sprintf(newtext, "Sync Source: Disabled");
		strcat(text, newtext);
	}

	if (l_vioStatus.vioStatus.outStatus.frameLockEnable) {
		sprintf(newtext, "Framelock Enabled\n");
	} else {
		sprintf(newtext, "Framelock Disabled\n");
	}
	strcat(text, newtext);

	if (l_vioStatus.vioStatus.outStatus.outputVideoLocked) {
		sprintf(newtext, "Output Video Locked\n");
	} else {
		sprintf(newtext, "Output Video Not Locked\n");
	}
	strcat(text, newtext);
#else
	NVGVOSTATUS l_gvoStatus;
	NVRESULT l_nvResult = NV_OK;
	if (NvGvoIsRunning(m_hGVO) == NV_RUNNING) {
		sprintf(newtext, "NV_RUNNING\n");
	} else if (NvGvoIsRunning(m_hGVO) == NV_NOTRUNNING) {
		sprintf(newtext, "NV_NOTRUNNING\n");
	} else {
		sprintf(newtext, "undefined\n");
	}

	strcat(text, newtext);

	// Query milliseconds to wait before getting incoming sync format
	DWORD l_dwWait;
	l_nvResult = NvGvoSyncFormatDetect(m_hGVO, &l_dwWait);
 
	// Get video status.
	l_gvoStatus.cbSize = sizeof(l_gvoStatus);
	l_nvResult = NvGvoStatus(m_hGVO, &l_gvoStatus);

	sprintf(newtext, "Video 1 Out: ");
	strcat(text, newtext);
	switch(l_gvoStatus.vid1Out) {		
		case NVGVOOUTPUTSTATUS_OFF:
			sprintf(newtext, "NVGVOOUTPUTSTATUS_OFF\n");
			break;
		case NVGVOOUTPUTSTATUS_ERROR:
			sprintf(newtext, "NVGVOOUTPUTSTATUS_ERROR\n");
			break;
		case NVGVOOUTPUTSTATUS_SDI_SD:
			sprintf(newtext, "NVGVOOUTPUTSTATUS_SDI_SD\n");
			break;
		case NVGVOOUTPUTSTATUS_SDI_HD:
			sprintf(newtext, "NVGVOOUTPUTSTATUS_SDI_HD\n");
			break;
		default:
			sprintf(newtext, "undefined\n");
	}
	strcat(text, newtext);

	sprintf(newtext, "Video 2 Out: ");
	strcat(text, newtext);
	switch(l_gvoStatus.vid2Out) {	
		case NVGVOOUTPUTSTATUS_OFF:
			sprintf(newtext, "NVGVOOUTPUTSTATUS_OFF\n");
			break;
		case NVGVOOUTPUTSTATUS_ERROR:
			sprintf(newtext, "NVGVOOUTPUTSTATUS_ERROR\n");
			break;
		case NVGVOOUTPUTSTATUS_SDI_SD:
			sprintf(newtext, "NVGVOOUTPUTSTATUS_SDI_SD\n");
			break;
		case NVGVOOUTPUTSTATUS_SDI_HD:
			sprintf(newtext, "NVGVOOUTPUTSTATUS_SDI_HD\n");
			break;
		default:
			sprintf(newtext, "undefined\n");
	}
	strcat(text, newtext);

	if (l_gvoStatus.syncEnable) {
		sprintf(newtext, "Sync Source: ");
		strcat(text, newtext);
		switch(l_gvoStatus.syncSource) {
			case NVGVOSYNCSOURCE_SDISYNC:
				sprintf(newtext, "NVGVOSYNCSOURCE_SDISYNC\n");
				break;
			case NVGVOSYNCSOURCE_COMPSYNC:
				sprintf(newtext, "NVGVOSYNCSOURCE_COMPSYNC\n");
				break;
			default:
				sprintf(newtext, "undefined\n");
		}
		strcat(text, newtext);

		sprintf(newtext, "Sync Format: ");
		strcat(text, newtext);
		switch(l_gvoStatus.syncFormat) {
			case NVGVOSIGNALFORMAT_ERROR:
				sprintf(newtext, "NVGVOSIGNALFORMAT_ERROR\n");
				break;
    		case NVGVOSIGNALFORMAT_487I_5994_SMPTE259_NTSC:
				sprintf(newtext, "NVGVOSIGNALFORMAT_487I_5994_SMPTE259_NTSC\n");
				break;
			case NVGVOSIGNALFORMAT_576I_5000_SMPTE259_PAL:
				sprintf(newtext, "NVGVOSIGNALFORMAT_576I_5000_SMPTE259_PAL\n");
				break;
			case NVGVOSIGNALFORMAT_720P_5994_SMPTE296:
				sprintf(newtext, "NVGVOSIGNALFORMAT_720P_5994_SMPTE296\n");
				break;
			case NVGVOSIGNALFORMAT_720P_6000_SMPTE296:
				sprintf(newtext, "NVGVOSIGNALFORMAT_720P_6000_SMPTE296\n");
				break;
			case NVGVOSIGNALFORMAT_1035I_5994_SMPTE260:
				sprintf(newtext, "NVGVOSIGNALFORMAT_1035I_5994_SMPTE26\n");
				break;
			case NVGVOSIGNALFORMAT_1035I_6000_SMPTE260:
				sprintf(newtext, "NVGVOSIGNALFORMAT_1035I_6000_SMPTE260\n");
				break;
			case NVGVOSIGNALFORMAT_1080I_5000_SMPTE295:
				sprintf(newtext, "NVGVOSIGNALFORMAT_1080I_5000_SMPTE295\n");
				break;
			case NVGVOSIGNALFORMAT_1080I_5000_SMPTE274:
				sprintf(newtext, "NVGVOSIGNALFORMAT_1080I_5000_SMPTE274\n");
				break;
			case NVGVOSIGNALFORMAT_1080I_5994_SMPTE274:
				sprintf(newtext, "NVGVOSIGNALFORMAT_1080I_5994_SMPTE274\n");
				break;
			case NVGVOSIGNALFORMAT_1080I_6000_SMPTE274:
				sprintf(newtext, "NVGVOSIGNALFORMAT_1080I_6000_SMPTE274\n");
				break;
			case NVGVOSIGNALFORMAT_1080PSF_23976_SMPTE274:
				sprintf(newtext, "NVGVOSIGNALFORMAT_1080PSF_23976_SMPTE274\n");
				break;
			case NVGVOSIGNALFORMAT_1080PSF_2400_SMPTE274:
				sprintf(newtext, "NVGVOSIGNALFORMAT_1080PSF_2400_SMPTE274\n");
				break;
			case NVGVOSIGNALFORMAT_1080PSF_2500_SMPTE274:
				sprintf(newtext, "NVGVOSIGNALFORMAT_1080PSF_2500_SMPTE274\n");
				break;
//			case NVGVOSIGNALFORMAT_1080PSF_2997_SMPTE274:
//				sprintf(newtext, "NVGVOSIGNALFORMAT_1080PSF_2997_SMPTE274\n");
//				break;
			case NVGVOSIGNALFORMAT_1080P_23976_SMPTE274:
				sprintf(newtext, "NVGVOSIGNALFORMAT_1080P_23976_SMPTE274\n");
				break;
			case NVGVOSIGNALFORMAT_1080P_2400_SMPTE274:
				sprintf(newtext, "NVGVOSIGNALFORMAT_1080P_2400_SMPTE274\n");
				break;
			case NVGVOSIGNALFORMAT_1080P_2500_SMPTE274:
				sprintf(newtext, "NVGVOSIGNALFORMAT_1080P_2500_SMPTE274\n");
				break;
			case NVGVOSIGNALFORMAT_1080P_2997_SMPTE274:
				sprintf(newtext, "NVGVOSIGNALFORMAT_1080P_2997_SMPTE274\n");
				break;
			case NVGVOSIGNALFORMAT_1080P_3000_SMPTE274:
				sprintf(newtext, "NVGVOSIGNALFORMAT_1080P_3000_SMPTE274\n");
				break;
			default:
				sprintf(newtext, "undefined\n");
		}
		strcat(text, newtext);

		sprintf(newtext, "Composite Sync In: ");
		strcat(text, newtext);
		switch(l_gvoStatus.compSyncIn) {
			case NVGVOSYNCSTATUS_OFF:
				sprintf(newtext, "NVGVOSYNCSTATUS_OFF\n");
				break;
			case NVGVOSYNCSTATUS_ERROR: 
				sprintf(newtext, "NVGVOSYNCSTATUS_ERROR\n");
				break;
			case NVGVOSYNCSTATUS_SYNCLOSS:
				sprintf(newtext, "NVGVOSYNCSTATUS_SYNCLOSS\n");
				break;
			case NVGVOSYNCSTATUS_COMPOSITE:
				sprintf(newtext, "NVGVOSYNCSTATUS_COMPOSITE\n");
				break;
			case NVGVOSYNCSTATUS_SDI_SD:
				sprintf(newtext, "NVGVOSYNCSTATUS_SDI_SD\n");
				break;
			case NVGVOSYNCSTATUS_SDI_HD:
				sprintf(newtext, "NVGVOSYNCSTATUS_SDI_HD\n");
				break;
		}
		strcat(text, newtext);

		sprintf(newtext, "SDI Sync In: ");
		strcat(text, newtext);
		switch(l_gvoStatus.sdiSyncIn) {
			case NVGVOSYNCSTATUS_OFF:
				sprintf(newtext, "NVOGVOSYNCSTATUS_OFF\n");
				break;
			case NVGVOSYNCSTATUS_ERROR: 
				sprintf(newtext, "NVOGVOSYNCSTATUS_ERROR\n");
				break;
			case NVGVOSYNCSTATUS_SYNCLOSS:
				sprintf(newtext, "NVOGVOSYNCSTATUS_SYNCLOSS\n");
				break;
			case NVGVOSYNCSTATUS_COMPOSITE:
				sprintf(newtext, "NVOGVOSYNCSTATUS_COMPOSITE\n");
				break;
			case NVGVOSYNCSTATUS_SDI_SD:
				sprintf(newtext, "NVGVOSYNCSTATUS_SDI_SD\n");
				break;
			case NVGVOSYNCSTATUS_SDI_HD:
				sprintf(newtext, "NVGVOSYNCSTATUS_SDI_HD\n");
				break;
		}
		strcat(text, newtext);

	} else {
		sprintf(newtext, "Sync Source: Disabled");
		strcat(text, newtext);
	}

	if (l_gvoStatus.bframeLockEnable) {
		sprintf(newtext, "Framelock Enabled\n");
	} else {
		sprintf(newtext, "Framelock Disabled\n");
	}
	strcat(text, newtext);

	if (l_gvoStatus.bOutputVideoLocked) {
		sprintf(newtext, "Output Video Locked\n");
	} else {
		sprintf(newtext, "Output Video Not Locked\n");
	}
	strcat(text, newtext);
#endif

	MessageBox(NULL, text, "Video Status", MB_ICONINFORMATION/* | MB_APPLMODAL*/);

	return S_OK;
}	


//
// Display video signal format information.
//
HRESULT
CNvSDIout::DisplaySignalFormatInfo()
{
	char text[1000];
	char newtext[100];
#ifdef USE_NVAPI
	// Get signal format from GVO config.
	NVVIOCONFIG_V1 l_vioConfig;
	memset(&l_vioConfig, 0, sizeof(l_vioConfig));
	l_vioConfig.version = NVVIOCONFIG_VER1;
	l_vioConfig.fields = NVVIOCONFIG_SIGNALFORMAT;
	if (NvAPI_VIO_GetConfig(m_vioHandle, (NVVIOCONFIG *)&l_vioConfig) != NVAPI_OK) {
		MessageBox(NULL, "Unable to get video config.\n", "Error", MB_OK);
		return E_FAIL;
	}

	// Get signal format detail.
	NVVIOSIGNALFORMATDETAIL l_vioSignalFormatDetail;
	memset(&l_vioSignalFormatDetail, 0, sizeof(l_vioSignalFormatDetail));

	ULONG l_vioSignalFormatIndex = (ULONG)NVVIOSIGNALFORMAT_NONE; 

	// Enumerate all sigal formats until we find the one we are looking
	// for, the enueration ends, or there is an error.
	NvAPI_Status l_ret;
	while(1) { 
		l_ret = NvAPI_VIO_EnumSignalFormats(m_vioHandle, 
		 								    l_vioSignalFormatIndex, 
										    &l_vioSignalFormatDetail); 
        if (l_ret == NVAPI_END_ENUMERATION || l_ret != NVAPI_OK)  { 
			return E_FAIL; 
		} // if

		// We found the signal format that we were looking for so break.
		if (l_vioSignalFormatDetail.signalFormat == 
			l_vioConfig.vioConfig.outConfig.signalFormat) { 
			break;
		} // if

		l_vioSignalFormatIndex++; 
	} // while

	sprintf(text, "%s\n", SignalFormatToString(l_vioConfig.vioConfig.outConfig.signalFormat).c_str());

	sprintf(newtext, "Horizontal Pixels = %d\n", l_vioSignalFormatDetail.videoMode.horizontalPixels);
	strcat(text, newtext);
	sprintf(newtext, "Vertical Lines = %d\n", l_vioSignalFormatDetail.videoMode.verticalLines);
	strcat(text, newtext);
	sprintf(newtext, "Frame Rate = %f\n", l_vioSignalFormatDetail.videoMode.fFrameRate);
	strcat(text, newtext);

	switch (l_vioSignalFormatDetail.videoMode.interlaceMode) {
	case NVVIOINTERLACEMODE_PROGRESSIVE:
		sprintf(newtext, "Progressive\n");
		break;
	 
	case NVVIOINTERLACEMODE_INTERLACE:
		sprintf(newtext, "Interlace\n");
		break;

	case NVVIOINTERLACEMODE_PSF:
		sprintf(newtext, "Progressive Segment Frame\n");
		break;
	}
	strcat(text, newtext);

	switch (l_vioSignalFormatDetail.videoMode.videoStandard) {
	case NVVIOVIDEOSTANDARD_SMPTE259:
		sprintf(newtext, "SMPTE259\n");
		break;

	case NVVIOVIDEOSTANDARD_SMPTE260:
		sprintf(newtext, "SMPTE260\n");
		break;

	case NVVIOVIDEOSTANDARD_SMPTE274:
		sprintf(newtext, "SMPTE274\n");
		break;

	case NVVIOVIDEOSTANDARD_SMPTE295:
		sprintf(newtext, "SMPTE295\n");
		break;

	case NVVIOVIDEOSTANDARD_SMPTE296:
		sprintf(newtext, "SMPTE296\n");
		break;

	case NVVIOVIDEOSTANDARD_SMPTE372:
		sprintf(newtext, "SMPTE372\n");
		break;
	}
	strcat(text, newtext);

	switch (l_vioSignalFormatDetail.videoMode.videoType) {
	case NVVIOVIDEOTYPE_HD: 
		sprintf(newtext, "High-Definition\n");
		break;

	case NVVIOVIDEOTYPE_SD:
		sprintf(newtext, "Standard-Definition\n");
		break;

	}

#else
	// Get signal format from GVO config.
	NVGVOCONFIG l_gvoConfig = {0};
	l_gvoConfig.cbSize   = sizeof(l_gvoConfig);
	l_gvoConfig.dwFields = NVGVOCONFIG_SIGNALFORMAT;
	if (NvGvoConfigGet(g_hGVO, &l_gvoConfig) != NV_OK) {
		MessageBox(NULL, "Unable to get video config.\n", "Error", MB_OK);
		return FALSE;
	}

	// Enumerate signal format.
	NVGVOSIGNALFORMATDETAIL l_signalFormatDetail = {0};
	l_signalFormatDetail.cbSize = sizeof(l_signalFormatDetail);
	if (NvGvoEnumSignalFormats(g_hGVO, l_gvoConfig.signalFormat,
		TRUE, &l_signalFormatDetail) != NV_OK) {
		MessageBox(NULL, "Unable to enum video signal formats\n", "Error", MB_OK);
	}
		
	sprintf(text, "%s\n", l_signalFormatDetail.szValueName);
	if (l_signalFormatDetail.szAlternateName[0] != '\0') {
		sprintf(newtext, "%s\n", l_signalFormatDetail.szAlternateName);
		strcat(text, newtext);
	}

	sprintf(newtext, "Horizontal Pixels = %d\n", l_signalFormatDetail.videoMode.dwHorizontalPixels);
	strcat(text, newtext);
	sprintf(newtext, "Vertical Lines = %d\n", l_signalFormatDetail.videoMode.dwVerticalLines);
	strcat(text, newtext);
	sprintf(newtext, "Frame Rate = %f\n", l_signalFormatDetail.videoMode.fFrameRate);
	strcat(text, newtext);

	switch (l_signalFormatDetail.videoMode.interlaceMode) {
	case NVINTERLACEMODE_PROGRESSIVE:
		sprintf(newtext, "Progressive\n");
		break;
	 
	case NVINTERLACEMODE_INTERLACE:
		sprintf(newtext, "Interlace\n");
		break;

	case NVINTERLACEMODE_PSF:
		sprintf(newtext, "Progressive Segment Frame\n");
		break;
	}
	strcat(text, newtext);

	switch (l_signalFormatDetail.videoMode.videoStandard) {
	case NVVIDEOSTANDARD_SMPTE259:
		sprintf(newtext, "SMPTE259\n");
		break;

	case NVVIDEOSTANDARD_SMPTE260:
		sprintf(newtext, "SMPTE260\n");
		break;

	case NVVIDEOSTANDARD_SMPTE274:
		sprintf(newtext, "SMPTE274\n");
		break;

	case NVVIDEOSTANDARD_SMPTE295:
		sprintf(newtext, "SMPTE295\n");
		break;

	case NVVIDEOSTANDARD_SMPTE296:
		sprintf(newtext, "SMPTE296\n");
		break;
	}
	strcat(text, newtext);

	switch (l_signalFormatDetail.videoMode.videoType) {
	case NVVIDEOTYPE_HD: 
		sprintf(newtext, "High-Definition\n");
		break;

	case NVVIDEOTYPE_SD:
		sprintf(newtext, "Standard-Definition\n");
		break;

	}
#endif
	strcat(text, newtext);

	MessageBox(NULL, text, "Signal Format Info", MB_ICONINFORMATION/* | MB_APPLMODAL*/);

	return S_OK;
}

//
// Display data format information
//
HRESULT
CNvSDIout::DisplayDataFormatInfo()
{
	char text[1000];
	char newtext[100];
#ifdef USE_NVAPI
	// Get data format from GVO config.
	NVVIOCONFIG_V1 l_vioConfig;
	memset(&l_vioConfig, 0, sizeof(l_vioConfig));
	l_vioConfig.version = NVVIOCONFIG_VER1;
	l_vioConfig.fields = NVVIOCONFIG_ALLFIELDS; //NVVIOCONFIG_DATAFORMAT;
	if (NvAPI_VIO_GetConfig(m_vioHandle, (NVVIOCONFIG *)&l_vioConfig) != NVAPI_OK) {
		MessageBox(NULL, "Unable to get video config.\n", "Error", MB_OK);
		return E_FAIL;
	}

	// Get data format detail.
	NVVIODATAFORMATDETAIL l_dataFormatDetail;
	if (NvAPI_VIO_EnumDataFormats(m_vioHandle, 
									l_vioConfig.vioConfig.outConfig.dataFormat,
									&l_dataFormatDetail) != NVAPI_OK) {
		MessageBox(NULL, "Unable to enum video data formats\n", "Error", MB_OK);
		return E_FAIL;
	}

	if (l_dataFormatDetail.vioCaps != 0) {
		if (l_dataFormatDetail.vioCaps & NVVIOCAPS_VIDOUT_SDI) {
			sprintf(text, "VIDOUT_SDI\n");
		}
		if (l_dataFormatDetail.vioCaps & NVVIOCAPS_OUTPUTMODE_DESKTOP) {
			sprintf(text, "OUTPUTMODE_DESKTOP\n");
		}
		if (l_dataFormatDetail.vioCaps & NVVIOCAPS_OUTPUTMODE_OPENGL) {
			sprintf(text, "OUTPUTMODE_OPENGL\n");
		}
		if (l_dataFormatDetail.vioCaps & NVVIOCAPS_SYNC_INTERNAL) {
			sprintf(text, "SYNC_INTERNAL\n");
		}
		if (l_dataFormatDetail.vioCaps & NVVIOCAPS_SYNC_GENLOCK) {
			sprintf(text, "SYNC_GENLOCK\n");
		}
		if (l_dataFormatDetail.vioCaps & NVVIOCAPS_SYNCSRC_SDI) {
			sprintf(text, "SYNCSRC_SDI\n");
		}
		if (l_dataFormatDetail.vioCaps & NVVIOCAPS_SYNCSRC_COMP) {
			sprintf(text, "SYNCSRC_COMP\n");
		}
	}
	
	sprintf(newtext, DataFormatToString(l_dataFormatDetail.dataFormat).c_str());
	strcat(text, newtext);

#else
	// Get signal format from GVO config.
	NVGVOCONFIG l_gvoConfig = {0};
	l_gvoConfig.cbSize   = sizeof(l_gvoConfig);
	l_gvoConfig.dwFields = NVGVOCONFIG_DATAFORMAT;
	if (NvGvoConfigGet(g_hGVO, &l_gvoConfig) != NV_OK) {
		MessageBox(NULL, "Unable to get video config.\n", "Error", MB_OK);
		return FALSE;
	}

	// Enumerate data format.
	NVGVODATAFORMATDETAIL l_dataFormatDetail = {0};
	l_dataFormatDetail.cbSize = sizeof(l_dataFormatDetail);
	if (NvGvoEnumDataFormats(g_hGVO, l_gvoConfig.dataFormat,
		TRUE, &l_dataFormatDetail) != NV_OK) {
		MessageBox(NULL, "Unable to enum video data formats\n", "Error", MB_OK);
		return FALSE;
	}

	if (l_dataFormatDetail.dwCaps != 0) {
		if (l_dataFormatDetail.dwCaps & NVGVOCAPS_VIDOUT_SDI) {
			sprintf(text, "VIDOUT_SDI\n");
		}
		if (l_dataFormatDetail.dwCaps & NVGVOCAPS_OUTPUTMODE_DESKTOP) {
			sprintf(text, "OUTPUTMODE_DESKTOP\n");
		}
		if (l_dataFormatDetail.dwCaps & NVGVOCAPS_OUTPUTMODE_OPENGL) {
			sprintf(text, "OUTPUTMODE_OPENGL\n");
		}
		if (l_dataFormatDetail.dwCaps & NVGVOCAPS_SYNC_INTERNAL) {
			sprintf(text, "SYNC_INTERNAL\n");
		}
		if (l_dataFormatDetail.dwCaps & NVGVOCAPS_SYNC_GENLOCK) {
			sprintf(text, "SYNC_GENLOCK\n");
		}
		if (l_dataFormatDetail.dwCaps & NVGVOCAPS_SYNCSRC_SDI) {
			sprintf(text, "SYNCSRC_SDI\n");
		}
		if (l_dataFormatDetail.dwCaps & NVGVOCAPS_SYNCSRC_COMP) {
			sprintf(text, "SYNCSRC_COMP\n");
		}
	}
	sprintf(newtext, "Input Format = %s\n", l_dataFormatDetail.in.szValueName);
	strcat(text, newtext);
	
	sprintf(newtext, "Output Format = %s\n", l_dataFormatDetail.out.szValueName);
	strcat(text, newtext);
#endif
	MessageBox(NULL, text, "Data Format Info", MB_ICONINFORMATION/* | MB_APPLMODAL*/);

	return S_OK;
}

//
// Display color conversion information
//
HRESULT
CNvSDIout::DisplayColorConversionInfo()
{
#ifdef USE_NVAPI
#else
	// Get signal format from GVO config.
	NVGVOCONFIG l_gvoConfig = {0};
	l_gvoConfig.cbSize   = sizeof(l_gvoConfig);
	l_gvoConfig.dwFields = NVGVOCONFIG_COLORCONVERSION;
	if (NvGvoConfigGet(g_hGVO, &l_gvoConfig) != NV_OK) {
		MessageBox(NULL, "Unable to get color conversion matrix.\n", "Error", MB_OK);
		return FALSE;
	}
#endif
	return S_OK;
}

//
// Display gamma correction information
//
HRESULT
CNvSDIout::DisplayGammaCorrectionInfo()
{
#ifdef USE_NVAPI
#else
	// Get signal format from GVO config.
	NVGVOCONFIG l_gvoConfig = {0};
	l_gvoConfig.cbSize   = sizeof(l_gvoConfig);
	l_gvoConfig.dwFields = NVGVOCONFIG_GAMMACORRECTION;
	if (NvGvoConfigGet(g_hGVO, &l_gvoConfig) != NV_OK) {
		MessageBox(NULL, "Unable to get gamma correction information.\n", "Error", MB_OK);
		return FALSE;
	}
#endif
	return S_OK;
}

//
// Get frame rate
//
HRESULT
CNvSDIout::GetFrameRate(float *rate)
{
	*rate = 0.0f;

	// Get signal format from GVO config.
	NVVIOCONFIG_V1 l_vioConfig;
	memset(&l_vioConfig, 0, sizeof(l_vioConfig));
	l_vioConfig.version = NVVIOCONFIG_VER1;
	l_vioConfig.fields = NVVIOCONFIG_SIGNALFORMAT;
	if (NvAPI_VIO_GetConfig(m_vioHandle, (NVVIOCONFIG *)&l_vioConfig) != NVAPI_OK) {
		MessageBox(NULL, "Unable to get video config.\n", "Error", MB_OK);
		return E_FAIL;
	}

	// Enumerate all sigal formats until we find the one we are looking
	// for, the enueration ends, or there is an error.
	NVVIOSIGNALFORMATDETAIL l_vioSignalFormatDetail;
	memset(&l_vioSignalFormatDetail, 0, sizeof(l_vioSignalFormatDetail));

	ULONG l_vioSignalFormatIndex = (ULONG)NVVIOSIGNALFORMAT_NONE; 

	while(1) { 
		NvAPI_Status l_ret = NVAPI_OK;
		l_ret = NvAPI_VIO_EnumSignalFormats(m_vioHandle, 
		 								    l_vioSignalFormatIndex, 
										    &l_vioSignalFormatDetail); 
        if (l_ret == NVAPI_END_ENUMERATION || l_ret != NVAPI_OK)  { 
			return E_FAIL; 
		} // if

		// We found the signal format that we were looking for so break.
		if (l_vioSignalFormatDetail.signalFormat == 
			l_vioConfig.vioConfig.outConfig.signalFormat) { 
			break;
		} // if

		l_vioSignalFormatIndex++; 
	} // while

	// Set rate
	*rate = l_vioSignalFormatDetail.videoMode.fFrameRate;
  
	return S_OK;
}

//
// Set Colorspace conversion
//
HRESULT
CNvSDIout::SetCSC(NVVIOCOLORCONVERSION *csc, bool enable)
{
	NVVIOCONFIG_V1 l_vioConfig;
	NvAPI_Status l_ret;

	memset(&l_vioConfig, 0, sizeof(l_vioConfig));
	l_vioConfig.version = NVVIOCONFIG_VER1;

	l_vioConfig.fields = NVVIOCONFIG_CSCOVERRIDE;

	// If not enabled, simply set as disabled.
	// Otherwise, enable and set values.
	if (!enable) {
		l_vioConfig.vioConfig.outConfig.cscOverride = FALSE;
	} else {
		l_vioConfig.vioConfig.outConfig.cscOverride = TRUE;

		l_vioConfig.fields |= NVVIOCONFIG_COLORCONVERSION;

		// Offset
		l_vioConfig.vioConfig.outConfig.colorConversion.colorOffset[0] = csc->colorOffset[0];
		l_vioConfig.vioConfig.outConfig.colorConversion.colorOffset[1] = csc->colorOffset[1];
		l_vioConfig.vioConfig.outConfig.colorConversion.colorOffset[2] = csc->colorOffset[2];

		// Scale
		l_vioConfig.vioConfig.outConfig.colorConversion.colorScale[0] = csc->colorScale[0];
	    l_vioConfig.vioConfig.outConfig.colorConversion.colorScale[1] = csc->colorScale[1];
	    l_vioConfig.vioConfig.outConfig.colorConversion.colorScale[2] = csc->colorScale[2];
		l_vioConfig.vioConfig.outConfig.colorConversion.compositeSafe = TRUE;

		// Matrix
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[0][0] = csc->colorMatrix[0][0]; 
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[0][1] = csc->colorMatrix[0][1]; 
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[0][2] = csc->colorMatrix[0][2];
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[1][0] = csc->colorMatrix[1][0];  
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[1][1] = csc->colorMatrix[1][1];   
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[1][2] = csc->colorMatrix[1][2];
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[2][0] = csc->colorMatrix[2][0];  
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[2][1] = csc->colorMatrix[2][1]; 
		l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[2][2] = csc->colorMatrix[2][2]; 

	} // if

	// Set configuration.
	l_ret = NvAPI_VIO_SetConfig(m_vioHandle, (NVVIOCONFIG *)&l_vioConfig);
	if (l_ret != NVAPI_OK) {

		// Display NVAPI error.
		NvAPI_ShortString l_desc;
		NvAPI_GetErrorMessage(l_ret, l_desc);
		MessageBox(NULL, l_desc, "Error", MB_OK);

		return E_FAIL;
	}

	return S_OK;
}

//
// Get Colorspace conversion.
//
HRESULT
CNvSDIout::GetCSC(NVVIOCOLORCONVERSION *csc, bool *enable)
{
	NVVIOCONFIG_V1 l_vioConfig;
	NvAPI_Status l_ret;

	memset(&l_vioConfig, 0, sizeof(l_vioConfig));
	l_vioConfig.version = NVVIOCONFIG_VER1;
	l_vioConfig.fields = NVVIOCONFIG_COLORCONVERSION | NVVIOCONFIG_CSCOVERRIDE;

	// Get configuration.
	l_ret = NvAPI_VIO_GetConfig(m_vioHandle, (NVVIOCONFIG *)&l_vioConfig);
	if (l_ret != NVAPI_OK) {
		return E_FAIL;
	}

	// Enable / disable state
	*enable = l_vioConfig.vioConfig.outConfig.cscOverride;

	// Offset
	csc->colorOffset[0] = l_vioConfig.vioConfig.outConfig.colorConversion.colorOffset[0];
	csc->colorOffset[1] = l_vioConfig.vioConfig.outConfig.colorConversion.colorOffset[1];
	csc->colorOffset[1] = l_vioConfig.vioConfig.outConfig.colorConversion.colorOffset[2];

	// Scale
	csc->colorScale[0] = l_vioConfig.vioConfig.outConfig.colorConversion.colorScale[0];
	csc->colorScale[1] = l_vioConfig.vioConfig.outConfig.colorConversion.colorScale[1];
	csc->colorScale[2] = l_vioConfig.vioConfig.outConfig.colorConversion.colorScale[2];

	// Composite safe?
	csc->compositeSafe = l_vioConfig.vioConfig.outConfig.colorConversion.compositeSafe;

	// Matrix
	csc->colorMatrix[0][0] = l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[0][0]; 
	csc->colorMatrix[0][1] = l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[0][1];
	csc->colorMatrix[0][2] = l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[0][2];
	csc->colorMatrix[1][0] = l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[1][0];  
	csc->colorMatrix[1][1] = l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[1][1];   
	csc->colorMatrix[1][2] = l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[1][2];
	csc->colorMatrix[2][0] = l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[2][0];  
	csc->colorMatrix[2][1] = l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[2][1]; 
	csc->colorMatrix[2][2] = l_vioConfig.vioConfig.outConfig.colorConversion.colorMatrix[2][2]; 

	return S_OK;
}

//
// Set sync delay
//
HRESULT
CNvSDIout::SetSyncDelay(NVVIOSYNCDELAY *delay)
{
	NVVIOCONFIG_V1 l_vioConfig;
	NvAPI_Status l_ret;

	memset(&l_vioConfig, 0, sizeof(l_vioConfig));
	l_vioConfig.version = NVVIOCONFIG_VER1;
	l_vioConfig.fields = NVVIOCONFIG_SYNCDELAY;

	// Set delay
	l_vioConfig.vioConfig.outConfig.syncDelay.version = delay->version;
	l_vioConfig.vioConfig.outConfig.syncDelay.horizontalDelay = delay->horizontalDelay;
	l_vioConfig.vioConfig.outConfig.syncDelay.verticalDelay = delay->verticalDelay;

	// Set configuration.
	l_ret = NvAPI_VIO_SetConfig(m_vioHandle, (NVVIOCONFIG *)&l_vioConfig);
	if (l_ret != NVAPI_OK) {
		return E_FAIL;
	}

	return S_OK;
}


//
// Get sync delay
//
HRESULT
CNvSDIout::GetSyncDelay(NVVIOSYNCDELAY *delay)
{
	NVVIOCONFIG_V1 l_vioConfig;
	NvAPI_Status l_ret;

	memset(&l_vioConfig, 0, sizeof(l_vioConfig));
	l_vioConfig.version = NVVIOCONFIG_VER1;
	l_vioConfig.fields = NVVIOCONFIG_SYNCDELAY;

	// Get configuration.
	l_ret = NvAPI_VIO_GetConfig(m_vioHandle, (NVVIOCONFIG *)&l_vioConfig);
	if (l_ret != NVAPI_OK) {
		return E_FAIL;
	}

	// Get delay
	delay->version = l_vioConfig.vioConfig.outConfig.syncDelay.version;
	delay->horizontalDelay = l_vioConfig.vioConfig.outConfig.syncDelay.horizontalDelay;
	delay->verticalDelay = l_vioConfig.vioConfig.outConfig.syncDelay.verticalDelay;

	return S_OK;
}




CNvSDIoutGpuTopology::CNvSDIoutGpuTopology() 	
{
	m_bInitialized = false;
	m_nGpu = 0;
	if(init())
		m_bInitialized = true;
}

CNvSDIoutGpuTopology::~CNvSDIoutGpuTopology()
{
	if (m_bInitialized == false)
		return;
	
	for (int i = 0; i < m_nGpu; i++)
	{
		delete dynamic_cast<CNvSDIoutGpu*>(m_lGpu[i]);
		m_lGpu[i] = NULL;
	}
	
	m_bInitialized = false;
	m_nGpu = 0;
}

static CNvSDIoutGpuTopology *s_instance = nullptr;
CNvSDIoutGpuTopology& CNvSDIoutGpuTopology::instance()
{
	if (!s_instance)
		s_instance = new CNvSDIoutGpuTopology;
	return *s_instance;
}

void CNvSDIoutGpuTopology::destroy()
{
	if (s_instance)
	{
		delete s_instance;
		s_instance = nullptr;
	}
}

bool CNvSDIoutGpuTopology::init()
{
	if(m_bInitialized)
		return true;

	m_nGpu = 0;

	//HWND hWnd;
	//HGLRC hGLRC;
	//if(CreateDummyGLWindow(&hWnd,&hGLRC) == false)
	//	return false;

	if(!loadAffinityExtension())
	{
		printf("Could not load OpenGL Affinity extension\n");
		return false;
	}

	// Query available VIO topologies.  
	// Initialize NVAPI
	if (NvAPI_Initialize() != NVAPI_OK) {
		return false;
	}	
	// Fail if there are no VIO topologies or devices available.
	NVVIOTOPOLOGY l_vioTopologies;
	memset(&l_vioTopologies, 0, sizeof(l_vioTopologies));
	l_vioTopologies.version = NVVIOTOPOLOGY_VER;

	if (NvAPI_VIO_QueryTopology(&l_vioTopologies) != NVAPI_OK) {
		return false;
	}

	if (l_vioTopologies.vioTotalDeviceCount == 0) {
		return false;
	}

	printf("\n\nListing Gpus available for OpenGL Gpu Affinity\n");
	
	unsigned int GPUIdx = 0;    

	GPU_DEVICE gpuDevice;
	gpuDevice.cb = sizeof(gpuDevice);	
	HGPUNV hGPU;
	bool bDisplay;
	bool bPrimary;
	bool bSDIoutput;
	NvVioHandle hVioHandle;
	while(wglEnumGpusNV(GPUIdx,&hGPU))	// First call this function to get a handle to the gpu
	{									    // wglEnumPGUsNV will fails if DeviceIdx > the available devices
        printf("GPU# %d:\n",GPUIdx);        
		bDisplay = false;
		bPrimary = false;
		bSDIoutput = false;
		hVioHandle = NULL;
		// Now get the detailed information about this device:
		//how many displays it's attached to and whethre any of them 
        int DisplayDeviceIdx = 0;
		while(wglEnumGpuDevicesNV(hGPU,DisplayDeviceIdx,&gpuDevice))
		{			
            printf(" Display# %d\n",DisplayDeviceIdx);
			printf("   Name: %s\n",gpuDevice.DeviceName);
			printf("   String: %s\n",gpuDevice.DeviceString);
			
			bDisplay = true;
			
			if(gpuDevice.Flags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
			{
                printf("   Attached to the desktop: LEFT=%d, RIGHT=%d, TOP=%d, BOTTOM=%d\n",
					gpuDevice.rcVirtualScreen.left, gpuDevice.rcVirtualScreen.right, gpuDevice.rcVirtualScreen.top, gpuDevice.rcVirtualScreen.bottom);
			}
			else
			{
				printf("   Not attached to the desktop\n");
			}
		  
            // See if it's the primary GPU
            if(gpuDevice.Flags & DISPLAY_DEVICE_PRIMARY_DEVICE)
			{
				printf("   This is the PRIMARY Display Device\n");
				bPrimary = true;
			}						
			DisplayDeviceIdx++;

			NvDisplayHandle hNvDisplay;
			NvAPI_Status status;
			status = NvAPI_GetAssociatedNvidiaDisplayHandle(gpuDevice.DeviceName,&hNvDisplay);
			if(status != NVAPI_OK)
				break;
			

			NvU32 count = 0;
			NvPhysicalGpuHandle hNvPhysicalGPU;		
			status = NvAPI_GetPhysicalGPUsFromDisplay(hNvDisplay,&hNvPhysicalGPU,&count);
			if(status != NVAPI_OK)
				break;
			// Cycle through all SDI topologies looking for the first
			// available SDI output device topology.
			BOOL l_bFound = FALSE;
			unsigned int i = 0;
			NVVIOCAPS l_vioCaps;
			while ((i < l_vioTopologies.vioTotalDeviceCount) && (!l_bFound)) {

				if(l_vioTopologies.vioTarget[i].hPhysicalGpu == hNvPhysicalGPU){
					// Get video I/O capabilities for current video I/O target.
					memset(&l_vioCaps, 0, sizeof(l_vioCaps));
					l_vioCaps.version = NVVIOCAPS_VER;
					if (NvAPI_VIO_GetCapabilities(l_vioTopologies.vioTarget[i].hVioHandle, 
									  &l_vioCaps) != NVAPI_OK) {
						continue;
					}
					//MessageBox(NULL, "Video I/O Unsupported.", "Error", MB_OK);
					//return E_FAIL;
					// If video output device found, save VIO handle and set flag.
					if (l_vioCaps.adapterCaps & NVVIOCAPS_VIDOUT_SDI) {					
						bSDIoutput = true;
						hVioHandle = l_vioTopologies.vioTarget[i].hVioHandle;
						l_bFound = TRUE;					
					} else {
						i++;
					}
				}
				else
				{
					i++;
				}
			} // while i < vioTotalDeviceCount
			
			if(l_bFound)
				printf("   SDI out: yes\n");
			else
			{				
				printf("   SDI out: no\n");
			}
			if(gpuDevice.Flags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
			{
                printf("   Attached to the desktop: LEFT=%d, RIGHT=%d, TOP=%d, BOTTOM=%d\n",
					gpuDevice.rcVirtualScreen.left, gpuDevice.rcVirtualScreen.right, gpuDevice.rcVirtualScreen.top, gpuDevice.rcVirtualScreen.bottom);
			}
			else
			{
				printf("   Not attached to the desktop\n");
			}
		}
		CNvSDIoutGpu *gpu = new CNvSDIoutGpu();
		gpu->init(hGPU,bPrimary,bDisplay,bSDIoutput,hVioHandle);

		m_lGpu[GPUIdx] = gpu;
		
		GPUIdx++;
	}
	
	m_nGpu = GPUIdx; 
	m_bInitialized = true;
	// We can kill the dummy window now
	//if(DestroyGLWindow(&hWnd,&hGLRC) == false)
	//	return false;

	return true;

}


CNvSDIoutGpu *CNvSDIoutGpuTopology::getGpu(int index)
{
	if(index >= 0 && index < m_nGpu)
		return dynamic_cast<CNvSDIoutGpu *>(m_lGpu[index]);
	return NULL;
}

CNvSDIoutGpu *CNvSDIoutGpuTopology::getPrimaryGpu()
{
	for(int i = 0;i < m_nGpu;i++)
	{
		if(m_lGpu[i]->isPrimary())
			return dynamic_cast<CNvSDIoutGpu *>(m_lGpu[i]);
	}
	return NULL;
}

bool  CNvSDIoutGpu::init(HGPUNV gpuAffinityHandle,bool bPrimary, bool bDisplay, bool bSDIoutput, NvVioHandle hVioHandle)
{
	CNvGpu::init(gpuAffinityHandle,bPrimary,bDisplay);
	m_bSDIoutput = bSDIoutput;
	m_hVioHandle = hVioHandle;
	return true;
}

