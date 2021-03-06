#include "SdiPresentFrame.h"


SdiPresentFrame::SdiPresentFrame() :mPrintStats(false)
{
}


SdiPresentFrame::~SdiPresentFrame()
{
}


void SdiPresentFrame::Initialize()
{
	mStats.cur_query = 0;
	mStats.frame_count = 0;
	mStats.queryTime = GL_FALSE;

	glPresentFrameKeyedNV = (PFNGLPRESENTFRAMEKEYEDNVPROC)wglGetProcAddress("glPresentFrameKeyedNV");
	glPresentFrameDualFillNV = (PFNGLPRESENTFRAMEDUALFILLNVPROC)wglGetProcAddress("glPresentFrameDualFillNV");

	glGetVideoui64vNV = (PFNGLGETVIDEOUI64VNVPROC)wglGetProcAddress("glGetVideoui64vNV");

	glGenQueriesARB = (PFNGLGENQUERIESARBPROC)wglGetProcAddress("glGenQueriesARB");
	glDeleteQueriesARB = (PFNGLDELETEQUERIESARBPROC)wglGetProcAddress("glDeleteQueriesARB");
	glGetQueryObjectui64vEXT = (PFNGLGETQUERYOBJECTUI64VEXTPROC)wglGetProcAddress("glGetQueryObjectui64vEXT");
	glGetQueryObjectuivARB = (PFNGLGETQUERYOBJECTUIVARBPROC)wglGetProcAddress("glGetQueryObjectuivARB");

	// Initialize timing query objects
	glGenQueriesARB(NUM_QUERIES, m_PresentID);
	glGenQueriesARB(NUM_QUERIES, m_DurationID);

	if (!glPresentFrameKeyedNV && !glPresentFrameDualFillNV && !glGetVideoui64vNV)
	{
		throw(std::exception("GL_NV_Error: NV_present_video extension not available"));
	}

	if (!glGenQueriesARB && !glGetQueryObjectui64vEXT && !glGetQueryObjectuivARB && !glDeleteQueriesARB)
	{
		throw(std::exception("GL_EXT_ARB_Error: Time extension not available"));
	}

}


void SdiPresentFrame::Uninitialize()
{
	glDeleteQueriesARB(NUM_QUERIES, m_PresentID);
	glDeleteQueriesARB(NUM_QUERIES, m_DurationID);
}



void SdiPresentFrame::PrintStats(bool print)
{
	mPrintStats = print;
}

