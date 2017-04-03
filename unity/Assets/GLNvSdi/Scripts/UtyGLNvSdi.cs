using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System.Xml.Serialization;
using System.Globalization;


[System.Serializable]
public enum SdiRenderEvent
{
    Initialize,
    PreSetup,
    Setup,
    StartCapture,
    StopCapture,
    CaptureFrame,
    PresentFrame,
    Shutdown
};



[System.Serializable]
public enum SdiVideoFormat
{
    HD_1080P_29_97,
    HD_1080P_30_00,
    HD_1080I_59_94,
    HD_1080I_60_00,
    HD_720P_29_97,
    HD_720P_30_00,
    HD_720P_59_94,
    HD_720P_60_00,
    SD_487I_59_94,
    UNKNOWN
};

[System.Serializable]
public enum SdiSyncSource
{
    NONE,
    COMP_SYNC,
    SDI_SYNC
};

[System.Serializable]
public enum SdiReturn
{
    GL_PARTIAL_SUCCESS_NV  = 0x902E,
    GL_SUCCESS_NV          = 0x902F,
    GL_FAILURE_NV          = 0x9030
};



[System.Serializable]
public class GLNvSdiOptions
{
    public SdiVideoFormat videoFormat;
	public SdiSyncSource syncSource;
    public bool inputCaptureFields;
    public int inputRingBufferSize;
    public int outputRingBufferSize;
    public float outputDelay;
    public int outputHorizontalDelay;
    public int outputVerticalDelay;
    public bool outputInvertFields;
    public bool outputDual;
    public bool useInputVideoAsBackground;
    public bool logToFile;
    public bool printStats;

    public GLNvSdiOptions()
    {
        videoFormat = SdiVideoFormat.HD_1080I_59_94;
        inputRingBufferSize = 5;
        outputDual = false;
        outputRingBufferSize = 5;
        outputDelay = 3.5f;
        outputHorizontalDelay = 0;
        outputVerticalDelay = 0;
        outputInvertFields = false;
        inputCaptureFields = true;
        useInputVideoAsBackground = true;
        logToFile = false;
    }


    static public bool Load(string xmlFileName, ref GLNvSdiOptions rOptions)
    {
        try
        {
            XmlSerializer xmlReader = new XmlSerializer(typeof(GLNvSdiOptions));
            System.IO.TextReader textReader = new System.IO.StreamReader(xmlFileName);
            rOptions = (GLNvSdiOptions)xmlReader.Deserialize(textReader);
            textReader.Close();
            return true;
        }
        catch (System.Exception e)
        {
            UnityEngine.Debug.LogError("Error: " + e.Message);
            return false;
        }
    }

    static public bool Save(string xmlFileName, GLNvSdiOptions options)
    {
        try
        {
            System.IO.FileInfo fileInfo = new System.IO.FileInfo(xmlFileName);
            if (!fileInfo.Exists)
                System.IO.Directory.CreateDirectory(fileInfo.Directory.FullName);
            
            XmlSerializer serializer = new XmlSerializer(typeof(GLNvSdiOptions));
            System.IO.TextWriter textWriter = new System.IO.StreamWriter(xmlFileName);
            serializer.Serialize(textWriter, options);
            textWriter.Close();
            return true;
        }
        catch (System.Exception e)
        {
            UnityEngine.Debug.LogError("Error: " + e.Message);
            return false;
        }
    }
}


public class UtyGLNvSdi : MonoBehaviour
{
    public const int LOG_MAX_SIZE = 512;
    public static System.Text.StringBuilder Log = null;

    public const int MAX_COUNT = 8;
    public const int NVAPI_MAX_VIO_DEVICES = 8;    
    public const int MAX_VIDEO_STREAMS = 8;

    public const int MAX_INPUT_DEVICES = 1; 
    public const int MAX_INPUT_STREAMS = 4;

    public const int MAX_OUTPUT_DEVICES = 1;
    public const int MAX_OUTPUT_STREAMS = 4; 

    public GLNvSdiOptions options;


    [DllImport("GLNvSdi")]
    public static extern void SdiClearLog();
    [DllImport("GLNvSdi")]
    private static extern void SdiGetLog(System.Text.StringBuilder sdiLogStrSB, int lenght);

    public static System.Text.StringBuilder SdiGetLog()
    {
        if (Log == null)
            Log = new System.Text.StringBuilder(LOG_MAX_SIZE);

        UtyGLNvSdi.SdiGetLog(UtyGLNvSdi.Log, UtyGLNvSdi.LOG_MAX_SIZE);

        return UtyGLNvSdi.Log;
    }

    [DllImport("GLNvSdi")]
    public static extern int SdiError();

  
    [DllImport("GLNvSdi")]
    public static extern void SdiSetupLogConsole();
    
