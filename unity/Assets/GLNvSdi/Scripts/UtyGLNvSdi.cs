using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System.Xml.Serialization;
using System.Globalization;


[System.Serializable]
public enum SdiRenderEvent
{
    Initialize,
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
    SDI_SYNC,
	COMP_SYNC,
	NONE
};

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
    public int inputRingBufferSize;
    public int flipQueueLength;
    public bool dualOutput;
    public int hDelay;
    public int vDelay;
    public bool invertFields;

    public GLNvSdiOptions()
    {
        videoFormat = SdiVideoFormat.HD_1080I_59_94;
        inputRingBufferSize = 2;
        dualOutput = false;
        flipQueueLength = 2;
        hDelay = 0;
        vDelay = 0;
        invertFields = false;
    }


    static public bool ReadXml(string xmlFileName, ref GLNvSdiOptions rOptions)
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

    static public bool WriteXml(string xmlFileName, GLNvSdiOptions options)
    {
        try
        {
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


public class UtyGLNvSdi
{
    public const int LOG_MAX_SIZE = 512;
    public static System.Text.StringBuilder Log = null;

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
    public static extern bool SdiMakeCurrent();

    [DllImport("GLNvSdi")]
    public static extern void SdiSetUtyDC();
    [DllImport("GLNvSdi")]
    public static extern void SdiSetUtyGLRC();
    [DllImport("GLNvSdi")]
    public static extern bool SdiMakeUtyCurrent();


    [DllImport("GLNvSdi")]
    public static extern bool SdiOutputInitialize();
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputUninitialize();
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputSetGlobalOptions();
    [DllImport("GLNvSdi")]
    public static extern void SdiOutputSetVideoFormat(SdiVideoFormat video_format, SdiSyncSource sync_source, int h_delay, int v_delay, bool dual_output, int flip_queue_lenght);
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
    public static extern System.IntPtr GetSdiOutputRenderEventFunc();


    [DllImport("GLNvSdi")]
    public static extern bool SdiInputInitialize();
    [DllImport("GLNvSdi")]
    public static extern void SdiInputUninitialize();
    [DllImport("GLNvSdi")]
    public static extern void SdiInputSetGlobalOptions(int ringBufferSizeInFrames);
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
    public static extern System.IntPtr GetSdiInputRenderEventFunc();


    public static void GetSizeFromVideoFormat(SdiVideoFormat video_format, ref int rWidth, ref int rHeight, 
                                                                        ref float rAspect, ref bool rIsInterlaced)
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

    static public string sdiConfigFolder
    {
        get
        {
#if UNITY_EDITOR
            System.IO.DirectoryInfo mainFolderPath = new System.IO.DirectoryInfo(Application.dataPath + "/");
#else
            System.IO.DirectoryInfo mainFolderPath = new System.IO.DirectoryInfo(Application.dataPath + "/../");
#endif
            return mainFolderPath.FullName + @"Config/Sdi/";
        }
    }
}