void SdiPresentFrame::PrePresentFrame()
{
	clk::time start, end;


	GLenum val = GL_NO_ERROR;
	GLuint64EXT presentTime = 0;
	static GLuint64EXT lastPresentTime = 0, lastSendTime = 0;
	static GLuint64EXT sendTime[NUM_QUERIES];   // Send time queries
	GLenum eVal = GL_NO_ERROR;

	mStats.frame_count += 1;

	GLuint presentTimeID = m_PresentID[mStats.cur_query];
	GLuint presentDurationID = m_DurationID[mStats.cur_query];

	mStats.presentationInterval = 0;
	mStats.sendInterval = 0;
	mStats.latency = 0;
	mStats.bufsQueued = 0;


	// Query video present time and duration.  Only do this once
	// we have been through the query loop once to ensure that 
	// results are available.
	if (mStats.queryTime)
	{
		start = clk::GetTime();

		glGetQueryObjectui64vEXT(presentTimeID, GL_QUERY_RESULT_ARB, &presentTime);
		glGetQueryObjectuivARB(presentDurationID, GL_QUERY_RESULT_ARB, &mStats.durationTime);
		end = clk::GetTime();

		float duration_query = end - start;
		float d_q = (float)duration_query / 1000000.0f;
		mStats.latency = ((float)presentTime - sendTime[mStats.cur_query]) * .000001f; 
		mStats.presentationInterval = (presentTime - lastPresentTime) * .000001;
		mStats.sendInterval = (sendTime[mStats.cur_query] - lastSendTime)* .000001;
		mStats.bufsQueued = (((float)mStats.latency / mStats.presentationInterval) + 0.5f);

		if (mPrintStats)
		{

			//fprintf(stderr, "\n send time: %I64d  present time: %I64d  latency: %f msec  \n present interval: %f msec send interval %f \n buffs queued: %.1f  duration: %d frame Query Interval: %.1f \n",
			//	sendTime[mStats.cur_query], presentTime, mStats.latency, mStats.presentationInterval, mStats.sendInterval, mStats.bufsQueued, mStats.durationTime, d_q);
			std::cout << std::fixed
				<< "send time: " << sendTime[mStats.cur_query] << " present time: " << presentTime
				<< " latency: " << mStats.latency << " msec present interval: " << mStats.presentationInterval
				<< " msec send interval " << mStats.sendInterval << " buffs queued: " << mStats.bufsQueued
				<< " duration: " << mStats.durationTime << " frame Query Interval: " << d_q << std::endl;

			if (mStats.durationTime > 1)
			{
				//fprintf(stderr, "\n Duplicate Frame, Frame Presented %d times.\n", mStats.durationTime);
				std::cout << "Duplicate Frame, Frame Presented " << mStats.durationTime << " times." << std::endl;
			}
		}

		if (mStats.durationTime > 1)
		{
			std::cout << std::fixed 
				<< "*** send time: " << sendTime[mStats.cur_query] << " present time: " << presentTime
				<< " latency: " << mStats.latency << " msec present interval: " << mStats.presentationInterval
				<< " msec send interval " << mStats.sendInterval << " buffs queued: " << mStats.bufsQueued
				<< " duration: " << mStats.durationTime << " frame Query Interval: " << d_q << std::endl;
		}

		

		lastPresentTime = presentTime;
		lastSendTime = sendTime[mStats.cur_query];

		//CheckGLError("<QueryTime>");
	}

	// Draw to video
	glGetVideoui64vNV(1, GL_CURRENT_TIME_NV, &sendTime[mStats.cur_query]);
	start = clk::GetTime();
	eVal = glGetError();

	mStartSend = clk::GetTime();

}


void SdiPresentFrame::PostPresentFrame()
{
	mStopSend = clk::GetTime();

	//clk::GetIntervalMsec(mStartSend, mStopSend);

	mStats.cur_query++;
	if (mStats.cur_query == NUM_QUERIES)
	{
		mStats.cur_query = 0;
		mStats.queryTime = GL_TRUE;	// this line cause an debug assertion when input video is active
	}

	GLuint64 t;
	glGetVideoui64vNV(1, GL_CURRENT_TIME_NV, &t);
}



void SdiPresentFrame::PresentFrame(GLenum texType, GLuint texId, GLuint64EXT minPresentTime)
{
	this->PrePresentFrame();

	GLuint presentTimeID = m_PresentID[mStats.cur_query];
	GLuint presentDurationID = m_DurationID[mStats.cur_query];
	

	glPresentFrameKeyedNV(1, minPresentTime,
		presentTimeID, presentDurationID,
		GL_FRAME_NV,
		texType, texId, 0,
		GL_NONE, 0, 0);

	this->PostPresentFrame();
}




void SdiPresentFrame::PresentFrame(GLenum texType, GLuint texId_0, GLuint texId_1, GLuint64EXT minPresentTime)
{
	this->PrePresentFrame();

	GLuint presentTimeID = m_PresentID[mStats.cur_query];
	GLuint presentDurationID = m_DurationID[mStats.cur_query];

	glPresentFrameKeyedNV(1, minPresentTime,
		presentTimeID, presentDurationID,
		GL_FIELDS_NV,
		texType, texId_0, 0,
		texType, texId_1, 0);

	this->PostPresentFrame();
}