    [DllImport("GLNvSdi")]
    public static extern void SdiSetupLogFile();


    [DllImport("GLNvSdi")]
    public static extern bool SdiAncGetTimeCode(System.IntPtr timecode_int8, int video_index);
    [DllImport("GLNvSdi")]
    public static extern bool SdiAncSetupInput();
    [DllImport("GLNvSdi")]
    public static extern void SdiAncCleanupInput();
    [DllImport("GLNvSdi")]
    public static extern bool SdiAncCapture();
    [DllImport("GLNvSdi")]
    public static extern void SdiAncPresent();
    [DllImport("GLNvSdi")]
    public static extern bool SdiAncSetupOutput();
    [DllImport("GLNvSdi")]
    public static extern void SdiAncCleanupOutput();

    

    [DllImport("GLNvSdi")]
    public static extern void SdiSetCurrentDC();
    [DllImport("GLNvSdi")]
    public static extern void SdiSetCurrentGLRC();
    [DllImport("GLNvSdi")]
    public static extern bool SdiMakeCurrentExternal();
    [DllImport("GLNvSdi")]
    public static extern int SdiGpuCount();


    [DllImport("GLNvSdi")]
    public static extern int SdiOutputGpuCount();
    [DllImport("GLNvSdi")]
    public static extern bool SdiOutputInitialize();
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputUninitialize();
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputSetGlobalOptions();
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputSetVideoFormat(SdiVideoFormat video_format, SdiSyncSource sync_source, float oudput_delay, int h_delay, int v_delay, bool dual_output, int flip_queue_lenght);
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputSetDelay(float delay);
    [DllImport("GLNvSdi")]
    public static extern bool SdiOutputSetupDevices();
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputCleanupDevices();
    [DllImport("GLNvSdi")]
    public static extern bool SdiOutputSetupGL();
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputCleanupGL();
    [DllImport("GLNvSdi")]
    public static extern bool SdiOutputBindVideo();
    [DllImport("GLNvSdi")]
    public static extern bool SdiOutputUnbindVideo();
    [DllImport("GLNvSdi")]
    public static extern bool SdiOutputStart();
    [DllImport("GLNvSdi")]
    public static extern bool SdiOutputStop();
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputComputePresentTimeFromCapture(bool compute);
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputSetMinPresentTime(ulong minPresentTime);
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputPresentFrame();
    [DllImport("GLNvSdi")]
    public static extern bool SdiOutpuIsPresentingFrames();
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputSetTexture(int index, int id);
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputSetTexturePtr(int index, System.IntPtr texture, int width, int height);
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputInvertFields(bool invert);
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputCreateTextures();
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputInitializeFbo();
    [DllImport("GLNvSdi")]
    public static extern int SdiOutputDuplicatedFrames();
    [DllImport("GLNvSdi")]
    public static extern int SdiOutputDuplicatedFramesCount();
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputResetDuplicatedFramesCount();
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputPrintStats(bool print);
    [DllImport("GLNvSdi")]
    public static extern System.IntPtr GetSdiOutputRenderEventFunc();


    [DllImport("GLNvSdi")]
    public static extern int SdiInputGpuCount();
    [DllImport("GLNvSdi")]
    public static extern bool SdiInputInitialize();
    [DllImport("GLNvSdi")]
    public static extern void SdiInputUninitialize();
    [DllImport("GLNvSdi")]
    public static extern void SdiInputSetBufferSize(int ringBufferSizeInFrames);
    [DllImport("GLNvSdi")]
    public static extern void SdiInputSetCaptureFields(bool captureFields);
    [DllImport("GLNvSdi")]
    public static extern void SdiInputSetGlobalOptions();
    [DllImport("GLNvSdi")]
    public static extern bool SdiInputCreateTextures();
    [DllImport("GLNvSdi")]
    public static extern bool SdiInputSetupDevices();
    [DllImport("GLNvSdi")]
    public static extern void SdiInputCleanupDevices();
    [DllImport("GLNvSdi")]
    public static extern bool SdiInputSetupGL();
    [DllImport("GLNvSdi")]
    public static extern void SdiInputCleanupGL();
    [DllImport("GLNvSdi")]
    public static extern bool SdiInputBindVideoTextureFrame();
    [DllImport("GLNvSdi")]
    public static extern bool SdiInputUnbindVideoTextureFrame();
    [DllImport("GLNvSdi")]
    public static extern bool SdiInputBindVideoTextureField();
    [DllImport("GLNvSdi")]
    public static extern bool SdiInputUnbindVideoTextureField();
    [DllImport("GLNvSdi")]
    public static extern uint SdiInputCaptureVideo();
    [DllImport("GLNvSdi")]
    public static extern int SdiInputWidth();
    [DllImport("GLNvSdi")]
    public static extern int SdiInputHeight();
    [DllImport("GLNvSdi")]
    public static extern int SdiInputVideoCount();
    [DllImport("GLNvSdi")]
    public static extern void SdiInputSetTexture(int index, int id);
    [DllImport("GLNvSdi")]
    public static extern void SdiInputSetTexturePtr(int index, System.IntPtr texture, int width, int height);
    [DllImport("GLNvSdi")]
    public static extern bool SdiInputIsCapturing();
    [DllImport("GLNvSdi")]
    public static extern bool SdiInputStart();
    [DllImport("GLNvSdi")]
    public static extern void SdiInputStop();
    [DllImport("GLNvSdi")]
    public static extern float SdiInputGpuTime();
    [DllImport("GLNvSdi")]
    public static extern float SdiInputGviTime();
    [DllImport("GLNvSdi")]
    public static extern int SdiInputDroppedFrames();
    [DllImport("GLNvSdi")]
    public static extern int SdiInputDroppedFramesCount();
    [DllImport("GLNvSdi")]
    public static extern void SdiInputResetDroppedFramesCount();
    [DllImport("GLNvSdi")]
    public static extern SdiReturn SdiInputCaptureStatus();
    [DllImport("GLNvSdi")]
    public static extern System.UInt64 SdiInputCaptureTime();
    [DllImport("GLNvSdi")]
    public static extern float SdiInputFrameRate();
    [DllImport("GLNvSdi")]
    public static extern double SdiInputFrameRateNanoSec();
    [DllImport("GLNvSdi")]
    public static extern System.IntPtr GetSdiInputRenderEventFunc();



