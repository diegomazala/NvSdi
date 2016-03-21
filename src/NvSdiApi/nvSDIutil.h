#ifndef NVSDIUTIL_H
#define NVSDIUTIL_H

#include <windows.h>
#include <assert.h>
#include <stdio.h>

#include <string>
#include <iostream>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/wglext.h>

#include "nvapi.h"


using namespace std;

string SignalFormatToString(NVVIOSIGNALFORMAT format);
string DataFormatToString(NVVIODATAFORMAT format);
string SyncStatusToString(NVVIOSYNCSTATUS status);
string SyncSourceToString(NVVIOSYNCSOURCE source);
string ComponentSamplingFormatToString(NVVIOCOMPONENTSAMPLING sampling);
string ColorSpaceToString(NVVIOCOLORSPACE space);
string LinkIDToString(NVVIOLINKID id);




#endif