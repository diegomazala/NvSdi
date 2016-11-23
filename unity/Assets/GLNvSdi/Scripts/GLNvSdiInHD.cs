using UnityEngine;
using System.Collections;

[AddComponentMenu("Diem/Video In SDI - GLNvSdiInHD")]
public class GLNvSdiInHD : UtyGLNvSdi 
{
    public RenderTexture[] sdiInputTexture = { null, null, null, null};

    private IEnumerator InputCoroutine = null;
    private bool sdiEnabled = false;

    void OnEnable()
    {
        if (!SystemInfo.graphicsDeviceVersion.StartsWith("OpenGL"))
        {
            enabled = false;
            return;
        }

        sdiEnabled = false;
        InputCoroutine = SdiInputCoroutine();

  
#if !UNITY_EDITOR
        if (!GLNvSdiOptions.Load(UtyGLNvSdi.ConfigFileName, ref options))
            GLNvSdiOptions.Save(UtyGLNvSdi.ConfigFileName, options);
#endif

        if (options.logToFile)
            UtyGLNvSdi.SdiSetupLogFile();

        options.inputCaptureFields = false;

        StartCoroutine(InputCoroutine);
    }


    void OnDisable()
    {        
        if (sdiEnabled)
        {
            StopCoroutine(InputCoroutine);

            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.Shutdown);
            sdiEnabled = false;
        }
    }


    private IEnumerator SdiInputCoroutine()
    {
        yield return new WaitForEndOfFrame();

        // Issue a plugin event with an integer identifier.
        // The plugin can distinguish between different based on this ID.
        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.Initialize);
        yield return new WaitForEndOfFrame();

        options.inputCaptureFields = false;
        UtyGLNvSdi.SdiInputSetGlobalOptions(options.inputRingBufferSize, options.inputCaptureFields);   // capture_fields = false

        SetupTextures();

        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.Setup);
        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.StartCapture);
        yield return new WaitForEndOfFrame();

        sdiEnabled = true;

        while (UtyGLNvSdi.SdiInputIsCapturing())
        {
            // Wait until all frame rendering is done
            yield return new WaitForEndOfFrame();

            // Capture frame from device
            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.CaptureFrame);
        }
        
    }



    void SetupTextures()
    {
        for (int i = 0; i < sdiInputTexture.Length; ++i)
        {
            if (sdiInputTexture[i] == null)
                continue;

            if (!sdiInputTexture[i].IsCreated())
            {
                if (!sdiInputTexture[i].Create())
                    Debug.LogError("Could not create sdi input texture");
            }

            UtyGLNvSdi.SdiInputSetTexturePtr(i, sdiInputTexture[i].GetNativeTexturePtr(), sdiInputTexture[i].width, sdiInputTexture[i].height);
        }
    }


}
