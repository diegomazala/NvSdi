using UnityEngine;
using System.Collections;

[AddComponentMenu("Diem/Video In SDI - GLNvSdiIn")]
public class GLNvDvpInput : UtyGLNvSdi 
{
    public Material[] dvpMaterials = new Material[MAX_INPUT_DEVICES * MAX_INPUT_STREAMS];

    public RenderTexture[] dvpTexture = new RenderTexture[MAX_INPUT_DEVICES * MAX_INPUT_STREAMS];

    private IEnumerator InputCoroutine = null;
    private bool dvpEnabled = false;

    void Awake()
    {
        InputCoroutine = SdiInputCoroutine();
    }

    void OnEnable()
    {
        dvpEnabled = false;
          
#if !UNITY_EDITOR
        if (!GLNvSdiOptions.Load(UtyGLNvSdi.ConfigFileName, ref options))
            GLNvSdiOptions.Save(UtyGLNvSdi.ConfigFileName, options);
#endif

        if (options.logToFile)
            UtyGLNvSdi.SdiSetupLogFile();

        StartCoroutine(InputCoroutine);
    }


    void OnDisable()
    {        
        if (dvpEnabled)
        {
            StopCoroutine(InputCoroutine);

            GL.IssuePluginEvent(UtyGLNvSdi.GetDvpRenderEventFunc(), (int)SdiRenderEvent.Shutdown);
            dvpEnabled = false;
        }

        DestroyTextures();
    }


    private IEnumerator SdiInputCoroutine()
    {
        yield return new WaitForEndOfFrame();


        // Issue a plugin event with an integer identifier.
        // The plugin can distinguish between different based on this ID.
        GL.IssuePluginEvent(UtyGLNvSdi.GetDvpRenderEventFunc(), (int)SdiRenderEvent.Initialize);
        yield return new WaitForEndOfFrame();

        GL.IssuePluginEvent(UtyGLNvSdi.GetDvpRenderEventFunc(), (int)SdiRenderEvent.PreSetup);
        yield return new WaitForEndOfFrame();

        dvpEnabled = UtyGLNvSdi.DvpIsOk();

        if (dvpEnabled)
        {
            for (int i = 0; i < UtyGLNvSdi.DvpActiveDeviceCount(); ++i)
                CreateTextures(i, UtyGLNvSdi.DvpStreamsPerFrame(i), UtyGLNvSdi.DvpWidth(), UtyGLNvSdi.DvpHeight());
                      
        }

        GL.IssuePluginEvent(UtyGLNvSdi.GetDvpRenderEventFunc(), (int)SdiRenderEvent.Setup);
        GL.IssuePluginEvent(UtyGLNvSdi.GetDvpRenderEventFunc(), (int)SdiRenderEvent.StartCapture);

        dvpEnabled = UtyGLNvSdi.DvpIsOk();

        while (dvpEnabled)
        {
            // Wait until all frame rendering is done
            yield return new WaitForEndOfFrame();

            // Capture frame from device
            GL.IssuePluginEvent(UtyGLNvSdi.GetDvpRenderEventFunc(), (int)SdiRenderEvent.CaptureFrame);
        }
    }



    void CreateTextures(int device_index, int video_stream_count, int texWidth, int texHeight)
    {
        for (int j = 0; j < video_stream_count; ++j)
        {
            int i = device_index * MAX_INPUT_STREAMS + j;

            dvpTexture[i] = new RenderTexture(texWidth, texHeight, 32, RenderTextureFormat.ARGB32);
            dvpTexture[i].name = name + i.ToString();
            dvpTexture[i].isPowerOfTwo = false;
            dvpTexture[i].wrapMode = TextureWrapMode.Clamp;
            dvpTexture[i].Create();

            UtyGLNvSdi.DvpSetDisplayTexturePtr(dvpTexture[i].GetNativeTexturePtr(), device_index, j);

            if (dvpMaterials[i] != null)
                dvpMaterials[i].mainTexture = dvpTexture[i];
        }
    }



    void DestroyTextures()
    {
        for (int i = 0; i < MAX_INPUT_DEVICES * MAX_INPUT_STREAMS; ++i)
        {
            if (dvpTexture[i] != null && dvpTexture[i].IsCreated())
            {
                dvpTexture[i].Release();
                dvpTexture[i] = null;
            }
        }
    }



}