    [DllImport("GLNvSdi")]
    public static extern bool DvpIsOk();
    [DllImport("GLNvSdi")]
    public static extern int DvpWidth();
    [DllImport("GLNvSdi")]
    public static extern int DvpHeight();
    [DllImport("GLNvSdi")]
    public static extern int DvpActiveDeviceCount();
    [DllImport("GLNvSdi")]
    public static extern int DvpStreamsPerFrame(int device_index = 0);
    [DllImport("GLNvSdi")]
    public static extern ulong DvpFrameNumber(int device_index = 0);
    [DllImport("GLNvSdi")]
    public static extern ulong DvpDroppedFrames(int device_index = 0);
    [DllImport("GLNvSdi")]
    public static extern float DvpCaptureElapsedTime(int device_index = 0);
    [DllImport("GLNvSdi")]
    public static extern float DvpUploadElapsedTime(int device_index = 0);
    [DllImport("GLNvSdi")]
    public static extern float DvpDownloadElapsedTime(int device_index = 0);
    [DllImport("GLNvSdi")]
    public static extern void DvpSetDisplayTexturePtr(System.IntPtr texture, int device_index = 0, int video_stream_index = 0);
    [DllImport("GLNvSdi")]
    public static extern System.IntPtr GetDvpRenderEventFunc();


    public static void GetSizeFromVideoFormat(
        SdiVideoFormat video_format, 
        ref int rWidth, 
        ref int rHeight, 
        ref float rAspect, 
        ref bool rIsInterlaced)
    {
        if (video_format <= SdiVideoFormat.HD_1080I_60_00)
        {
            // HD FULL : 1920 x 1080
            rWidth = 1920;
            rHeight = 1080;
            rAspect = 1.777778f;

            if (video_format >= SdiVideoFormat.HD_1080I_59_94)
                rIsInterlaced = true;
            else
                rIsInterlaced = false;
        }
        else if (video_format >= SdiVideoFormat.HD_720P_29_97 &&
                video_format <= SdiVideoFormat.HD_720P_60_00)
        {
            // HD: 1280 x 720
            rWidth = 1280;
            rHeight = 720;
            rAspect = 1.777778f;
            rIsInterlaced = false;
        }
        else if (video_format >= SdiVideoFormat.SD_487I_59_94)
        {
            // SD: 720 x 486
            rWidth = 720;
            rHeight = 487; // 486;
            rAspect = 1.33333f;
            rIsInterlaced = true;
        }
        else
        {
            rWidth = 0;
            rHeight = 0;
            rAspect = 1.0f;
            rIsInterlaced = false;
        }
    }

    public static void GetSizeFromVideoFormat(SdiVideoFormat video_format, ref int rWidth, ref int rHeight)
    {
        float lAspect = 0;
        bool  lInterlaced = false;
        GetSizeFromVideoFormat(video_format, ref rWidth, ref rHeight, ref lAspect, ref lInterlaced);
    }

    static public string ConfigFileName
    {
        get
        {
            string path = Application.dataPath + "/";
#if !UNITY_EDITOR
            path += "../";
#endif
            return (new System.IO.DirectoryInfo(path)).FullName + @"Config/Sdi/Sdi.xml";
        }
    }
}

