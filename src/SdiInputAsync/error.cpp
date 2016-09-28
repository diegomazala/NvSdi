#include "error.h"
#include <stdio.h>

void _CheckError(CUresult err, const char * filename, const int linenum)
{
	if(err != CUDA_SUCCESS)
	{
		char * error_str;
		switch(err)
		{		
			case CUDA_ERROR_INVALID_VALUE:
				error_str = "CUDA_ERROR_INVALID_VALUE";
				break;				
			case CUDA_ERROR_OUT_OF_MEMORY:
				error_str = "CUDA_ERROR_OUT_OF_MEMORY";
				break;				
			case CUDA_ERROR_NOT_INITIALIZED:
				error_str = "CUDA_ERROR_NOT_INITIALIZED";
				break;				
			case CUDA_ERROR_DEINITIALIZED:
				error_str = "CUDA_ERROR_DEINITIALIZED";
				break;	
			case CUDA_ERROR_NO_DEVICE:
				error_str = "CUDA_ERROR_NO_DEVICE";
				break;				
			case CUDA_ERROR_INVALID_DEVICE:
				error_str = "CUDA_ERROR_INVALID_DEVICE";
				break;				
			case CUDA_ERROR_INVALID_IMAGE:
				error_str = "CUDA_ERROR_INVALID_IMAGE";
				break;				
			case CUDA_ERROR_INVALID_CONTEXT:
				error_str = "CUDA_ERROR_INVALID_CONTEXT";
				break;				
			case CUDA_ERROR_CONTEXT_ALREADY_CURRENT:
				error_str = "CUDA_ERROR_CONTEXT_ALREADY_CURRENT";
				break;				
			case CUDA_ERROR_MAP_FAILED:
				error_str = "CUDA_ERROR_MAP_FAILED";
				break;				
			case CUDA_ERROR_UNMAP_FAILED:
				error_str = "CUDA_ERROR_UNMAP_FAILED";
				break;				
			case CUDA_ERROR_ARRAY_IS_MAPPED:
				error_str = "CUDA_ERROR_ARRAY_IS_MAPPED";
				break;				
			case CUDA_ERROR_ALREADY_MAPPED:
				error_str = "CUDA_ERROR_ALREADY_MAPPED";
				break;				
			case CUDA_ERROR_NO_BINARY_FOR_GPU:
				error_str = "CUDA_ERROR_NO_BINARY_FOR_GPU";
				break;				
			case CUDA_ERROR_ALREADY_ACQUIRED:
				error_str = "CUDA_ERROR_ALREADY_ACQUIRED";
				break;				
			case CUDA_ERROR_NOT_MAPPED:
				error_str = "CUDA_ERROR_NOT_MAPPED";
				break;	
			case CUDA_ERROR_INVALID_SOURCE:
				error_str = "CUDA_ERROR_INVALID_SOURCE";
				break;				
			case CUDA_ERROR_FILE_NOT_FOUND:
				error_str = "CUDA_ERROR_FILE_NOT_FOUND";
				break;
			case CUDA_ERROR_INVALID_HANDLE:
				error_str = "CUDA_ERROR_INVALID_HANDLE";
				break;	
			case CUDA_ERROR_NOT_FOUND:
				error_str = "CUDA_ERROR_NOT_FOUND";
				break;	
			case CUDA_ERROR_NOT_READY:
				error_str = "CUDA_ERROR_NOT_READY";
				break;	
			case CUDA_ERROR_LAUNCH_FAILED:
				error_str = "CUDA_ERROR_LAUNCH_FAILED";
				break;				
			case CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES:
				error_str = "CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES";
				break;				
			case CUDA_ERROR_LAUNCH_TIMEOUT:
				error_str = "CUDA_ERROR_LAUNCH_TIMEOUT";
				break;				
			case CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING:
				error_str = "CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING";
				break;	
			case CUDA_ERROR_UNKNOWN:
				error_str = "CUDA_ERROR_UNKNOWN";
				break;
			default:
				error_str = "Unknown Error Code";
				break;
		}
		printf("CUDA Error #%d, %s in file: %s, line: %d\n",err,error_str,filename,linenum);
	}
}