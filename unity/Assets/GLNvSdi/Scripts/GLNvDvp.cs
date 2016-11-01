using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System.Xml.Serialization;

[System.Serializable]
public enum DvpRenderEvent
{
    CheckAvalability,
    Initialize,
    Setup,
    UpdateInput,
    Update,
    UpdateOutput,
    Cleanup,
    Uninitialize
};


[System.Serializable]
public enum DvpSyncSource
{
    SDI_SYNC,
    COMP_SYNC,
    NONE
};



[System.Serializable]
public enum DvpVideoFormat
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

[AddComponentMenu("Diem/Video IO SDI - Dvp")]
public class GLNvDvp : MonoBehaviour
{
    public bool inputEnabled = true;
    public bool outputEnabled = true;

    public DvpOptions options;
    private const int MaxOutputTextureCount = 4;

    public RenderTexture[] inputVideoTexture = new RenderTexture[MaxOutputTextureCount];
    public RenderTexture[] outputVideoTexture = new RenderTexture[MaxOutputTextureCount];

    private IEnumerator IOCoroutine = null;
    private bool sdiEnabled = false;

    
   

    public class Plugin
    {
        [DllImport("GLNvSdi")]
        public static extern bool DvpIsOk();
        [DllImport("GLNvSdi")]
        public static extern bool DvpInputIsAvailable();
        [DllImport("GLNvSdi")]
        public static extern bool DvpOutputIsAvailable();
        [DllImport("GLNvSdi")]
        public static extern void DvpInputDisable();
        [DllImport("GLNvSdi")]
        public static extern void DvpOutputDisable();
        [DllImport("GLNvSdi")]
        public static extern void DvpSetOptions(DvpSyncSource syncSource, int inputBufferSize, int outputBufferSize, int hDelay, int vDelay);
        [DllImport("GLNvSdi")]
        public static extern int DvpInputActiveDeviceCount();
        [DllImport("GLNvSdi")]
        public static extern int DvpInputStreamsPerFrame(int device_index);
        [DllImport("GLNvSdi")]
        public static extern void DvpInputSetTexturePtr(System.IntPtr texture, int device_index, int video_stream_index);
        [DllImport("GLNvSdi")]
        public static extern void DvpOutputSetTexturePtr(System.IntPtr texture, int video_stream_index);
        [DllImport("GLNvSdi")]
        public static extern uint DvpInputDroppedFrames(int device_index = 0);
        [DllImport("GLNvSdi")]
        public static extern uint DvpDroppedFrames(int device_index = 0);
        [DllImport("GLNvSdi")]
        public static extern void DvpResetDroppedFrames();
        [DllImport("GLNvSdi")]
        public static extern float DvpInputCaptureElapsedTime(int device_index = 0);
        [DllImport("GLNvSdi")]
        public static extern DvpVideoFormat DvpInputVideoFormat();
        [DllImport("GLNvSdi")]
        public static extern uint DvpOutputDuplicatedFramesCount();
        [DllImport("GLNvSdi")]
        public static extern void DvpOutputResetDuplicatedFramesCount();
        [DllImport("GLNvSdi")]
        public static extern System.IntPtr GetGLNvDvpRenderEventFunc();
    }

    public bool IsRunning
    {
        get { return sdiEnabled; }
    }


    void OnEnable()
    {
        sdiEnabled = false;
        IOCoroutine = SdiIOCoroutine();

#if !UNITY_EDITOR
        if (!DvpOptions.Load(DvpOptions.ConfigFileName, ref options))
            DvpOptions.Save(DvpOptions.ConfigFileName, options);
#endif

        StartCoroutine(IOCoroutine);
    }



    void OnDisable()
    {
        if (sdiEnabled)
        {
            StopCoroutine(IOCoroutine);

            GL.IssuePluginEvent(Plugin.GetGLNvDvpRenderEventFunc(), (int)DvpRenderEvent.Cleanup);
            GL.IssuePluginEvent(Plugin.GetGLNvDvpRenderEventFunc(), (int)DvpRenderEvent.Uninitialize);
            sdiEnabled = false;
        }
    }

