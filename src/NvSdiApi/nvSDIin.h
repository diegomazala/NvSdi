#ifndef NVSDIIN_H
#define NVSDIIN_H

#include <windows.h>
#include <assert.h>
#include <stdio.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/wglext.h>

//DECLARE_HANDLE(HVIDEOINPUTDEVICENV);



#include "nvapi.h"
#include "CommandLine.h"
// Definitions
#define MAX_VIDEO_STREAMS 4
#define MEASURE_PERFORMANCE 



typedef class CNvSDIinTopology 
{
	protected:
		NVVIOTOPOLOGYTARGET *m_lDevice[NVAPI_MAX_VIO_DEVICES];
		bool m_bInitialized;
		int m_nDevice;
		CNvSDIinTopology();
		virtual ~CNvSDIinTopology();
		bool init();
	public:
		static CNvSDIinTopology& instance();		
		NVVIOTOPOLOGYTARGET *getDevice(int index);		
		int getNumDevice();
}CNvSDIinTopology;


 /*
 **	CNvSDIin - class that allows to capture one or multiple streams into
 ** a texture/video buffer object
 ** 
 */

typedef class CNvSDIin
{
protected:	
	
	//Capture Interpretation settings
	bool m_bDualLink;
	NVVIOCOMPONENTSAMPLING m_Sampling;
	int m_BitsPerComponent;
	int m_ExpansionEnable;

	//Capture detected settings
	unsigned int m_videoWidth;				// Video format resolution in pixels
	unsigned int m_videoHeight;				// Video format resolution in lines	
	float		 m_fFrameRate;                     // Video frame rate
    NVVIOSIGNALFORMAT m_videoFormat;  
    unsigned int m_numStreams;
    unsigned int m_activeJacks[NVAPI_MAX_VIO_JACKS];
	NVVIOSIGNALFORMATDETAIL m_signalFormatDetail;//Video Signal Details


	HDC                 m_hDC;			    // GPU device context
	GLuint				m_videoSlot;				// GPU Video slot	

	HVIDEOINPUTDEVICENV m_device;			// Video input device
	NvU32       m_vioID;                    //unique video device ID
	NvVioHandle m_vioHandle;				// Video device handle
	int m_deviceNumber;

	//GPU object settings
	GLfloat m_cscMat[4][4];
	GLfloat m_cscMax[4];
	GLfloat m_cscMin[4];
	GLfloat m_cscOffset[4];

	bool m_bCaptureStarted;					// Set to true when glBeginVideoCaptureNV had successfuly completed

	// Get video input state
	HRESULT getVideoInState(NVVIOCONFIG_V1 *vioConfig, NVVIOSTATUS *vioStatus);
	HRESULT setVideoConfig(bool bShowMessageBox);
	void DumpChannelStatus(NVVIOCHANNELSTATUS jack);
	void DumpStreamStatus(NVVIOSTREAM stream);	
	
	GLuint        m_captureTimeQuery;

	int mRingBufferSizeInFrames;

public:

	void SetRingBufferSize(int frameCount){mRingBufferSizeInFrames = frameCount;}
	int  GetRingBufferSize()const {return mRingBufferSizeInFrames;}

	
	float m_gviTime;	
	float m_gpuTime;
	CNvSDIin();
	~CNvSDIin();
    HRESULT Init(Options *options = NULL);
	
	HRESULT SetupDevice(bool bShowMessageBox = true, int deviceNumber = 0); //sets up a particular capture device	
	//it is up to the developer to make sure that the same video slot is not used twice for two different capture devices 
	//on one context. Later this accounting can be made transparent to the user and done in the class
	HRESULT BindDevice(GLuint videoSlot, HDC hDC);	
		

	// target = GL_FIELD_UPPER_NV, GL_FIELD_LOWER_NV, GL_FRAME_NV
	HRESULT BindVideoTexture(GLuint videoTexture, int stream, GLenum textureType = GL_TEXTURE_RECTANGLE_NV, GLenum target = GL_FRAME_NV);	
	HRESULT UnbindVideoTexture(int stream, GLenum textureType = GL_TEXTURE_RECTANGLE_NV, GLenum target = GL_FRAME_NV);	

	HRESULT BindVideoFrameBuffer(GLuint videoBuffer, GLint videoBufferFormat, int stream);		
	HRESULT UnbindVideoFrameBuffer(int stream);	
	
	HRESULT StartCapture();
	GLenum Capture(GLuint *sequenceNum, GLint64EXT *captureTime);
	HRESULT EndCapture();

	HRESULT Cleanup();
	HRESULT UnbindDevice();

	void SetCSCParams(GLfloat *cscMat,GLfloat *cscOffset, GLfloat *cscMin, GLfloat *cscMax);

	inline NvVioHandle GetVioHandle() {return m_vioHandle; }
	inline unsigned int GetWidth() {return m_videoWidth; }
	inline unsigned int GetHeight() {return m_videoHeight; }	
	int    GetBufferObjectPitch(int streamIndex);
	//inline unsigned int GetNumStreams(){return MAX_VIDEO_STREAMS;}
	inline unsigned int GetNumStreams(){return m_numStreams;}
	inline NVVIOSIGNALFORMAT GetSignalFormat(){return m_videoFormat;}
	HRESULT GetFrameRate(float *rate);
	inline int GetDeviceNumber(){return m_deviceNumber;}
}CNvSDIin;



#endif NVSDIIN_H
