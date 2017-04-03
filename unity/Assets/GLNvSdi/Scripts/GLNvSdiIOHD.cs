using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

[AddComponentMenu("Diem/Video IO SDI - GLNvSdiIO")]
public class GLNvSdiIOHD : UtyGLNvSdi
{
    public RenderTexture[] inputTexture = { null, null, null, null};
    public RenderTexture[] outputTexture = { null, null, null, null};
        

    private IEnumerator IOCoroutine = null;
    public bool sdiEnabled = false;

    void Awake()
    {
        options.inputCaptureFields = false;
    }


    void OnEnable()
    {
        if (!SystemInfo.graphicsDeviceVersion.StartsWith("OpenGL"))
        {
            enabled = false;
            return;
        }

        sdiEnabled = false;
        IOCoroutine = SdiIOCoroutine();

#if !UNITY_EDITOR
        if (!GLNvSdiOptions.Load(UtyGLNvSdi.ConfigFileName, ref options))
            GLNvSdiOptions.Save(UtyGLNvSdi.ConfigFileName, options);
#endif
        options.inputCaptureFields = false;

        if (options.logToFile)
            UtyGLNvSdi.SdiSetupLogFile();

        StartCoroutine(IOCoroutine);
    }


    void OnDisable()
    {
        if (sdiEnabled)
        {
            StopCoroutine(IOCoroutine);

            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.Shutdown);
            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.Shutdown);
            sdiEnabled = false;
        }
    }

    
    //void Update()
    //{
    //    UpdateFrame();
    //}

    //public void UpdateFrame()
    //{
    //    if (sdiEnabled)
    //    {
    //        // Capture frame from device
    //        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.CaptureFrame);
    //        // Present frame
    //        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.PresentFrame);
    //    }
    //}

    private IEnumerator SdiIOCoroutine()
    {
        yield return new WaitForEndOfFrame();

        UtyGLNvSdi.SdiInputSetBufferSize(options.inputRingBufferSize);
        UtyGLNvSdi.SdiInputSetCaptureFields(options.inputCaptureFields);

        //
        // Input/Ouput Initialization
        //
        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.Initialize);
        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.Initialize);
        yield return new WaitForEndOfFrame();


        if (UtyGLNvSdi.SdiError() == 0)
        {
            //
            // Input Setup
            //
            options.inputCaptureFields = false;
            SetupInputTextures();
            

            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.Setup);
            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.StartCapture);
            yield return new WaitForEndOfFrame();

            //
            // Output Setup
            //
            UtyGLNvSdi.SdiOutputSetGlobalOptions();
            UtyGLNvSdi.SdiOutputSetVideoFormat(
                options.videoFormat,
                options.syncSource,
                options.outputDelay,
                options.outputHorizontalDelay,
                options.outputVerticalDelay,
                options.outputDual,
                options.outputRingBufferSize);

            UtyGLNvSdi.SdiOutputComputePresentTimeFromCapture(true);

            yield return new WaitForEndOfFrame();

        
            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.Setup);


            int texWidth = 1920;
            int texHeight = 1080;
            bool interlaced = true;
            float aspect = 1.0f;

            UtyGLNvSdi.GetSizeFromVideoFormat(options.videoFormat, ref texWidth, ref texHeight, ref aspect, ref interlaced);
            if (!SetupOutputTextures())
            {
                UnityEngine.Debug.LogError("GLNvSdi_Plugin could not setup sdi textures for input/output");
            }

            sdiEnabled = true;

            while (true)
            {
                // Wait until all frame rendering is done
                yield return new WaitForEndOfFrame();

                // Capture frame from device
                GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.CaptureFrame);
                // Present frame
                GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.PresentFrame);
            }
        }

    }

   


    bool SetupOutputTextures()
    {
        int texWidth = 1920;	// HD=1920, SD=720
        int texHeight = 1080;	// HD=1080, SD=486
        float lAspect = 0;
        bool lIsInterlaced = false;
        UtyGLNvSdi.GetSizeFromVideoFormat(options.videoFormat, ref texWidth, ref texHeight, ref lAspect, ref lIsInterlaced);

        // Check the count of cameras. The plugin support until two cameras
        if (outputTexture.Length > 4)
        {
            UnityEngine.Debug.LogWarning("GLNvSdi plugin does not support more than four cameras.");
        }

        // Verify the amount of render textures needed
        int lTexCount = 0;
        if (options.outputDual)
            lTexCount = 2;
        else
            lTexCount = 1;

        if (lIsInterlaced)
            lTexCount *= 2;


        // If dual progressive output, change the order of cameras. So, the first two are active
        if (options.outputDual && !lIsInterlaced)     // dual output progressive
        {
            RenderTexture tmp = outputTexture[1];
            outputTexture[1] = outputTexture[2];
            outputTexture[2] = tmp;
        }


        // Setting the cameras and directing the render to texture
        for (int i = 0; i < lTexCount; ++i)
        {
            if (outputTexture[i] != null)
            {
                //
                // Checking texture sizes
                //
                if (texWidth != outputTexture[i].width || texHeight != outputTexture[i].height)
                {
                    UnityEngine.Debug.LogError("The size of RenderTexture does not match with sdi video options");
                    return false;
                }

                //
                // Check if the texture has been created
                //
                if (!outputTexture[i].IsCreated())
                    if (!outputTexture[i].Create())
                        UnityEngine.Debug.LogError("Could not create Output Sdi RenderTexture");

                //
                // Sending texture id to plugin
                //
#if true        // render with video
                UtyGLNvSdi.SdiOutputSetTexturePtr(i, outputTexture[i].GetNativeTexturePtr(), outputTexture[i].width, outputTexture[i].height);
#else           // input video pasthrough
                UtyGLNvSdi.SdiOutputSetTexturePtr(i, inputTexture[i].GetNativeTexturePtr(), inputTexture[i].width, inputTexture[i].height);
#endif

            }
            else
            {
                UnityEngine.Debug.LogError("Sdi Output RenderTexture is null");
                return false;
            }

        }


        // If the options to invert fields is marked, we have to invert 
        // the textures id already sent to plugin
        if (options.outputInvertFields)
        {
            UtyGLNvSdi.SdiOutputInvertFields(options.outputInvertFields);
        }

        return true;
    }





    void SetupInputTextures()
    {
        for (int i = 0; i < inputTexture.Length; ++i)
        {
            if (inputTexture[i] == null)
                continue;

            if (!inputTexture[i].IsCreated())
            {
                if (!inputTexture[i].Create())
                    Debug.LogError("Could not create sdi input texture");
            }

            UtyGLNvSdi.SdiInputSetTexturePtr(i, inputTexture[i].GetNativeTexturePtr(), inputTexture[i].width, inputTexture[i].height);
        }
    }



}
