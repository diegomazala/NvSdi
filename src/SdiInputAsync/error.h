#pragma once

#include <cuda.h>

// Error handler, just prints the error test to stdio
void _CheckError(CUresult error, const char * filename, const int linenum );  // perform whatever error checking we want
	

//define to add the line & filename to the error output
#define CheckError(err) _CheckError(err, __FILE__, __LINE__)

