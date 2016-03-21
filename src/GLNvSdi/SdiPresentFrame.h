#ifndef __SDI_PRESENT_FRAME_H__
#define __SDI_PRESENT_FRAME_H__

#include "GLNvSdiCore.h"
#include "GLFbo.h"
//#include "SdiOptions.h"
#include "ClockTime.h"

struct SdiStats
{
	const int NumQueries;
	bool queryTime;		
	GLuint durationTime;	
	int frame_count;
	float presentationInterval;
	float sendInterval;
	float latency;
	float bufsQueued;
	int	cur_query;
	SdiStats():NumQueries(5), durationTime(1)
	{}
};

class GLNVSDI_API SdiPresentFrame
{
public:

	SdiPresentFrame();
	virtual ~SdiPresentFrame();

	void Initialize();
	void Uninitialize();

	virtual void PresentFrame(){};// = 0;

	virtual void PresentFrame(GLenum texType, GLuint texId);
	virtual void PresentFrame(GLenum texType, GLuint texId_0, GLuint texId_1);

	virtual void PresentFrame(gl::Texture2D& tex);
	virtual void PresentFrame(gl::Texture2D& tex_0, gl::Texture2D& tex_1);

	virtual void PresentFrameDual(gl::Texture2D& tex_0, gl::Texture2D& tex_1);
	virtual void PresentFrameDual(gl::Texture2D& tex_0, gl::Texture2D& tex_1, gl::Texture2D& tex_2, gl::Texture2D& tex_3);

	virtual void PresentFrameDual(GLenum texType, GLuint texId_0, GLuint texId_1);
	virtual void PresentFrameDual(GLenum texType, GLuint texId_0, GLuint texId_1, GLuint texId_2, GLuint texId_3);
 

	void PrintStats(bool print=true);

	const SdiStats& GetStats() const { return mStats; }

protected:
	void PrePresentFrame();
	void PostPresentFrame();

	bool		mPrintStats;
	SdiStats	mStats;
	GLuint		m_PresentID[NUM_QUERIES];	// Present time query IDs
	GLuint		m_DurationID[NUM_QUERIES];	// Duration time query IDs

	clk::time mStartSend;
	clk::time mStopSend;

	PFNGLPRESENTFRAMEKEYEDNVPROC	glPresentFrameKeyedNV;
	PFNGLPRESENTFRAMEDUALFILLNVPROC glPresentFrameDualFillNV;
	PFNGLGENQUERIESARBPROC			glGenQueriesARB;
	PFNGLDELETEQUERIESARBPROC		glDeleteQueriesARB;
	PFNGLGETQUERYOBJECTUIVARBPROC	glGetQueryObjectuivARB;
	PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64vEXT;
	PFNGLGETVIDEOUI64VNVPROC		glGetVideoui64vNV;
};



#endif // __SDI_PRESENT_FRAME_H__