using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

[AddComponentMenu("Diem/Video IO SDI - Dvp")]
public class DvpIO : MonoBehaviour
{
    private const int MaxOutputTextureCount = 4;

    public bool useInputAsBackground = true;

    public RenderTexture[] inputVideoTexture = new RenderTexture[MaxOutputTextureCount];
    public RenderTexture[] outputVideoTexture = new RenderTexture[MaxOutputTextureCount];

    private IEnumerator IOCoroutine = null;
    private bool sdiEnabled = false;


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


    private class Plugin
    {
        [DllImport("GLNvSdi")]
        public static extern bool DvpIsOk();
        [DllImport("GLNvSdi")]
        public static extern bool DvpInputIsAvailable();
        [DllImport("GLNvSdi")]
        public static extern bool DvpOutputIsAvailable();
        [DllImport("GLNvSdi")]
        public static extern int DvpInputActiveDeviceCount();
        [DllImport("GLNvSdi")]
        public static extern int DvpInputStreamsPerFrame(int device_index);
        [DllImport("GLNvSdi")]
        public static extern void DvpInputSetTexturePtr(System.IntPtr texture, int device_index, int video_stream_index);
        [DllImport("GLNvSdi")]
        public static extern void DvpOutputSetTexturePtr(System.IntPtr texture, int video_stream_index);
        [DllImport("GLNvSdi")]
        public static extern System.IntPtr GetGLNvDvpRenderEventFunc();
    }


    //void OnPreRender()
    //{
    //    if (sdiEnabled)
    //        GL.IssuePluginEvent(Plugin.GetGLNvDvpRenderEventFunc(), (int)DvpRenderEvent.UpdateInput);
    //}

    //void OnPostRender()
    //{
    //    if (sdiEnabled)
    //        GL.IssuePluginEvent(Plugin.GetGLNvDvpRenderEventFunc(), (int)DvpRenderEvent.UpdateOutput);
    //}

    void OnEnable()
    {
        sdiEnabled = false;
        IOCoroutine = SdiIOCoroutine();

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


        if (Plugin.DvpInputIsAvailable() && Plugin.DvpOutputIsAvailable())
        {
            // Initialize sdi
            GL.IssuePluginEvent(Plugin.GetGLNvDvpRenderEventFunc(), (int)DvpRenderEvent.Initialize);
            yield return new WaitForEndOfFrame();

            //
            // Setup textures input/output textures
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

                    if (useInputAsBackground)
                    {
                        Plugin.DvpOutputSetTexturePtr(inputVideoTexture[render_tex_index].GetNativeTexturePtr(), j);
                    }
                    else
                    {
                        if (!outputVideoTexture[j].IsCreated())
                            outputVideoTexture[j].Create();
                        Plugin.DvpOutputSetTexturePtr(outputVideoTexture[j].GetNativeTexturePtr(), j);
                    }
                }
            }

            // Setup sdi
            GL.IssuePluginEvent(Plugin.GetGLNvDvpRenderEventFunc(), (int)DvpRenderEvent.Setup);
            sdiEnabled = Plugin.DvpIsOk();

            if (!sdiEnabled)
                Debug.LogError("Could not setup Dvp");

            // Render Loop
            while (sdiEnabled)
            {
                // Wait until all frame rendering is done
                yield return new WaitForEndOfFrame();

                // Capture frame from device and Present frame
                GL.IssuePluginEvent(Plugin.GetGLNvDvpRenderEventFunc(), (int)DvpRenderEvent.Update);

                sdiEnabled = Plugin.DvpIsOk();
            }
        }

    }

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.Space))
        {
            Debug.Log("set input");
            //
            // Setup textures output textures
            // 
            for (int i = 0; i < MaxOutputTextureCount; ++i)
            {
                if (!outputVideoTexture[i].IsCreated())
                    outputVideoTexture[i].Create();
                Plugin.DvpOutputSetTexturePtr(outputVideoTexture[i].GetNativeTexturePtr(), i);
            }
        }
    }

}