    private IEnumerator SdiIOCoroutine()
    {
        yield return new WaitForEndOfFrame();
        GL.IssuePluginEvent(Plugin.GetGLNvDvpRenderEventFunc(), (int)DvpRenderEvent.CheckAvalability);
        yield return new WaitForEndOfFrame();

        if (!inputEnabled)
            Plugin.DvpInputDisable();

        if (!outputEnabled)
            Plugin.DvpOutputDisable();


        if (Plugin.DvpInputIsAvailable() && Plugin.DvpOutputIsAvailable())
        {
            Plugin.DvpSetOptions(options.syncSource, 
                options.inputRingBufferSize, options.outputRingBufferSize,
                options.outputHorizontalDelay, options.outputVerticalDelay);


            // Initialize sdi
            GL.IssuePluginEvent(Plugin.GetGLNvDvpRenderEventFunc(), (int)DvpRenderEvent.Initialize);
            yield return new WaitForEndOfFrame();


            
             //Setup textures output textures
             
            if (!options.passthrough)
            {
                for (int i = 0; i < MaxOutputTextureCount; ++i)
                {
                    if (!outputVideoTexture[i].IsCreated())
                        outputVideoTexture[i].Create();
                    Plugin.DvpOutputSetTexturePtr(outputVideoTexture[i].GetNativeTexturePtr(), i);
                }
            }


            //
            // Setup textures input textures
            // 
            int devices = Plugin.DvpInputActiveDeviceCount();
            for (int i = 0; i < devices; ++i)
            {
                int streams = Plugin.DvpInputStreamsPerFrame(i);
                for (int j = 0; j < streams; ++j)
                {
                    int render_tex_index = i * streams + j;
                    if (!inputVideoTexture[render_tex_index].IsCreated())
                        inputVideoTexture[render_tex_index].Create();

                    Plugin.DvpInputSetTexturePtr(inputVideoTexture[render_tex_index].GetNativeTexturePtr(), i, j);

                    if (options.passthrough)
                    {
                        Plugin.DvpOutputSetTexturePtr(inputVideoTexture[render_tex_index].GetNativeTexturePtr(), j);
                    }
                }
            }
                        

            // Setup sdi
            GL.IssuePluginEvent(Plugin.GetGLNvDvpRenderEventFunc(), (int)DvpRenderEvent.Setup);
            sdiEnabled = Plugin.DvpIsOk();

            options.videoFormat = Plugin.DvpInputVideoFormat();

            if (!sdiEnabled)
                Debug.LogError("Could not setup Dvp");
        }

    }

    public uint dropFramesIn = 0;
    public uint dropFramesOut = 0;
    public uint duplicateOut = 0;
    void Update()
    {
        if (sdiEnabled)
        {
            GL.IssuePluginEvent(GLNvDvp.Plugin.GetGLNvDvpRenderEventFunc(), (int)DvpRenderEvent.Update);

            dropFramesIn = GLNvDvp.Plugin.DvpInputDroppedFrames();
            dropFramesOut = GLNvDvp.Plugin.DvpDroppedFrames();
            duplicateOut = GLNvDvp.Plugin.DvpOutputDuplicatedFramesCount();
        }
    }


}





[System.Serializable]
public class DvpOptions
{
    public DvpVideoFormat videoFormat;
    public DvpSyncSource syncSource;
    public int inputRingBufferSize;
    public int outputRingBufferSize;
    public int outputHorizontalDelay;
    public int outputVerticalDelay;
    public bool passthrough;

    public DvpOptions()
    {
        videoFormat = DvpVideoFormat.UNKNOWN;
        syncSource = DvpSyncSource.NONE;
        inputRingBufferSize = 3;
        outputRingBufferSize = 3;
        outputHorizontalDelay = 0;
        outputVerticalDelay = 0;
        passthrough = false;
    }


    static public string ConfigFileName
    {
        get
        {
            string path = Application.dataPath + "/";
#if !UNITY_EDITOR
            path += "../";
#endif
            return (new System.IO.DirectoryInfo(path)).FullName + @"Config/Sdi/Dvp.xml";
        }
    }

    static public bool Load(string xmlFileName, ref DvpOptions rOptions)
    {
        try
        {
            XmlSerializer xmlReader = new XmlSerializer(typeof(DvpOptions));
            System.IO.TextReader textReader = new System.IO.StreamReader(xmlFileName);
            rOptions = (DvpOptions)xmlReader.Deserialize(textReader);
            textReader.Close();
            return true;
        }
        catch (System.Exception e)
        {
            UnityEngine.Debug.LogError("Error: " + e.Message);
            return false;
        }
    }

    static public bool Save(string xmlFileName, DvpOptions options)
    {
        try
        {
            System.IO.FileInfo fileInfo = new System.IO.FileInfo(xmlFileName);
            if (!fileInfo.Exists)
                System.IO.Directory.CreateDirectory(fileInfo.Directory.FullName);

            XmlSerializer serializer = new XmlSerializer(typeof(DvpOptions));
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

