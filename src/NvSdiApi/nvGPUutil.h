
#ifndef NVGPUUTIL_H
#define NVGPUUTIL_H

#include <windows.h>
#include <assert.h>
#include <stdio.h>

#include <string>
#include <iostream>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/wglext.h>


using namespace std;

#define MAX_GPUS 4
//
//CNvGpu
//
typedef class CNvGpu
{
	protected:
		HGPUNV m_hGpu; //GPU affinity handle
		bool m_bPrimary;//contains the desktop		
		bool m_bDisplay;	
	public:		
		CNvGpu(){}
		virtual ~CNvGpu(){}
		bool init(HGPUNV gpuAffinityHandle,bool bPrimary, bool bDisplay);
		bool isPrimary(){return m_bPrimary;}		
		bool isDisplay(){return m_bDisplay;}
		HGPUNV getAffinityHandle(){return m_hGpu;}
}CNvGPU;

//
//CNvGpuTopology
//
typedef class CNvGpuTopology
{
	protected:
		CNvGpu *m_lGpu[MAX_GPUS];
		int m_nGpu;
		bool m_bInitialized;
		CNvGpuTopology();
		virtual ~CNvGpuTopology();
		bool init();
	public:
		static CNvGpuTopology& instance();
		static void destroy();
		virtual CNvGpu *getGpu(int index);	
		virtual CNvGpu *getPrimaryGpu();	
		int				getNumGpu();	
		int				getPrimaryGpuIndex();
}CNvGpuTopology;

bool CreateDummyGLWindow(HWND * hWnd, HGLRC * hGLRC);
bool DestroyGLWindow(HWND * hWnd, HGLRC * hGLRC);

#endif
