#ifndef __SDI_OPTIONS_H__
#define __SDI_OPTIONS_H__

#include "GLNvSdiCore.h"
#include "CommandLine.h"


enum SdiVideoFormat
{
	HD_1080P_29_97,
	HD_1080P_30_00,
	HD_1080I_59_94,
	HD_1080I_60_00,
	HD_720P_29_97,
	HD_720P_30_00,
	HD_720P_59_94,
	HD_720P_60_00,
	SD_487I_59_94
};

enum SdiSyncSource
{
	SDI_SYNC,
	COMP_SYNC,
	NONE
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class	SdiOptions
///
/// \brief	Struct with the parameters to be set the Sdi video transfer
///
/// \author	Diego
/// \date	March 2011
////////////////////////////////////////////////////////////////////////////////////////////////////
struct GLNVSDI_API SdiOptions : public Options
{
	int inputRingBufferSize;
	float outputDelay;
	int antiAliasing;
	bool invertFields;
	bool captureFields;

	SdiVideoFormat mSdiVideoFormat;

	SdiOptions();
	
	void SetVideoFormat(SdiVideoFormat video_format, SdiSyncSource sync_source, float outputDelay, int h_delay = 0, int v_delay = 0, bool dualOutput = false);
	void SetOutputDelay(float delay);
	void InvertFields(bool invert);
	bool InvertFields() const;
	void SetDualOutput(bool dual = true);
	bool IsDualOutput() const;
	
	static SdiVideoFormat GetVideoFormatFromNV(NVVIOSIGNALFORMAT video_format);
};




#endif	// __SDI_OPTIONS_H__
