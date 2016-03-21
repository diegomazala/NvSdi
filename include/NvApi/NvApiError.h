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

#ifndef __NVAPIERROR_H__
#define __NVAPIERROR_H__

#ifdef __cplusplus
    extern "C" {
#endif

#ifndef NVAPIENTRY
#   define NVAPIENTRY APIENTRY
#endif//NVAPIENTRY

#define NVRESULT_SUCCESS(nr)    ((int)(nr) <= 0) // Success (NV_OK or NV_WARN_xxx)
#define NVRESULT_FAILURE(nr)    ((int)(nr) >  0) // Failure
#define NVRESULT_WARN(nr)       ((int)(nr) <  0) // Warning

#define NVRESULT_ERRORCODE(nr)  ((NVRESULT)(abs((int)nr))) // Extract error code from NVRESULT value (warning --> error)

typedef enum NVRESULT
{
    NV_OK                   =   0,  // Success.
//----------------------------------------------------------------------------
    NV_INTERNALERROR        =   1,  // Internal error.
    NV_ALREADYINITIALIZED   =   2,  // Already initialized.
    NV_NOTINITIALIZED       =   3,  // Not initialized.
    NV_OUTOFMEMORY          =   4,  // Not enough memory for operation.
    NV_NOTSUPPORTED         =   5,  // Feature not supported.
    NV_NOTAVAILABLE         =   6,  // Feature not presently available.
    NV_NOTIMPLEMENTED       =   7,  // Feature not implemented.
    NV_BADPARAMETER         =   8,  // Invalid parameter.
    NV_ACCESSDENIED         =   9,  // Access denied.
    NV_RUNNING              =  10,  // Operation requires inactive environment.
    NV_NOTRUNNING           =  11,  // Operation requires active environment.
    NV_FILENOTFOUND         =  12,  // Unable to locate file.
    NV_NOMORE               =  13,  // No more items.
    NV_ILLEGALSTATE         =  14,  // Illegal state could not be resolved.
    NV_NOTFOUND             =  15,  // Not found
    NV_ALLOCATED            =  16,  // Indicates that the request cannot be completed as the resource is already allocated/deallocated
    NV_BADPARAMETERSIZE     =  17,  // As mentioned by Satish, we require to give details of what bad parameter is
    NV_SET_ROTATE_3DAPP_ERR =  18,  // Unable to rotate as a 3D app is running
    NV_SET_ROTATE_ERR       =  19,  // Unable to rotate because of invalid refreshrate or resolution
//----------------------------------------------------------------------------
    NV_WARN_INTERNALERROR   =  -1,  // Internal warning.
    NV_WARN_ILLEGALSTATE    = -14,  // Illegal state was automatically resolved.
    NV_WARN_NOTEQUAL        = -15,  // State compare failed
    NV_WARN_NOMORE          = -16,  // Warning that state compare failed and there are no more to enum
    NV_WARN_ALREADYINITIALIZED = -17,//Warning that we are already initialized
    NV_WARN_ALLOCATED       = -18,  // Warning that this state is already allocation/deallocated
    NV_WARN_NOTINITIALIZED  = -19,  // Warning that the api is not initialized
} NVRESULT;

NVRESULT NVAPIENTRY NvGetLastError();
typedef NVRESULT (NVAPIENTRY* fNvGetLastError)();

LPCWSTR NVAPIENTRY NvGetLastErrorMessageW();
LPCSTR NVAPIENTRY NvGetLastErrorMessageA();
typedef LPCWSTR (NVAPIENTRY* fNvGetLastErrorMessageW)();
typedef LPCSTR (NVAPIENTRY* fNvGetLastErrorMessageA)();

LPCWSTR NVAPIENTRY NvGetErrorMessageW(NVRESULT nr);
LPCSTR NVAPIENTRY NvGetErrorMessageA(NVRESULT nr);
typedef LPCWSTR (NVAPIENTRY* fNvGetErrorMessageW)(NVRESULT nr);
typedef LPCSTR (NVAPIENTRY* fNvGetErrorMessageA)(NVRESULT nr);

#ifdef  UNICODE
#   define NvGetLastErrorMessage NvGetLastErrorMessageW
#   define NvGetErrorMessage NvGetLastMessageW
#else
#   define NvGetLastErrorMessage NvGetLastErrorMessageA
#   define NvGetErrorMessage NvGetLastMessageA
#endif//UNICODE

#ifdef __cplusplus
} //extern "C" {
#endif

#endif//__NVAPIERROR_H__