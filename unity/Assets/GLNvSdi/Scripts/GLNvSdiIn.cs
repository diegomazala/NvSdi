using UnityEngine;
using System.Collections;

[AddComponentMenu("Diem/Video In SDI - GLNvSdiIn")]
public class GLNvSdiIn : UtyGLNvSdi 
{
    public Material[] sdiMaterials = { null, null, null, null, null, null, null, null };

    private RenderTexture[] sdiTexture = { null, null, null, null, null, null, null, null };

    private IEnumerator InputCoroutine = null;
    private bool sdiEnabled = false;

    void OnEnable()
    {
        sdiEnabled = false;
        InputCoroutine = SdiInputCoroutine();

  
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
        if (sdiEnabled)
        {
            StopCoroutine(InputCoroutine);

            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.Shutdown);
            sdiEnabled = false;
        }

        DestroyTextures();
    }


    private IEnumerator SdiInputCoroutine()
    {
        yield return new WaitForEndOfFrame();

        // Issue a plugin event with an integer identifier.
        // The plugin can distinguish between different based on this ID.
        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.Initialize);
        yield return new WaitForEndOfFrame();

        if (options.inputCaptureFields)
            CreateTextures(8, UtyGLNvSdi.SdiInputWidth(), UtyGLNvSdi.SdiInputHeight() / 2);
        else
            CreateTextures(4, UtyGLNvSdi.SdiInputWidth(), UtyGLNvSdi.SdiInputHeight());

        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.Setup);
        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.StartCapture);
        yield return new WaitForEndOfFrame();

        sdiEnabled = true;

        while (UtyGLNvSdi.SdiInputIsCapturing())
        {
            // Wait until all frame rendering is done
            yield return new WaitForEndOfFrame();

            // Get status of the capture (GL_SUCCESS_NV, GL_FAILURE_NV, GL_PARTIAL_SUCCESS_NV)
            //Debug.Log(UtyGLNvSdi.SdiInputCaptureStatus().ToString());

            // Capture frame from device
            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.CaptureFrame);
        }
        
    }



    void CreateTextures(int count, int texWidth, int texHeight)
    {
        for (int i = 0; i < count; ++i)
        {
            sdiTexture[i] = new RenderTexture(texWidth, texHeight, 32, RenderTextureFormat.ARGB32);
            sdiTexture[i].name = name;
            sdiTexture[i].isPowerOfTwo = false;
            sdiTexture[i].wrapMode = TextureWrapMode.Clamp;
            sdiTexture[i].Create();

            UtyGLNvSdi.SdiInputSetTexturePtr(i, sdiTexture[i].GetNativeTexturePtr(), sdiTexture[i].width, sdiTexture[i].height);

            int multiplier = options.inputCaptureFields ? 2 : 1;
            if (i < UtyGLNvSdi.SdiInputVideoCount() * multiplier)
				sdiMaterials[i].mainTexture = sdiTexture[i];
        }
    }



    void DestroyTextures()
    {
        for (int i = 0; i < MAX_COUNT; ++i)
        {
            if (sdiTexture[i] != null && sdiTexture[i].IsCreated())
            {
                sdiTexture[i].Release();
                sdiTexture[i] = null;
            }
        }
    }






}