void SdiPresentFrame::PresentFrameDual(GLenum texType, GLuint texId_0, GLuint texId_1, GLuint64EXT minPresentTime)
{
	this->PrePresentFrame();

	GLuint presentTimeID = m_PresentID[mStats.cur_query];
	GLuint presentDurationID = m_DurationID[mStats.cur_query];
	

	glPresentFrameDualFillNV(1, minPresentTime,
		presentTimeID, presentDurationID,
		GL_FRAME_NV,
		texType, texId_0,
		GL_NONE, 0,
		texType, texId_1,
		GL_NONE, 0);

	this->PostPresentFrame();
}



void SdiPresentFrame::PresentFrameDual(GLenum texType, GLuint texId_0, GLuint texId_1, GLuint texId_2, GLuint texId_3, GLuint64EXT minPresentTime)
{
	this->PrePresentFrame();

	GLuint presentTimeID = m_PresentID[mStats.cur_query];
	GLuint presentDurationID = m_DurationID[mStats.cur_query];
	

	glPresentFrameDualFillNV(1, minPresentTime,
		presentTimeID, presentDurationID,
		GL_FIELDS_NV,
		texType, texId_0,
		texType, texId_1,
		texType, texId_2,
		texType, texId_3);

	this->PostPresentFrame();
}



void SdiPresentFrame::PresentFrame(gl::Texture2D& tex, GLuint64EXT minPresentTime)
{
	this->PrePresentFrame();

	GLuint presentTimeID = m_PresentID[mStats.cur_query];
	GLuint presentDurationID = m_DurationID[mStats.cur_query];
	

	glPresentFrameKeyedNV(1, minPresentTime,
		presentTimeID, presentDurationID,
		GL_FRAME_NV,
		tex.Type(), tex.Id(), 0,
		GL_NONE, 0, 0);

	this->PostPresentFrame();
}


void SdiPresentFrame::PresentFrame(gl::Texture2D& tex_0, gl::Texture2D& tex_1, GLuint64EXT minPresentTime)
{
	this->PrePresentFrame();

	GLuint presentTimeID = m_PresentID[mStats.cur_query];
	GLuint presentDurationID = m_DurationID[mStats.cur_query];
	


	glPresentFrameKeyedNV(1, minPresentTime,
		presentTimeID, presentDurationID,
		GL_FIELDS_NV,
		tex_0.Type(), tex_0.Id(), 0,
		tex_1.Type(), tex_1.Id(), 0);

	this->PostPresentFrame();
}


void SdiPresentFrame::PresentFrameDual(gl::Texture2D& tex_0, gl::Texture2D& tex_1, GLuint64EXT minPresentTime)
{
	this->PrePresentFrame();

	GLuint presentTimeID = m_PresentID[mStats.cur_query];
	GLuint presentDurationID = m_DurationID[mStats.cur_query];
	

	glPresentFrameDualFillNV(1, minPresentTime,
		presentTimeID, presentDurationID,
		GL_FRAME_NV,
		tex_0.Type(), tex_0.Id(),
		GL_NONE, 0,
		tex_1.Type(), tex_1.Id(),
		GL_NONE, 0);

	this->PostPresentFrame();
}


void SdiPresentFrame::PresentFrameDual(gl::Texture2D& tex_0, gl::Texture2D& tex_1, gl::Texture2D& tex_2, gl::Texture2D& tex_3, GLuint64EXT minPresentTime)
{
	this->PrePresentFrame();

	GLuint presentTimeID = m_PresentID[mStats.cur_query];
	GLuint presentDurationID = m_DurationID[mStats.cur_query];
	

	glPresentFrameDualFillNV(1, minPresentTime,
		presentTimeID, presentDurationID,
		GL_FIELDS_NV,
		tex_0.Type(), tex_0.Id(),
		tex_1.Type(), tex_1.Id(),
		tex_2.Type(), tex_2.Id(),
		tex_3.Type(), tex_3.Id());

	this->PostPresentFrame();
}

