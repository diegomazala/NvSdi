#ifndef NVSDIOUT_H
#define NVSDIOUT_H

#include <windows.h>
#include <assert.h>
#include <stdio.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/wglext.h>

#include "nvapi.h"
#include "CommandLine.h"

#include "nvGPUutil.h"

typedef class CNvSDIoutGpu : public virtual CNvGpu
{
	protected:
		bool m_bSDIoutput;
		NvVioHandle m_hVioHandle;

	public:			
		CNvSDIoutGpu(){}
		virtual ~CNvSDIoutGpu(){}

		bool init(HGPUNV gpuAffinityHandle,bool bPrimary, bool bDisplay, bool bSDIOutput, NvVioHandle hVioHandle);
		bool isSDIoutput(){return m_bSDIoutput;}
		NvVioHandle getVioHandle(){return m_hVioHandle;}
}CNvSDIoutGpu;


#if 1
typedef class CNvSDIoutGpuTopology : public virtual CNvGpuTopology
{
protected:
	CNvSDIoutGpuTopology();
	virtual ~CNvSDIoutGpuTopology();
	bool init();

public:
	static CNvSDIoutGpuTopology& instance();
	//{
	//	if (!s_instance)
	//		s_instance = new CNvSDIoutGpuTopology;
	//	return *s_instance;
	//}
	static void destroy();
	//{
	//	if (s_instance)
	//	{
	//		delete s_instance;
	//		s_instance = nullptr;
	//	}
	//}
	virtual CNvSDIoutGpu *getGpu(int index);
	virtual CNvSDIoutGpu *getPrimaryGpu();
}CNvSDIoutGpuTopology;

#else

typedef class CNvSDIoutGpuTopology : public virtual CNvGpuTopology
{
	//protected:
	public:
		CNvSDIoutGpuTopology();
		virtual ~CNvSDIoutGpuTopology();
		bool init();
		void destroy();
	public:
		static CNvSDIoutGpuTopology& instance();
		virtual CNvSDIoutGpu *getGpu(int index);
		virtual CNvSDIoutGpu *getPrimaryGpu();
}CNvSDIoutGpuTopology;
#endif

typedef class CNvSDIout
{
private:

#ifdef USE_NVAPI
	NvVioHandle m_vioHandle;				// Video device handle
#else
	NVGVOHANDLE m_hGVO;						// Video device handle.
#endif

	float m_frameRate;                      // Video frame rate

	unsigned int m_videoWidth;				// Video format resolution in pixels
	unsigned int m_videoHeight;				// Video format resolution in lines

	BOOL m_bInterlaced;						// Interlaced flag.

public:

	CNvSDIout();
	~CNvSDIout();

	HRESULT Init(Options *options, CNvSDIoutGpu *SdiOutGpu = NULL);
	HRESULT Start();
	HRESULT Stop();
	HRESULT Cleanup();

	inline NvVioHandle getHandle() {return m_vioHandle; };

	inline unsigned int GetWidth() {return m_videoWidth; };
	inline unsigned int GetHeight() {return m_videoHeight; };

	inline float GetFrameRate(){return m_frameRate; };

	inline BOOL IsInterlaced() {return m_bInterlaced; };

	HRESULT DisplayVideoStatus();
	HRESULT DisplaySignalFormatInfo();
	HRESULT DisplayDataFormatInfo();
	HRESULT DisplayColorConversionInfo();
	HRESULT DisplayGammaCorrectionInfo();
	HRESULT GetFrameRate(float *rate);

	HRESULT SetCSC(NVVIOCOLORCONVERSION *csc, bool enable);
	HRESULT GetCSC(NVVIOCOLORCONVERSION *csc, bool *enable);

	HRESULT GetSyncDelay(NVVIOSYNCDELAY *delay);
	HRESULT SetSyncDelay(NVVIOSYNCDELAY *delay);

}CNvSDIout;

#endif NVSDIOUT_H
