#ifndef COMMANDLINE_H
#define COMMANDLINE_H


#ifdef USE_NVAPI
#include "nvapi.h"
#else
#include "NvGvoApi.h"
#include "NvApiError.h"
#endif

typedef enum eTestPattern{
	TEST_PATTERN_RGB_COLORBARS_100,
	TEST_PATTERN_RGB_COLORBARS_75,
	TEST_PATTERN_YCRCB_COLORBARS,
	TEST_PATTERN_COLORBARS8_75,
	TEST_PATTERN_COLORBARS8_100,
	TEST_PATTERN_COLORBARS10_75,
	TEST_PATTERN_COLORBARS10_100,
	TEST_PATTERN_FRAME,
	TEST_PATTERN_LOGO,
	TEST_PATTERN_RAMP8,
	TEST_PATTERN_RAMP16,
	TEST_PATTERN_BAR,
	TEST_PATTERN_LINES,
	TEST_PATTERN_SPHERE
} eTestPattern;


//
// Definition of command line option structure.
//
typedef struct {
	NVVIOSIGNALFORMAT videoFormat;
	NVVIODATAFORMAT dataFormat;
	NVVIOCOMPSYNCTYPE syncType;
	NVVIOSYNCSOURCE syncSource;
	eTestPattern testPattern;
	BOOL syncEnable;
	BOOL frameLock;
	int numFrames;
	int repeat;
	int gpu; //output GPU
	BOOL block;
	BOOL videoInfo;
	BOOL fps;
	int fsaa;
	int hDelay;
	int vDelay;
	int flipQueueLength;
	BOOL field;
	BOOL console;
	BOOL log;
	BOOL cscEnable;
	double cscOffset[3];
	double cscScale[3];
	float cscMatrix[3][3];
	float gamma[3];
	BOOL alphaComp;
	BOOL yComp;
	BOOL crComp;
	BOOL cbComp;
	int yCompRange[4];
	int crCompRange[4];
	int cbCompRange[4];
	int x;
	int y;
	int width;
	int height;
	char filename[100];
	char audioFile[100];
	int audioChannels;
	int audioBits;	
	//Capture settings//
	int captureGPU; //capture GPU
	int captureDevice; //capture card number
	bool dualLink;
	NVVIOCOMPONENTSAMPLING sampling;
	int bitsPerComponent;
	bool expansionEnable;
	bool fullScreen;
} Options;

//
// Command line processing functions.
//
BOOL ParseCommandLine(char *szCmdLine[], Options *options);

#endif