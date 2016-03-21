 /***************************************************************************\
|*                                                                           *|
|*      Copyright 2007 NVIDIA Corporation.  All rights reserved.             *|
|*                                                                           *|
|*   NOTICE TO USER:                                                         *|
|*                                                                           *|
|*   This source code is subject to NVIDIA ownership rights under U.S.       *|
|*   and international Copyright laws.  Users and possessors of this         *|
|*   source code are hereby granted a nonexclusive, royalty-free             *|
|*   license to use this code in individual and commercial software.         *|
|*                                                                           *|
|*   NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE     *|
|*   CODE FOR ANY PURPOSE. IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR         *|
|*   IMPLIED WARRANTY OF ANY KIND. NVIDIA DISCLAIMS ALL WARRANTIES WITH      *|
|*   REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF         *|
|*   MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR          *|
|*   PURPOSE. IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL,            *|
|*   INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES          *|
|*   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN      *|
|*   AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING     *|
|*   OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOURCE      *|
|*   CODE.                                                                   *|
|*                                                                           *|
|*   U.S. Government End Users. This source code is a "commercial item"      *|
|*   as that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting       *|
|*   of "commercial computer  software" and "commercial computer software    *|
|*   documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995)   *|
|*   and is provided to the U.S. Government only as a commercial end item.   *|
|*   Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through        *|
|*   227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the       *|
|*   source code with only those rights set forth herein.                    *|
|*                                                                           *|
|*   Any use of this source code in individual and commercial software must  *|
|*   include, in the user documentation and internal comments to the code,   *|
|*   the above Disclaimer and U.S. Government End Users Notice.              *|
|*                                                                           *|
|*                                                                           *|
 \***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// ANCAPI.H
//
// Header file for ANCAPI.CPP - This header file implements the NVIDIA GVO
// ancillary data API for SDI.
//
//  This file will be exposed to 3rd party developers
//
// Platforms/OS - Windows XP, linux
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __NVANCAPI_H__
#define __NVANCAPI_H__

#ifdef _WIN32
#include "nvapi.h"
#else
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include "NVCtrlLib.h"
#include "NVCtrl.h"
#endif

//---------------------------------------------------------------------
// NVIDIA Grapics to Video Out (GVO) Ancillary Data API
//---------------------------------------------------------------------

#ifdef  __cplusplus
    extern "C" {
#endif//__cplusplus

#ifndef IN
#   define IN
#endif//IN

#ifndef OUT
#   define OUT
#endif//OUT

#ifndef INOUT
#   define INOUT
#endif//INOUT

#ifdef _WIN32
#define NVVIOANCAPI_INTERFACE extern NvAPI_Status __cdecl 
#else
#define NVVIOANCAPI_INTERFACE NvAPI_Status 
#endif

// Need these nvapi.h defines on linux.
#ifndef _WIN32
typedef unsigned long long NvU64;
typedef unsigned int       NvU32;
typedef unsigned short     NvU16;
typedef long               NvS32;
typedef unsigned char      NvU8;

#define NVAPI_GENERIC_STRING_MAX    4096
#define NVAPI_LONG_STRING_MAX       256
#define NVAPI_SHORT_STRING_MAX      64

typedef char NvAPI_String[NVAPI_GENERIC_STRING_MAX];
typedef char NvAPI_LongString[NVAPI_LONG_STRING_MAX];
typedef char NvAPI_ShortString[NVAPI_SHORT_STRING_MAX];
#endif


//
// NVVIO Handle - NVVIO control handle 
//
#ifndef _WIN32
#ifndef DECLARE_HANDLE
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#endif
DECLARE_HANDLE(NvVioHandle);            // NvVIO Device Handle
#endif

// =========================================================================================
// NvAPI Version Definition
// Maintain per structure specific version define using the MAKE_NVAPI_VERSION macro.
// Usage: #define NVVIOANCDATAFRAME_VERSION  MAKE_NVAPI_VERSION(NVVIOANCDATAFRAME, 1)
// =========================================================================================
#define MAKE_NVAPI_VERSION(typeName,ver) (NvU32)(sizeof(typeName) | ((ver)<<16))
#define GET_NVAPI_VERSION(ver) (NvU32)((ver)>>16)
#define GET_NVAPI_SIZE(ver) (NvU32)((ver) & 0xffff)

//---------------------------------------------------------------------
// Types
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Enumerations
//---------------------------------------------------------------------

// ====================================================
// NvAPI Status Values
//    All NvAPI functions return one of these codes.
// ====================================================
#ifndef _WIN32
#ifndef NvAPI_Status
typedef enum 
{
    NVAPI_OK                            =  0,      // Success
    NVAPI_ERROR                         = -1,      // Generic error
    NVAPI_LIBRARY_NOT_FOUND             = -2,      // nvapi.dll can not be loaded
    NVAPI_NO_IMPLEMENTATION             = -3,      // not implemented in current driver installation
    NVAPI_API_NOT_INTIALIZED            = -4,      // NvAPI_Initialize has not been called (successfully)
    NVAPI_INVALID_ARGUMENT              = -5,      // invalid argument
    NVAPI_NVIDIA_DEVICE_NOT_FOUND       = -6,      // no NVIDIA display driver was found
    NVAPI_END_ENUMERATION               = -7,      // no more to enum
    NVAPI_INVALID_HANDLE                = -8,      // invalid handle
    NVAPI_INCOMPATIBLE_STRUCT_VERSION   = -9,      // an argument's structure version is not supported
    NVAPI_NOT_SUPPORTED                 = -10,     // Requested feature not supported in the selected GPU
    NVAPI_PORTID_NOT_FOUND              = -11      // NO port ID found for I2C transaction
} NvAPI_Status;
#endif
#endif

// Audio sample rate definitions - from SMPTE 299M-2004 Table 8
typedef enum
{
    NVVIOANCAUDIO_SAMPLING_RATE_48_0         = 0x0,
    NVVIOANCAUDIO_SAMPLING_RATE_44_1         = 0x1,
    NVVIOANCAUDIO_SAMPLING_RATE_32_0         = 0x2,
    NVVIOANCAUDIO_SAMPLING_RATE_FREE_RUNNING = 0x7
} NVVIOANCAUDIO_SAMPLE_RATE;

// Active channel definitions - from SMPTE 299M-2004 Table 9
typedef enum
{
    NVVIOANCAUDIO_ACTIVE_CH1    = 0x1,
    NVVIOANCAUDIO_ACTIVE_CH2    = 0x2,
    NVVIOANCAUDIO_ACTIVE_CH3    = 0x4,
    NVVIOANCAUDIO_ACTIVE_CH4    = 0x8
} NVVIOANCAUDIO_ACTIVE_CHANNEL;


//---------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------

// Audio control 
typedef struct tagNVVIOANCAUDIOCNTRL {
    NvU32 version;                   // Structure version
    NvU8 frameNumber1_2;             // Frame number for channels 1 and 2
	NvU8 frameNumber3_4;             // Frame number for channels 3 and 4
    NVVIOANCAUDIO_SAMPLE_RATE rate;  // Audio sample rate
    NvU8 asynchronous;               // 0 = synchronous, 1 = asynchronous
    NvU8 activeChannels;             // Bitwise OR of active channel definitions
} NVVIOANCAUDIOCNTRL;

#define NVVIOANCAUDIOCNTRL_VERSION  MAKE_NVAPI_VERSION(NVVIOANCAUDIOCNTRL, 2)

// Audio group
typedef struct tagNVVIOANCAUDIOGROUP {
    NvU32 numAudioSamples;           // Number of valid audio samples / channel
    NvU32  *audioData[4];	         // Data pointer for audio channels 1-4
    NVVIOANCAUDIOCNTRL audioCntrl;   // Controls for audio channels 1-4	
} NVVIOANCAUDIOGROUP;

#define NVVIOANCAUDIOGROUP_VERSION  MAKE_NVAPI_VERSION(NVVIOANCAUDIOGROUP, 1)

// Per ANC Data Packet
typedef struct tagNVVIOANCDATAPACKET {
    NvU32 version;               // Structure version
    NvU16 DID;
    NvU16 SDID;
    NvU16 DC;
    NvU8  data[255];             // Should this be unsigned short?
    NvU16 CS;
} NVVIOANCDATAPACKET;

#define NVVIOANCDATAPACKET_VERSION  MAKE_NVAPI_VERSION(NVVIOANCDATAPACKET, 1)

// Active Format Description and Bar Data
typedef struct tagNVVIOANCAFD {
	NvU8  AFD;           // AFD
	NvU8  BDF;           // Bar Data Flags
    NvU32 bdv1;          // Bar Data Value 1
    NvU32 bdv2;	         // Bar Data Value 2
} NVVIOANCAFD;

// Data field mask definitions (Indicate NVVIOANCDATAFRAME fields in use)
#define NVVIOANCDATAFRAME_AUDIO_GROUP_1     0x00000001
#define NVVIOANCDATAFRAME_AUDIO_GROUP_2     0x00000002
#define NVVIOANCDATAFRAME_AUDIO_GROUP_3     0x00000004
#define NVVIOANCDATAFRAME_AUDIO_GROUP_4     0x00000008
#define NVVIOANCDATAFRAME_LTC               0x00000010
#define NVVIOANCDATAFRAME_VITC              0x00000020
#define NVVIOANCDATAFRAME_FILM_TC           0x00000040
#define NVVIOANCDATAFRAME_PROD_TC           0x00000080
#define NVVIOANCDATAFRAME_FRAME_ID          0x00000100
#define NVVIOANCDATAFRAME_CUSTOM            0x00000200
#define NVVIOANCDATAFRAME_AFD               0x00000400

// Per Frame
typedef struct tagNVVIOANCDATAFRAME {
    NvU32 version;		             // Structure version
    NvU32 fields;                    // Field mask
    NVVIOANCAUDIOGROUP  AudioGroup1; // Audio group 1
    NVVIOANCAUDIOGROUP  AudioGroup2; // Audio group 2
	NVVIOANCAUDIOGROUP  AudioGroup3; // Audio group 3
	NVVIOANCAUDIOGROUP  AudioGroup4; // Audio group 4
    NvU32 LTCTimecode;               // RP188
    NvU32 LTCUserBytes;
    NvU32 VITCTimecode;
    NvU32 VITCUserBytes;
    NvU32 FilmTimecode;
    NvU32 FilmUserBytes;
    NvU32 ProductionTimecode;        // RP201
    NvU32 ProductionUserBytes;       // RP201
    NVVIOANCAFD ActiveFormatDescript;// Active format description / bar data
    NvU32 FrameID;
    NvU32 numCustomPackets;
    NVVIOANCDATAPACKET *CustomPackets;
} NVVIOANCDATAFRAME;

#define NVVIOANCDATAFRAME_VERSION  MAKE_NVAPI_VERSION(NVVIOANCDATAFRAME, 3)

// Per Sequence
typedef struct tagNVVIOANCDATACONFIG {
    NvU32 version;               // Structure version
    NvU32 numAudioChannels;
    NvU32 audioRate;
} NVVIOANCDATACONFIG;

#define NVVIOANCDATACONFIG_VERSION  MAKE_NVAPI_VERSION(NVVIOANCDATACONFIG, 1)

//---------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvVIOANCAPI_InitializeGVO
//
//   DESCRIPTION: Initializes NV GVO ancillary data library. This function must be 
//                called before any other NV GVO ancillary data library function.
//                This function queries the current video device state and
//                initializes all internal data structures.
//                
//
// RETURN STATUS: NVAPI_ERROR            Something is wrong during the initialization process (generic error)
//                NVAPI_LIBRARYNOTFOUND  Can not load nvapi.dll
//                NVAPI_OK               Initialized
//
///////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
NVVIOANCAPI_INTERFACE NvVIOANCAPI_InitializeGVO(NvVioHandle hVIO);
#else
NVVIOANCAPI_INTERFACE NvVIOANCAPI_InitializeGVO(Display *dpy, int target_id);
#endif

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvVIOANCAPI_InitializeGVI
//
//   DESCRIPTION: Initializes NV GVI ancillary data library. This function must be 
//                called before any other NV GVI ancillary data library function.
//                This function queries the current video device state and
//                initializes all internal data structures.
//                
//
// RETURN STATUS: NVAPI_ERROR            Something is wrong during the initialization process (generic error)
//                NVAPI_LIBRARYNOTFOUND  Can not load nvapi.dll
//                NVAPI_OK               Initialized
//
///////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
NVVIOANCAPI_INTERFACE NvVIOANCAPI_InitializeGVI(NvVioHandle hVIO);
#else
NVVIOANCAPI_INTERFACE NvVIOANCAPI_InitializeGVI(Display *dpy, int target_id);
#endif

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvVIOANCAPI_ReleaseGVO
//
//   DESCRIPTION: Releases NV GVO ancillary data library. This function must be 
//                called to release all NV GVO ancillary data library resources.
//                
//
// RETURN STATUS: NVAPI_ERROR            Something went wrong
//                NVAPI_OK               All resources released
//
///////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
NVVIOANCAPI_INTERFACE NvVIOANCAPI_ReleaseGVO(NvVioHandle hVIO);
#else
NVVIOANCAPI_INTERFACE NvVIOANCAPI_ReleaseGVO(Display *dpy, int target_id);
#endif


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvVIOANCAPI_ReleaseGVI
//
//   DESCRIPTION: Releases NV GVI ancillary data library. This function must be 
//                called to release all NV GVI ancillary data library resources.
//                
//
// RETURN STATUS: NVAPI_ERROR            Something went wrong
//                NVAPI_OK               All resources released
//
///////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
NVVIOANCAPI_INTERFACE NvVIOANCAPI_ReleaseGVI(NvVioHandle hVIO);
#else
NVVIOANCAPI_INTERFACE NvVIOANCAPI_ReleaseGVI(Display *dpy, int target_id);
#endif


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvVIOANCAPI_GetErrorMessage
//
//   DESCRIPTION: converts an NVVIOANCAPI error code into a null terminated string
//
// RETURN STATUS: null terminated string (always, never NULL)
//
///////////////////////////////////////////////////////////////////////////////
NVVIOANCAPI_INTERFACE NvVIOANCAPI_GetErrorMessage(NvAPI_Status nr,NvAPI_ShortString szDesc);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvVIOANCAPI_GetInterfaceVersionString
//
//   DESCRIPTION: Returns a string describing the version of the NVVIOANCAPI library.
//                Contents of the string are human readable.  Do not assume a fixed
//                format.
//
// RETURN STATUS: User readable string giving info on NvAPI's version
//
///////////////////////////////////////////////////////////////////////////////
NVVIOANCAPI_INTERFACE NvVIOANCAPI_GetInterfaceVersionString(NvAPI_ShortString szVersion);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvVIOANCAPI_SendANCData
//
//   DESCRIPTION: Sends ancillary data for current field or frame.
//
// RETURN STATUS: NVAPI_ERROR
//                NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////
NVVIOANCAPI_INTERFACE NvVIOANCAPI_SendANCData(NvVioHandle handle, NVVIOANCDATAFRAME *data);


///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvVIOANCAPI_NumAudioSamples
//
//   DESCRIPTION: When called with sequenceLength of 0 and/or numSamples
//                specified as NULL, returns the sample sequence length in
//                sequenceLength.  Otherwise, returns the number of
//                audio samples for each frame in the specified sequenceLength.
//                In this case, the parameter numSamples must point to an 
//                array of adequate size to hold the number of samples for
//                each frame in the sequence.
//
// RETURN STATUS: NVAPI_ERROR
//                NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////
NVVIOANCAPI_INTERFACE NvVIOANCAPI_NumAudioSamples(NvVioHandle handle, 
												  NVVIOANCAUDIO_SAMPLE_RATE audioRate,
												  NvU32 *sequenceLength,
												  NvU32 *numSamples);

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvVIOANCAPI_CaptureANCData
//
//   DESCRIPTION: Capture ancillary data for current field or frame.
//
// RETURN STATUS: NVAPI_ERROR
//                NVAPI_OK
//
///////////////////////////////////////////////////////////////////////////////
NVVIOANCAPI_INTERFACE NvVIOANCAPI_CaptureANCData(NvVioHandle handle, NVVIOANCDATAFRAME *data);

#ifdef __cplusplus
}; //extern "C" {
#endif

#endif  // __NVANCAPI_H__
