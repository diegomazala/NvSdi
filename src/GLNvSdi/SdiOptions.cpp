#include "SdiOptions.h"

SdiOptions::SdiOptions(): Options(),	
					antiAliasing(4),
					mSdiVideoFormat(HD_1080I_59_94)
{
	this->cscEnable = FALSE;
	this->cscOffset[0] = 0.0625; this->cscOffset[1] = 0.5; this->cscOffset[2] = 0.5;
	this->cscScale[0] = 0.85547; this->cscScale[1] = 0.875; this->cscScale[2] = 0.875;
	this->cscMatrix[0][0] = 0.2130f; this->cscMatrix[0][1] = 0.7156f; this->cscMatrix[0][2] = 0.0725f;
	this->cscMatrix[1][0] = 0.5000f; this->cscMatrix[1][1] = -0.4542f; this->cscMatrix[1][2] = -0.0455f;
	this->cscMatrix[2][0] = 0.1146f; this->cscMatrix[2][1] = -0.3350f; this->cscMatrix[2][2] = 0.5000f;
	this->gamma[0] = 1.0f; this->gamma[1] = 1.0f; this->gamma[2] = 1.0f;

	this->fsaa = antiAliasing;
	this->block = FALSE;
	this->fps = TRUE;
	this->log = FALSE;
	this->numFrames = 0;
	this->repeat = 1;
	this->videoInfo = TRUE;

	this->invertFields = false;
	this->captureFields = true;
}

void SdiOptions::InvertFields(bool invert)
{
	this->invertFields = invert;
}

bool SdiOptions::InvertFields() const
{
	return this->invertFields;
}

void SdiOptions::SetDualOutput(bool dual)
{
	if(dual)
		this->dataFormat = NVVIODATAFORMAT_DUAL_R8G8B8_TO_DUAL_YCRCB422;
	else
		this->dataFormat = NVVIODATAFORMAT_R8G8B8A8_TO_YCRCBA4224;
}

bool SdiOptions::IsDualOutput() const
{
	if(	this->dataFormat == NVVIODATAFORMAT_DUAL_R8G8B8_TO_DUAL_YCRCB422 ||
		this->dataFormat == NVVIODATAFORMAT_DUAL_X8X8X8_TO_DUAL_422_PASSTHRU)
	{
		return true;	// dual output
	}
	else
	{
		return false;	// single output
	}
}

void SdiOptions::SetVideoFormat(SdiVideoFormat video_format, SdiSyncSource sync_source, int h_delay, int v_delay, bool dualOutput)
{
	this->hDelay = h_delay;
	this->vDelay = v_delay;

	this->mSdiVideoFormat = video_format;
	
	switch(sync_source)
	{
		case SDI_SYNC:
			this->syncEnable = TRUE;
			this->syncSource = NVVIOSYNCSOURCE_SDISYNC;
			break;

		case COMP_SYNC:
			this->syncEnable = TRUE;
			this->syncSource = NVVIOSYNCSOURCE_COMPSYNC;
			break;

		case NONE:
		default:
			this->syncEnable = false;
			break;
	}
	

	switch(video_format)
	{
		case HD_1080P_29_97:
			this->videoFormat = NVVIOSIGNALFORMAT_1080P_29_97_SMPTE274;
			break;

		case HD_1080P_30_00:
			this->videoFormat = NVVIOSIGNALFORMAT_1080P_30_00_SMPTE274;
			break;

		case HD_1080I_59_94:
			this->videoFormat = NVVIOSIGNALFORMAT_1080I_59_94_SMPTE274;
			break;

		case HD_1080I_60_00:
			this->videoFormat = NVVIOSIGNALFORMAT_1080I_60_00_SMPTE274;
			break;

		case HD_720P_29_97:
			this->videoFormat = NVVIOSIGNALFORMAT_720P_29_97_SMPTE296;
			break;

		case HD_720P_30_00:
			this->videoFormat = NVVIOSIGNALFORMAT_720P_30_00_SMPTE296;
			break;

		case HD_720P_59_94:
			this->videoFormat = NVVIOSIGNALFORMAT_720P_59_94_SMPTE296;
			break;

		case HD_720P_60_00:
			this->videoFormat = NVVIOSIGNALFORMAT_720P_60_00_SMPTE296;
			break;

		case SD_487I_59_94:
		default:
			this->videoFormat = NVVIOSIGNALFORMAT_487I_59_94_SMPTE259_NTSC;
			mSdiVideoFormat = SD_487I_59_94;
			break;
	}

	this->SetDualOutput(dualOutput);
}

SdiVideoFormat SdiOptions::GetVideoFormatFromNV(NVVIOSIGNALFORMAT video_format)
{
	switch(video_format)
	{
		case NVVIOSIGNALFORMAT_1080P_29_97_SMPTE274:
			return HD_1080P_29_97;

		case NVVIOSIGNALFORMAT_1080P_30_00_SMPTE274:
			return HD_1080P_30_00;

		case NVVIOSIGNALFORMAT_1080I_59_94_SMPTE274:
			return HD_1080I_59_94;

		case NVVIOSIGNALFORMAT_1080I_60_00_SMPTE274:
			return HD_1080I_60_00;

		case NVVIOSIGNALFORMAT_720P_29_97_SMPTE296:
			return HD_720P_29_97;

		case NVVIOSIGNALFORMAT_720P_30_00_SMPTE296:
			return HD_720P_30_00;

		case NVVIOSIGNALFORMAT_720P_59_94_SMPTE296:
			return HD_720P_59_94;

		case NVVIOSIGNALFORMAT_720P_60_00_SMPTE296:
			return HD_720P_60_00;

		case NVVIOSIGNALFORMAT_487I_59_94_SMPTE259_NTSC:
			return SD_487I_59_94;

		default:
			return SD_487I_59_94;
	}
}

