#include "common.h"

static FILE *hf;
static int hFd;

//
// Open log file
//
void SetupLog()
{
	static int initialized = 0;

	if (initialized == 1) {
		return;
	}

	// Open log file
	if ((_sopen_s(&hFd, "log.txt", _O_CREAT | _O_WRONLY, _SH_DENYNO, _S_IWRITE )))
		exit( 1 );

	// Get stream from file descriptor
	if ((hf = _fdopen( hFd, "w" )) == NULL )
		exit( 1 );

	// Set standard output to write to log file.
	*stdout = *hf;
	setvbuf(stdout, NULL, _IONBF, 0);

	// Set standard error to write to log file.
	if ((hf = _fdopen( hFd, "w" )) == NULL )
		exit( 1 );

	*stderr = *hf;
	setvbuf(stderr, NULL, _IONBF, 0);

	initialized = 1;
}


//
// Close log file
//
void CloseLog()
{	
//	fclose(hf);

	_close(hFd);
}



//
// Setup console
//
void SetupConsole()
{
	int hCrt;
	FILE *hf;
	static int initialized = 0;

	if(initialized == 1) {
		return;
	}

	AllocConsole();

	// Setup stdout
	hCrt = _open_osfhandle( (long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT );
	hf = _fdopen(hCrt, "w" );
	*stdout = *hf;
	setvbuf(stdout, NULL, _IONBF, 0);

	// Setup stderr
	hCrt = _open_osfhandle( (long)GetStdHandle(STD_ERROR_HANDLE), _O_TEXT );
	hf = _fdopen(hCrt, "w" );
	*stderr = *hf;
	setvbuf(stderr, NULL, _IONBF, 0);

	initialized = 1;
}
