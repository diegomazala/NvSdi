 /***************************************************************************\
|*                                                                           *|
|*      Copyright 1993-2005 NVIDIA Corporation.  All rights reserved.        *|
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
// NVGVOAPI.H
//
// Header file for NVGVOAPI.CPP - This header file implements the nv control panel
// API for SDI
//
//  This file will be exposed to 3rd party developers
//
// Platforms/OS - Windows NT4, 9X, ME, 2K, XP
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __NVGVOAPI_H__
#define __NVGVOAPI_H__

//---------------------------------------------------------------------
// NVIDIA Grapics to Video Out (GVO) API
//---------------------------------------------------------------------

#ifdef  __cplusplus
    extern "C" {
#endif//__cplusplus

#include "NvApiError.h"

#ifndef IN
#   define IN
#endif//IN

#ifndef OUT
#   define OUT
#endif//OUT

#ifndef INOUT
#   define INOUT
#endif//INOUT

#ifndef NVFLOAT
typedef float NVFLOAT;
#endif//NVFLOAT

#ifndef NVADAPTERNAME_MAXLEN 
#define NVADAPTERNAME_MAXLEN    256
#endif//NVADAPTERNAME_MAXLEN

#ifndef NVVALUENAME_MAXLEN
#define NVVALUENAME_MAXLEN      256
#endif//NVVALUENAME_MAXLEN

//---------------------------------------------------------------------
// Types
//---------------------------------------------------------------------

typedef UINT NVGVOHANDLE;                                   // Handle from NvGvoOpen() or NvGvoDesktopOpen()
#define INVALID_NVGVOHANDLE                 0               // Invalid NVGVOHANDLE

typedef DWORD NVGVOOWNERID;                                 // Unique identifier for owner of Graphics to Video output (process identifier or NVGVOOWNERID_NONE)
#define NVGVOOWNERID_NONE                   0               // Unregistered ownerId

enum NVGVOOWNERTYPE                                         // Owner type for device
{
    NVGVOOWNERTYPE_NONE                             ,       //  No owner for device
    NVGVOOWNERTYPE_OPENGL                           ,       //  OpenGL application owns device
    NVGVOOWNERTYPE_DESKTOP                          ,       //  Desktop transparent mode owns device
};

// Access rights for NvGvoOpen() or NvGvoDesktopOpen()
#define NVGVO_O_READ                        0x00000000      // Read access
#define NVGVO_O_WRITE_EXCLUSIVE             0x00010001      // Write exclusive access

//---------------------------------------------------------------------
// Enumerations
//---------------------------------------------------------------------

// Video signal format and resolution
enum NVGVOSIGNALFORMAT
{
    NVGVOSIGNALFORMAT_ERROR = -1                    ,       // Invalid signal format 
    NVGVOSIGNALFORMAT_487I_5994_SMPTE259_NTSC       ,       // 01  487i    59.94Hz  (SMPTE259) NTSC
    NVGVOSIGNALFORMAT_576I_5000_SMPTE259_PAL        ,       // 02  576i    50.00Hz  (SMPTE259) PAL
    NVGVOSIGNALFORMAT_720P_5994_SMPTE296            ,       // 03  720p    59.94Hz  (SMPTE296)
    NVGVOSIGNALFORMAT_720P_6000_SMPTE296            ,       // 04  720p    60.00Hz  (SMPTE296)
    NVGVOSIGNALFORMAT_1035I_5994_SMPTE260           ,       // 05  1035i   59.94Hz  (SMPTE260)
    NVGVOSIGNALFORMAT_1035I_6000_SMPTE260           ,       // 06  1035i   60.00Hz  (SMPTE260)
    NVGVOSIGNALFORMAT_1080I_5000_SMPTE295           ,       // 07  1080i   50.00Hz  (SMPTE295)
    NVGVOSIGNALFORMAT_1080I_5000_SMPTE274           ,       // 08  1080i   50.00Hz  (SMPTE274)
    NVGVOSIGNALFORMAT_1080I_5994_SMPTE274           ,       // 09  1080i   59.94Hz  (SMPTE274)
    NVGVOSIGNALFORMAT_1080I_6000_SMPTE274           ,       // 10  1080i   60.00Hz  (SMPTE274)
    NVGVOSIGNALFORMAT_1080PSF_23976_SMPTE274        ,       // 11  1080PsF 23.976Hz (SMPTE274)
    NVGVOSIGNALFORMAT_1080PSF_2400_SMPTE274         ,       // 12  1080PsF 24.00Hz  (SMPTE274)
    NVGVOSIGNALFORMAT_1080PSF_2500_SMPTE274         ,       // 13  1080PsF 25.00Hz  (SMPTE274)
    NVGVOSIGNALFORMAT_1080PSF_3000_SMPTE274         ,       // 14  1080PsF 30.00Hz  (SMPTE274)
    NVGVOSIGNALFORMAT_1080P_23976_SMPTE274          ,       // 15  1080p   23.976Hz (SMPTE274)
    NVGVOSIGNALFORMAT_1080P_2400_SMPTE274           ,       // 16  1080p   24.00Hz  (SMPTE274)
    NVGVOSIGNALFORMAT_1080P_2500_SMPTE274           ,       // 17  1080p   25.00Hz  (SMPTE274)
    NVGVOSIGNALFORMAT_1080P_2997_SMPTE274           ,       // 18  1080p   29.97Hz  (SMPTE274)
    NVGVOSIGNALFORMAT_1080P_3000_SMPTE274           ,       // 19  1080p   30.00Hz  (SMPTE274)
    NVGVOSIGNALFORMAT_1080PSF_2997_SMPTE274         ,       // 20  1080PsF 29.97Hz  (SMPTE274)

    NVGVOSIGNALFORMAT_720P_5000_SMPTE296            ,       // 21  720p    50.00Hz  (SMPTE296)
    NVGVOSIGNALFORMAT_720P_3000_SMPTE296            ,       // 22  720p    30.00Hz  (SMPTE296)
    NVGVOSIGNALFORMAT_720P_2997_SMPTE296            ,       // 23  720p    29.97Hz  (SMPTE296)
    NVGVOSIGNALFORMAT_720P_2500_SMPTE296            ,       // 24  720p    25.00Hz  (SMPTE296)
    NVGVOSIGNALFORMAT_720P_2400_SMPTE296            ,       // 25  720p    24.00Hz  (SMPTE296)
    NVGVOSIGNALFORMAT_720P_2398_SMPTE296            ,       // 26  720p    23.98Hz  (SMPTE296)

    NVGVOSIGNALFORMAT_1080I_4800_SMPTE274           ,       // 27  1080i   48.00Hz  (SMPTE296)
    NVGVOSIGNALFORMAT_1080I_4796_SMPTE274           ,       // 28  1080i   47.96Hz  (SMPTE296)
    NVGVOSIGNALFORMAT_1080PSF_2398_SMPTE274         ,       // 29  1080PsF 23.98Hz  (SMPTE296)

    NVGVOSIGNALFORMAT_2048P_3000_SMPTE372           ,       // 30  2048P   30.00Hz  (SMPTE372)
    NVGVOSIGNALFORMAT_2048P_2997_SMPTE372           ,       // 31  2048P   29.97Hz  (SMPTE372)
    NVGVOSIGNALFORMAT_2048I_6000_SMPTE372           ,       // 32  2048I   60.00Hz  (SMPTE372)
    NVGVOSIGNALFORMAT_2048I_5994_SMPTE372           ,       // 33  2048I   59.94Hz  (SMPTE372)
    NVGVOSIGNALFORMAT_2048P_2500_SMPTE372           ,       // 34  2048P   25.00Hz  (SMPTE372)
    NVGVOSIGNALFORMAT_2048I_5000_SMPTE372           ,       // 35  2048I   50.00Hz  (SMPTE372)
    NVGVOSIGNALFORMAT_2048P_2400_SMPTE372           ,       // 36  2048P   24.00Hz  (SMPTE372)
    NVGVOSIGNALFORMAT_2048I_4800_SMPTE372           ,       // 37  2048I   48.00Hz  (SMPTE372)
    NVGVOSIGNALFORMAT_2048P_2398_SMPTE372           ,       // 38  2048P   23.98Hz  (SMPTE372)
    NVGVOSIGNALFORMAT_2048I_4796_SMPTE372           ,       // 39  2048I   47.96Hz  (SMPTE372)

    NVGVOSIGNALFORMAT_END                                   // 40  To indicate end of signal format list
};

// SMPTE standards format
enum NVVIDEOSTANDARD
{
    NVVIDEOSTANDARD_SMPTE259                        ,       // SMPTE259
    NVVIDEOSTANDARD_SMPTE260                        ,       // SMPTE260
    NVVIDEOSTANDARD_SMPTE274                        ,       // SMPTE274
    NVVIDEOSTANDARD_SMPTE295                        ,       // SMPTE295
    NVVIDEOSTANDARD_SMPTE296                        ,       // SMPTE296
    NVVIDEOSTANDARD_SMPTE372                        ,       // SMPTE372
};

// HD or SD video type
enum NVVIDEOTYPE
{
    NVVIDEOTYPE_SD                                  ,       // Standard-definition (SD)
    NVVIDEOTYPE_HD                                  ,       // High-definition     (HD)
};

// Interlace mode
enum NVINTERLACEMODE 
{
    NVINTERLACEMODE_PROGRESSIVE                     ,       // Progressive               (p)
    NVINTERLACEMODE_INTERLACE                       ,       // Interlace                 (i)
    NVINTERLACEMODE_PSF                             ,       // Progressive Segment Frame (psf)
};

// Video data format
enum NVGVODATAFORMAT
{
    NVGVODATAFORMAT_UNKNOWN = -1                    ,
    NVGVODATAFORMAT_R8G8B8_TO_YCRCB444              ,       // R8:G8:B8                => YCrCb  (4:4:4)
    NVGVODATAFORMAT_R8G8B8A8_TO_YCRCBA4444          ,       // R8:G8:B8:A8             => YCrCbA (4:4:4:4)
    NVGVODATAFORMAT_R8G8B8Z10_TO_YCRCBZ4444         ,       // R8:G8:B8:Z10            => YCrCbZ (4:4:4:4)
    NVGVODATAFORMAT_R8G8B8_TO_YCRCB422              ,       // R8:G8:B8                => YCrCb  (4:2:2)
    NVGVODATAFORMAT_R8G8B8A8_TO_YCRCBA4224          ,       // R8:G8:B8:A8             => YCrCbA (4:2:2:4)
    NVGVODATAFORMAT_R8G8B8Z10_TO_YCRCBZ4224         ,       // R8:G8:B8:Z10            => YCrCbZ (4:2:2:4)
    NVGVODATAFORMAT_R8G8B8_TO_RGB444                ,       // R8:G8:B8                => RGB    (4:4:4)
    NVGVODATAFORMAT_R8G8B8A8_TO_RGBA4444            ,       // R8:G8:B8:A8             => RGBA   (4:4:4:4)
    NVGVODATAFORMAT_R8G8B8Z10_TO_RGBZ4444           ,       // R8:G8:B8:Z10            => RGBZ   (4:4:4:4)
    NVGVODATAFORMAT_Y10CR10CB10_TO_YCRCB444         ,       // Y10:CR10:CB10           => YCrCb  (4:4:4)
    NVGVODATAFORMAT_Y10CR8CB8_TO_YCRCB444           ,       // Y10:CR8:CB8             => YCrCb  (4:4:4)
    NVGVODATAFORMAT_Y10CR8CB8A10_TO_YCRCBA4444      ,       // Y10:CR8:CB8:A10         => YCrCbA (4:4:4:4)
    NVGVODATAFORMAT_Y10CR8CB8Z10_TO_YCRCBZ4444      ,       // Y10:CR8:CB8:Z10         => YCrCbZ (4:4:4:4)
    NVGVODATAFORMAT_DUAL_R8G8B8_TO_DUAL_YCRCB422    ,       // R8:G8:B8 + R8:G8:B8     => YCrCb  (4:2:2 + 4:2:2)
    NVGVODATAFORMAT_DUAL_Y8CR8CB8_TO_DUAL_YCRCB422  ,       // Y8:CR8:CB8 + Y8:CR8:CB8 => YCrCb  (4:2:2 + 4:2:2)
    NVGVODATAFORMAT_R10G10B10_TO_YCRCB422           ,       // R10:G10:B10             => YCrCb  (4:2:2)
    NVGVODATAFORMAT_R10G10B10_TO_YCRCB444           ,       // R10:G10:B10             => YCrCb  (4:4:4)
    NVGVODATAFORMAT_Y12CR12CB12_TO_YCRCB444         ,       // Y12:CR12:CB12           => YCrCb  (4:4:4)
    NVGVODATAFORMAT_Y12CR12CB12_TO_YCRCB422         ,       // Y12:CR12:CB12           => YCrCb  (4:2:2)
    NVGVODATAFORMAT_Y10CR10CB10_TO_YCRCB422         ,       // Y10:CR10:CB10           => YCrCb  (4:2:2)
    NVGVODATAFORMAT_Y8CR8CB8_TO_YCRCB422            ,       // Y8:CR8:CB8              => YCrCb  (4:2:2)
    NVGVODATAFORMAT_Y10CR8CB8A10_TO_YCRCBA4224      ,       // Y10:CR8:CB8:A10         => YCrCbA (4:2:2:4)
    NVGVODATAFORMAT_R10G10B10_TO_RGB444             ,       // R10:G10:B10             => RGB    (4:4:4)
    NVGVODATAFORMAT_R12G12B12_TO_RGB444             ,       // R12:G12:B12             => RGB    (4:4:4)
};

// Video output area
enum NVGVOOUTPUTAREA
{
    NVGVOOUTPUTAREA_FULLSIZE                        ,       // Output to entire video resolution (full size)
    NVGVOOUTPUTAREA_SAFEACTION                      ,       // Output to centered 90% of video resolution (safe action)
    NVGVOOUTPUTAREA_SAFETITLE                       ,       // Output to centered 80% of video resolution (safe title)
};

// Synchronization source
enum NVGVOSYNCSOURCE
{
    NVGVOSYNCSOURCE_SDISYNC                         ,       // SDI Sync  (Digital input)
    NVGVOSYNCSOURCE_COMPSYNC                        ,       // COMP Sync (Composite input)
};

// Composite synchronization type
enum NVGVOCOMPSYNCTYPE
{
    NVGVOCOMPSYNCTYPE_AUTO                          ,       // Auto-detect
    NVGVOCOMPSYNCTYPE_BILEVEL                       ,       // Bi-level signal
    NVGVOCOMPSYNCTYPE_TRILEVEL                      ,       // Tri-level signal
};

// Video output status
enum NVGVOOUTPUTSTATUS
{
    NVGVOOUTPUTSTATUS_OFF                           ,       // Output not in use
    NVGVOOUTPUTSTATUS_ERROR                         ,       // Error detected
    NVGVOOUTPUTSTATUS_SDI_SD                        ,       // SDI output (standard-definition)
    NVGVOOUTPUTSTATUS_SDI_HD                        ,       // SDI output (high-definition)
};

// Synchronization input status
enum NVGVOSYNCSTATUS
{
    NVGVOSYNCSTATUS_OFF                             ,       // Sync not detected
    NVGVOSYNCSTATUS_ERROR                           ,       // Error detected
    NVGVOSYNCSTATUS_SYNCLOSS                        ,       // Genlock in use, format mismatch with output
    NVGVOSYNCSTATUS_COMPOSITE                       ,       // Composite sync
    NVGVOSYNCSTATUS_SDI_SD                          ,       // SDI sync (standard-definition)
    NVGVOSYNCSTATUS_SDI_HD                          ,       // SDI sync (high-definition)
};

//---------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------

#define NVGVOCAPS_VIDOUT_SDI                0x00000001      // Supports Serial Digital Interface (SDI) output
#define NVGVOCAPS_SYNC_INTERNAL             0x00000100      // Supports Internal timing source
#define NVGVOCAPS_SYNC_GENLOCK              0x00000200      // Supports Genlock timing source
#define NVGVOCAPS_SYNCSRC_SDI               0x00001000      // Supports Serial Digital Interface (SDI) synchronization input
#define NVGVOCAPS_SYNCSRC_COMP              0x00002000      // Supports Composite synchronization input
#define NVGVOCAPS_OUTPUTMODE_DESKTOP        0x00010000      // Supports Desktop transparent mode
#define NVGVOCAPS_OUTPUTMODE_OPENGL         0x00020000      // Supports OpenGL application mode

#define NVGVOCLASS_SDI                      0x00000001      // SDI-class interface: SDI output with two genlock inputs


struct  NVGVODRIVER                                                  
{                                                           // Driver version
    WORD    wMajorVersion;                                  //  Major version
    WORD    wMinorVersion;                                  //  Minor version
    WORD    wRevision;                                      //  Revision
    WORD    wBuild;                                         //  Build
};

struct  NVGVOFIRMWARE                                                  
{                                                       // Firmware version
    WORD          wMajorVersion;                        //  Major version
    WORD          wMinorVersion;                        //  Minor version
};

// Device capabilities
struct  NVGVOCAPS
{
    WORD              cbSize;                               // Caller sets to sizeof(NVGVOCAPS)
    char              szAdapterName[NVADAPTERNAME_MAXLEN];  // Graphics adapter name
    DWORD             dwClass;                              // Graphics adapter classes (NGVOCLASS_* mask)
    DWORD             dwCaps;                               // Graphics adapter capabilities (NVGVOCAPS_* mask)
    DWORD             dwDipSwitch;                          // On-board DIP switch settings bits
    DWORD             dwDipSwitchReserved;                  // On-board DIP switch settings reserved bits

    NVGVODRIVER       Driver;                               //
    NVGVOFIRMWARE     Firmware;                             //

    NVGVOOWNERID      ownerId;                              // Unique identifier for owner of video output (NVGVOOWNERID_NONE if free running)
    NVGVOOWNERTYPE    ownerType;                            // Owner type for video output (OpenGL application or Desktop mode)
};

// Device status
struct  NVGVOSTATUS
{
    WORD              cbSize;                               // Caller sets to sizeof(NVGVOSTATUS)
    NVGVOOUTPUTSTATUS vid1Out;                              // Video 1 output status
    NVGVOOUTPUTSTATUS vid2Out;                              // Video 2 output status
    NVGVOSYNCSTATUS   sdiSyncIn;                            // SDI sync input status
    NVGVOSYNCSTATUS   compSyncIn;                           // Composite sync input status
    BOOL              syncEnable;                           // Sync enable (TRUE if using syncSource)
    NVGVOSYNCSOURCE   syncSource;                           // Sync source
    NVGVOSIGNALFORMAT syncFormat;                           // Sync format
    NVGVOOWNERID      ownerId;                              // Unique identifier for owner of video output
    NVGVOOWNERTYPE    ownerType;                            // Owner type for video output (OpenGL application or Desktop mode)
    BOOL              bframeLockEnable;                     // Framelock enable flag
    BOOL              bOutputVideoLocked;                   // Output video timing locked status
    int               nDataIntegrityCheckErrorCount;        // Data integrity check error count
    BOOL              bDataIntegrityCheckEnabled;           // Data integrity check status enabled 
    BOOL              bDataIntegrityCheckFailed;            // Data integrity check status failed 
    BOOL              bSyncSourceLocked;                    // genlocked to framelocked to ref signal
    BOOL              bPowerOn;                             // TRUE: indicates there is sufficient power
};


// Output region
struct NVGVOOUTPUTREGION
{
    WORD              x;                                    // Horizontal origin in pixels
    WORD              y;                                    // Vertical origin in pixels
    WORD              width;                                // Width of region in pixels
    WORD              height;                               // Height of region in pixels
};

// Gamma ramp (8-bit index)
typedef struct tagNVGAMMARAMP8
{
    WORD              cbSize;                               // Caller sets to sizeof(NVGAMMARAMP8)
    WORD              wRed[256];                            // Red channel gamma ramp (8-bit index, 16-bit values)
    WORD              wGreen[256];                          // Green channel gamma ramp (8-bit index, 16-bit values)
    WORD              wBlue[256];                           // Blue channel gamma ramp (8-bit index, 16-bit values)
} NVGAMMARAMP8;

// Gamma ramp (10-bit index)
typedef struct tagNVGAMMARAMP10
{
    WORD              cbSize;                               // Caller sets to sizeof(NVGAMMARAMP10)
    WORD              wRed[1024];                           // Red channel gamma ramp (10-bit index, 16-bit values)
    WORD              wGreen[1024];                         // Green channel gamma ramp (10-bit index, 16-bit values)
    WORD              wBlue[1024];                          // Blue channel gamma ramp (10-bit index, 16-bit values)
} NVGAMMARAMP10;


// Sync delay
typedef struct tagNVGVOSYNCDELAY
{
    WORD              wHorizontalDelay;                     // Horizontal delay in pixels
    WORD              wVerticalDelay;                       // Vertical delay in lines
} NVGVOSYNCDELAY;


// Video mode information
struct  NVVIDEOMODE
{
    DWORD             dwHorizontalPixels;                   // Horizontal resolution (in pixels)
    DWORD             dwVerticalLines;                      // Vertical resolution for frame (in lines)
    NVFLOAT           fFrameRate;                           // Frame rate
    NVINTERLACEMODE   interlaceMode;                        // Interlace mode 
    NVVIDEOSTANDARD   videoStandard;                        // SMPTE standards format
    NVVIDEOTYPE       videoType;                            // HD or SD signal classification
};


// Signal format details
struct  NVGVOSIGNALFORMATDETAIL
{   
    WORD              cbSize;                               // Caller sets to sizeof(NVGVOSIGNALFORMATDETAIL)
    NVGVOSIGNALFORMAT signalFormat;                         // Signal format enumerated value
    char              szValueName[NVVALUENAME_MAXLEN];      // Signal format name, in the form:
                                                            //  <name>\t<rate>\tHz\t(<standard>)[\t<description>]
                                                            //  "480i\t59.94\tHz\t(SMPTE259)\tNTSC"
                                                            //  "1080i\t50.00\tHz\t(SMPTE274)"
    char              szAlternateName[NVVALUENAME_MAXLEN];  // Signal format alternate name (or empty string):
                                                            //  "1080PsF\t25.00\tHz\t(SMPTE274)"
    NVVIDEOMODE       videoMode;                            // Video mode for signal format
}; 


// P-buffer formats
#define NVGVOPBUFFERFORMAT_R8G8B8                  0x00000001   // R8:G8:B8
#define NVGVOPBUFFERFORMAT_R8G8B8Z24               0x00000002   // R8:G8:B8:Z24
#define NVGVOPBUFFERFORMAT_R8G8B8A8                0x00000004   // R8:G8:B8:A8       
#define NVGVOPBUFFERFORMAT_R8G8B8A8Z24             0x00000008   // R8:G8:B8:A8:Z24
#define NVGVOPBUFFERFORMAT_R16FPG16FPB16FP         0x00000010   // R16FP:G16FP:B16FP
#define NVGVOPBUFFERFORMAT_R16FPG16FPB16FPZ24      0x00000020   // R16FP:G16FP:B16FP:Z24
#define NVGVOPBUFFERFORMAT_R16FPG16FPB16FPA16FP    0x00000040   // R16FP:G16FP:B16FP:A16FP
#define NVGVOPBUFFERFORMAT_R16FPG16FPB16FPA16FPZ24 0x00000080   // R16FP:G16FP:B16FP:A16FP:Z24

// Data format details
struct  NVGVODATAFORMATDETAIL
{
    WORD              cbSize;                               // Caller sets to sizeof(NVGVODATAFORMATDETAIL)
    NVGVODATAFORMAT   dataFormat;                           // Data format enumerated value
    DWORD             dwCaps;                               // Data format capabilities (NVGVOCAPS_* mask)

    struct
    {
        DWORD         dwPbufferFormats;                     // Supported p-buffer formats (NVGVOPBUFFERFORMAT_* mask)
        DWORD         dwPbufferCount;                       // Number of p-buffers
        char          szValueName[NVVALUENAME_MAXLEN];      // Data format input name, in the form:
                                                            //  <name>
                                                            //  "R8:G8:B8:A8"
    } in;

    struct
    {
        char          szValueName[NVVALUENAME_MAXLEN];      // Data format output name, in the form:
                                                            //  <name>\t<format>
                                                            //  "YCrCbA\t(4:2:2:4)"
    } out;
};

// Device configuration (dwFields masks indicating NVGVOCONFIG fields to use for NvGvoGet/Set/Test/CreateDefaultConfig())
#define NVGVOCONFIG_SIGNALFORMAT            0x00000001      // dwFields: signalFormat
#define NVGVOCONFIG_DATAFORMAT              0x00000002      // dwFields: dataFormat
#define NVGVOCONFIG_OUTPUTREGION            0x00000004      // dwFields: outputRegion
#define NVGVOCONFIG_OUTPUTAREA              0x00000008      // dwFields: outputArea
#define NVGVOCONFIG_COLORCONVERSION         0x00000010      // dwFields: colorConversion
#define NVGVOCONFIG_GAMMACORRECTION         0x00000020      // dwFields: gammaCorrection
#define NVGVOCONFIG_SYNCSOURCEENABLE        0x00000040      // dwFields: syncSource and syncEnable
#define NVGVOCONFIG_SYNCDELAY               0x00000080      // dwFields: syncDelay
#define NVGVOCONFIG_COMPOSITESYNCTYPE       0x00000100      // dwFields: compositeSyncType
#define NVGVOCONFIG_FRAMELOCKENABLE         0x00000200      // dwFields: EnableFramelock
#define NVGVOCONFIG_422FILTER               0x00000400      // dwFields: bEnable422Filter
#define NVGVOCONFIG_COMPOSITETERMINATE      0x00000800      // dwFields: bCompositeTerminate
#define NVGVOCONFIG_DATAINTEGRITYCHECK      0x00001000      // dwFields: bEnableDataIntegrityCheck
#define NVGVOCONFIG_CSCOVERRIDE             0x00002000      // dwFields: colorConversion override
#define NVGVOCONFIG_FLIPQUEUELENGTH         0x00004000      // dwFields: flipqueuelength control
#define NVGVOCONFIG_ANCTIMECODEGENERATION   0x00008000      // dwFields: bEnableANCTimeCodeGeneration
#define NVGVOCONFIG_COMPOSITE               0x00010000      // dwFields: bEnableComposite
#define NVGVOCONFIG_ALPHAKEYCOMPOSITE       0x00020000      // dwFields: bEnableAlphaKeyComposite
#define NVGVOCONFIG_COMPOSITE_Y             0x00040000      // dwFields: compRange
#define NVGVOCONFIG_COMPOSITE_CR            0x00080000      // dwFields: compRange
#define NVGVOCONFIG_COMPOSITE_CB            0x00100000      // dwFields: compRange
#define NVGVOCONFIG_FULL_COLOR_RANGE        0x00200000      // dwFields: bEnableFullColorRange

//#define NVGVOCONFIG_COMPOSITE_RANGE_ALL     (NVGVOCONFIG_COMPOSITE_Y    | \
//                                             NVGVOCONFIG_COMPOSITE_CR   | \
//                                             NVGVOCONFIG_COMPOSITE_CB)

// Don't forget to update NVGVOCONFIG_VALIDFIELDS in NvGvoApiInternals.h when NVGVOCONFIG_ALLFIELDS changes.
#define NVGVOCONFIG_ALLFIELDS   ( NVGVOCONFIG_SIGNALFORMAT          | \
                                  NVGVOCONFIG_DATAFORMAT            | \
                                  NVGVOCONFIG_OUTPUTREGION          | \
                                  NVGVOCONFIG_OUTPUTAREA            | \
                                  NVGVOCONFIG_COLORCONVERSION       | \
                                  NVGVOCONFIG_GAMMACORRECTION       | \
                                  NVGVOCONFIG_SYNCSOURCEENABLE      | \
                                  NVGVOCONFIG_SYNCDELAY             | \
                                  NVGVOCONFIG_COMPOSITESYNCTYPE     | \
                                  NVGVOCONFIG_FRAMELOCKENABLE       | \
                                  NVGVOCONFIG_422FILTER             | \
                                  NVGVOCONFIG_COMPOSITETERMINATE    | \
                                  NVGVOCONFIG_DATAINTEGRITYCHECK    | \
                                  NVGVOCONFIG_CSCOVERRIDE           | \
                                  NVGVOCONFIG_FLIPQUEUELENGTH       | \
                                  NVGVOCONFIG_ANCTIMECODEGENERATION | \
                                  NVGVOCONFIG_COMPOSITE             | \
                                  NVGVOCONFIG_ALPHAKEYCOMPOSITE     | \
                                  NVGVOCONFIG_COMPOSITE_Y           | \
                                  NVGVOCONFIG_COMPOSITE_CR          | \
                                  NVGVOCONFIG_COMPOSITE_CB)

// Device configuration
// No members can be deleted from below structure. Only add new members at the 
// end of the structure
// Below structure is released in rel90. Any more field added after  rel95 should 
// put in version control before that version is releasing.

struct  NVGVOCOLORCONVERSION                                // Color conversion:
{                                                           //
    NVFLOAT       colorMatrix[3][3];                        //  Output[n] =
    NVFLOAT       colorOffset[3];                           //      Input[0] * colorMatrix[n][0] +
    NVFLOAT       colorScale[3];                            //      Input[1] * colorMatrix[n][1] +
                                                            //      Input[2] * colorMatrix[n][2] +
                                                            //      OutputRange * colorOffset[n]
                                                            //    where OutputRange is the standard magnitude of
                                                            //     Output[n][n] and colorMatrix and colorOffset 
                                                            //     values are within the range -1.0 to +1.0
    BOOL          bCompositeSafe;                           // bCompositeSafe constrains luminance range when using composite output
};

#define MAX_NUM_COMPOSITE_RANGE      2                      // maximum number of ranges per channel


typedef struct tagNVGVOCOMPOSITERANGE
{
    DWORD   dwRange;
    BOOL    bEnabled;
    DWORD   dwMin;
    DWORD   dwMax;
} NVGVOCOMPOSITERANGE;

typedef struct tagNVGVOCONFIG
{
    WORD              cbSize;                               // Caller sets to sizeof(NVGVOCONFIG)
    DWORD             dwFields;                             // Caller sets to NVGVOCONFIG_* mask for fields to use

    NVGVOSIGNALFORMAT signalFormat;                         // Signal format for video output
    NVGVODATAFORMAT   dataFormat;                           // Data format for video output
    NVGVOOUTPUTREGION outputRegion;                         // Region for video output (Desktop mode)
    NVGVOOUTPUTAREA   outputArea;                           // Usable resolution for video output (safe area)
    NVGVOCOLORCONVERSION  colorConversion;                  // Color conversion.

    union                                                   // Gamma correction:
    {                                                       //  cbSize field in gammaRamp describes type
        NVGAMMARAMP8  gammaRamp8;                           //  Gamma ramp (8-bit index, 16-bit values)
        NVGAMMARAMP10 gammaRamp10;                          //  Gamma ramp (10-bit index, 16-bit values)
    } gammaCorrection;                                      

    BOOL              syncEnable;                           // Sync enable (TRUE to use syncSource)
    NVGVOSYNCSOURCE   syncSource;                           // Sync source
    NVGVOSYNCDELAY    syncDelay;                            // Sync delay
    NVGVOCOMPSYNCTYPE compositeSyncType;                    // Composite sync type
    BOOL              frameLockEnable;                      // Flag indicating whether framelock was on/off
    double            fGammaValueR;                         // Red Gamma value within gamma ranges. 0.5 - 6.0
    double            fGammaValueG;                         // Green Gamma value within gamma ranges. 0.5 - 6.0
    double            fGammaValueB;                         // Blue Gamma value within gamma ranges. 0.5 - 6.0
    BOOL              bPSFSignalFormat;                     // Inidcates whether contained format is PSF Signal format
    BOOL              bEnable422Filter;                     // Enables/Disables 4:2:2 filter
    BOOL              bCompositeTerminate;                  // Composite termination
    BOOL              bEnableDataIntegrityCheck;            // Enable data integrity check: true - enable, false - disable
    BOOL              bCSCOverride;                         // Use provided CSC color matrix to overwrite 
    DWORD             dwFlipQueueLength;                    // Number of buffers used for the internal flipqueue used in pbuffer mode
    BOOL              bEnableANCTimeCodeGeneration;         // Enable SDI ANC time code generation
    BOOL              bEnableComposite;                     // Enable composite
    BOOL              bEnableAlphaKeyComposite;             // Enable Alpha key composite
    NVGVOCOMPOSITERANGE compRange;                          // Composite ranges
    BYTE              reservedData[256];                    // Inicates last stored SDI output state TRUE-ON / FALSE-OFF
    BOOL              bEnableFullColorRange;                // Flag indicating Full Color Range
} NVGVOCONFIG;

// Reserved Flags for NvGvoCaps
#define NVGVO_NO_MODESET            0x00000040L

//---------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Function:    NvGvoCaps
// Description: Determine graphics adapter Graphics to Video capabilities.
// Parameters:  nAdapterNumber  - Graphics adapter number
//              nReserved       - Reserved (must be set to zero)
//              pAdapterCaps    - Pointer to receive capabilities
// Returns:     NV_OK           - Success
//              NV_NOTSUPPORTED - Graphics to Video not supported
//---------------------------------------------------------------------
NVRESULT NVAPIENTRY  NvGvoCaps( UINT       nAdapterNumber IN, 
                                UINT       nReserved      IN,
                                NVGVOCAPS* pAdapterCaps   OUT );

//---------------------------------------------------------------------
// Function:    NvGvoOpen
// Description: Open graphics adapter for Graphics to Video operations
//              using the OpenGL application interface.  Read operations
//              are permitted in this mode by multiple clients, but Write 
//              operations are application exclusive.
// Parameters:  nAdapterNumber  - Graphics adapter number
//              nReserved       - Reserved (must be set to zero)
//              dwClass         - Class interface (NVGVOCLASS_* value)
//              dwAccessRights  - Access rights (NVGVO_O_* mask)
//              phGvoHandle     - Pointer to receive handle
// Returns:     NV_OK           - Success
//              NV_ACCESSDENIED - Access denied for requested access
//---------------------------------------------------------------------
NVRESULT NVAPIENTRY NvGvoOpen( UINT                  nAdapterNumber IN, 
                               UINT                  nReserved      IN,
                               DWORD                 dwClass        IN,
                               DWORD                 dwAccessRights IN,
                               NVGVOHANDLE*          phGvoHandle    OUT );
 
//---------------------------------------------------------------------
// Function:    NvGvoClose
// Description: Closes graphics adapter for Graphics to Video operations
//              using the OpenGL application interface.  Closing an 
//              OpenGL handle releases the device.
// Parameters:  hGvoHandle - Handle to graphics adapter
// Returns:     NV_OK      - Success
//---------------------------------------------------------------------
NVRESULT NVAPIENTRY NvGvoClose( NVGVOHANDLE hGvoHandle IN );

//---------------------------------------------------------------------
// Function:    NvGvoDesktopOpen
// Description: Open graphics adapter for Graphics to Video operations
//              using the Desktop transparent mode interface.  Read
//              operations are permitted in this mode by multiple clients,
//              but write operations are application exclusive.
// Parameters:  nAdapterNumber  - Graphics adapter number
//              nReserved       - Reserved (must be set to zero)
//              dwClass         - Class interface (NVGVOCLASS_* value)
//              dwAccessRights  - Access rights (NVGVO_O_* mask)
//              phGvoHandle     - Pointer to receive handle
// Returns:     NV_OK           - Success
//              NV_ACCESSDENIED - Access denied for requested access
//---------------------------------------------------------------------
NVRESULT NVAPIENTRY NvGvoDesktopOpen( UINT                  nAdapterNumber IN, 
                                      UINT                  nReserved      IN,
                                      DWORD                 dwClass        IN,
                                      DWORD                 dwAccessRights IN,
                                      NVGVOHANDLE*          phGvoHandle    OUT );
 
//---------------------------------------------------------------------
// Function:    NvGvoDesktopClose
// Description: Closes graphics adapter for Graphics to Video operations
//              using the Desktop transparent mode interface.
// Parameters:  hGvoHandle  - Handle to graphics adapter
//              bGvoRelease - TRUE to release device when handle closes
//                            FALSE to remain in desktop mode when handle
//                                  closes (other clients can open using
//                                  NvGvoDesktopOpen and release using 
//                                  NvGvoDesktopClose)
// Returns:     NV_OK       - Success
//---------------------------------------------------------------------
NVRESULT NVAPIENTRY NvGvoDesktopClose( NVGVOHANDLE hGvoHandle IN,
                                       BOOL        bRelease   IN );

//---------------------------------------------------------------------
// Function:    NvGvoStatus
// Description: Get Graphics to Video status.
// Parameters:  hGvoHandle - Handle to graphics adapter
// Returns:     NV_OK      - Success
//---------------------------------------------------------------------
NVRESULT NVAPIENTRY NvGvoStatus( NVGVOHANDLE  hGvoHandle IN, 
                                 NVGVOSTATUS* pStatus    OUT );

//---------------------------------------------------------------------
// Function:    NvGvoSyncFormatDetect
// Description: Detects Graphics to Video incoming sync video format.
// Parameters:  hGvoHandle - Handle to graphics adapter
//              pdwWait    - Pointer to receive milliseconds to wait 
//                           before NvGvoStatus will return detected 
//                           syncFormat.
// Returns:     NV_OK - Success
//---------------------------------------------------------------------
NVRESULT NVAPIENTRY NvGvoSyncFormatDetect( NVGVOHANDLE hGvoHandle IN,
                                           DWORD*      pdwWait    OUT );

//---------------------------------------------------------------------
// Function:    NvGvoConfigGet
// Description: Get Graphics to Video configuration.
// Parameters:  hGvoHandle - Handle to graphics adapter
//              pConfig    - Pointer to Graphics to Video configuration
// Returns:     NV_OK      - Success
//---------------------------------------------------------------------
NVRESULT NVAPIENTRY NvGvoConfigGet( NVGVOHANDLE  hGvoHandle IN, 
                                    NVGVOCONFIG* pConfig    OUT ); 

//---------------------------------------------------------------------
// Function:    NvGvoConfigSet
// Description: Set Graphics to Video configuration.
// Parameters:  hGvoHandle      - Handle to graphics adapter
//              pConfig         - Pointer to Graphics to Video config
// Returns:     NV_OK           - Success
//              NV_ACCESSDENIED - Access denied (no write access)
//              NV_RUNNING      - Requested settings require NvGvoStop
//---------------------------------------------------------------------
NVRESULT NVAPIENTRY NvGvoConfigSet( NVGVOHANDLE  hGvoHandle IN, 
                                    NVGVOCONFIG* pConfig    IN );

//---------------------------------------------------------------------
// Function:    NvGvoIsRunning
// Description: Determine if Graphics to Video output is running.
// Parameters:  hGvoHandle      - Handle to graphics adapter
// Returns:     NV_RUNNING      - Graphics to Video running
//              NV_NOTRUNNING   - Graphics to Video not running
//---------------------------------------------------------------------
NVRESULT NVAPIENTRY NvGvoIsRunning( NVGVOHANDLE hGvoHandle IN );

//---------------------------------------------------------------------
// Function:    NvGvoStart
// Description: Start Graphics to Video output.
// Parameters:  hGvoHandle      - Handle to graphics adapter
// Returns:     NV_OK           - Success
//              NV_ACCESSDENIED - Access denied (no write access)
//              NV_RUNNING      - Graphics to Video already running
//---------------------------------------------------------------------
NVRESULT NVAPIENTRY NvGvoStart( NVGVOHANDLE hGvoHandle IN );

//---------------------------------------------------------------------
// Function:    NvGvoStop
// Description: Stop Graphics to Video output.
// Parameters:  hGvoHandle      - Handle to graphics adapter
// Returns:     NV_OK           - Success
//              NV_ACCESSDENIED - Access denied (no write access)
//              NV_NOTRUNNING   - Graphics to Video not running
//---------------------------------------------------------------------
NVRESULT NVAPIENTRY NvGvoStop( NVGVOHANDLE hGvoHandle IN );

//---------------------------------------------------------------------
// Function:    NvGvoEnumSignalFormats
// Description: Enumerate signal formats supported by Graphics to Video.
// Parameters:  hGvoHandle          - Handle to graphics adapter
//              nEnumIndex          - Enumeration index
//              bByEnum             - TRUE nEnumIndex is NVSIGNALFORMAT_*
//                                    FALSE nEnumIndex is 0..n-1
//              pSignalFormatDetail - Pointer to receive detail or NULL
// Returns:     NV_OK               - Success
//              NV_NOMORE           - No more signal formats to enumerate
//              NV_NOTSUPPORTED     - Unsupported NVSIGNALFORMAT_ enumeration
//---------------------------------------------------------------------
NVRESULT NVAPIENTRY NvGvoEnumSignalFormats( NVGVOHANDLE              hGvoHandle          IN,
                                            int                      nEnumIndex          IN,
                                            BOOL                     bByEnum             IN,
                                            NVGVOSIGNALFORMATDETAIL* pSignalFormatDetail OUT );


//---------------------------------------------------------------------
// Function:    NvGvoIsFrameLockModeCompatible
// Description: Checkes whether modes are compatible in framelock mode
// Parameters:  hGvoHandle          - Handle to graphics adapter
//              nSrcEnumIndex       - Source Enumeration index
//              nDestEnumIndex      - Destination Enumeration index
//              
//              pbCompatible        - Pointer to receive compatability
// Returns:     NV_OK               - Success
//              NV_NOTSUPPORTED     - Unsupported NVSIGNALFORMAT_ enumeration
//---------------------------------------------------------------------
NVRESULT NVAPIENTRY NvGvoIsFrameLockModeCompatible( NVGVOHANDLE  hGvoHandle          IN,
                                                    int          nSrcEnumIndex       IN,
                                                    int          nDestEnumIndex      IN,
                                                    BOOL*        pbCompatible        OUT );


//---------------------------------------------------------------------
// Function:    NvGvoEnumDataFormats
// Description: Enumerate data formats supported by Graphics to Video.
// Parameters:  hGvoHandle        - Handle to graphics adapter
//              nEnumIndex        - Enumeration index
//              bByEnum           - TRUE nEnumIndex is NVDATAFORMAT_*
//                                  FALSE nEnumIndex is 0..n-1
//              pDataFormatDetail - Pointer to receive detail or NULL
// Returns:     NV_OK             - Success
//              NV_NOMORE         - No more data formats to enumerate
//              NV_NOTSUPPORTED   - Unsupported NVDATAFORMAT_ enumeration
//---------------------------------------------------------------------
NVRESULT NVAPIENTRY NvGvoEnumDataFormats( NVGVOHANDLE            hGvoHandle        IN,
                                          int                    nEnumIndex        IN,
                                          BOOL                   bByEnum           IN,
                                          NVGVODATAFORMATDETAIL* pDataFormatDetail OUT );


#ifdef  __cplusplus
    }
#endif//__cplusplus

#endif//__NVGVOAPI_H__
