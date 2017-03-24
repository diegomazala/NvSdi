using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

[AddComponentMenu("Diem/Video IO SDI - GLNvSdiIO")]
public class GLNvSdiIO : UtyGLNvSdi
{
    public Material[] sdiMaterials = { null, null, null, null, null, null, null, null };
    public Material[] sdiCompositeMaterial = { null, null, null, null };

    public RenderTexture[] sdiTexture = { null, null, null, null, null, null, null, null };
        
    public Camera[] m_Camera = { null, null, null, null };

    private int[] timeCodeData;
    private GCHandle timeCodeHandle;


    public void ShowGUI()
    {
        showGUI = !showGUI;
    }

    public bool showGUI = false;


    private IEnumerator IOCoroutine = null;
    public bool sdiEnabled = false;

    void OnEnable()
    {
        if (!SystemInfo.graphicsDeviceVersion.StartsWith("OpenGL"))
        {
            enabled = false;
            return;
        }

        sdiEnabled = false;
        IOCoroutine = SdiIOCoroutine();

        timeCodeData = new int[8] { 0, 0, 0, 0, 0, 0, 0, 0 };
        timeCodeHandle = GCHandle.Alloc(timeCodeData, GCHandleType.Pinned);

#if !UNITY_EDITOR
        if (!GLNvSdiOptions.Load(UtyGLNvSdi.ConfigFileName, ref options))
            GLNvSdiOptions.Save(UtyGLNvSdi.ConfigFileName, options);
#endif

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

        DestroyTextures();
        timeCodeHandle.Free();
    }

    
    void Update()
    {
        UpdateFrame();
    }

    public void UpdateFrame()
    {
        if (sdiEnabled)
        {
            // Capture frame from device
            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.CaptureFrame);
            // Present frame
            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.PresentFrame);
        }
    }

    private IEnumerator SdiIOCoroutine()
    {
        yield return new WaitForEndOfFrame();


        //
        // Input/Ouput Initialization
        //
        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.Initialize);
        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.Initialize);
        yield return new WaitForEndOfFrame();


        if (UtyGLNvSdi.SdiError() == 0)
        {
            UtyGLNvSdi.SdiInputSetGlobalOptions(options.inputRingBufferSize, options.inputCaptureFields);

            //
            // Input Setup
            //
            if (options.inputCaptureFields)
                CreateSdiInputTextures(8, UtyGLNvSdi.SdiInputWidth(), UtyGLNvSdi.SdiInputHeight() / 2);
            else
                CreateSdiInputTextures(4, UtyGLNvSdi.SdiInputWidth(), UtyGLNvSdi.SdiInputHeight());

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

            yield return new WaitForEndOfFrame();

        
            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.Setup);


            int texWidth = 1920;
            int texHeight = 1080;
            bool interlaced = true;
            float aspect = 1.0f;

            UtyGLNvSdi.GetSizeFromVideoFormat(options.videoFormat, ref texWidth, ref texHeight, ref aspect, ref interlaced);
            if (!SetupOutputTextures(texWidth, texHeight, aspect, interlaced, options.outputDual))
            {
                UnityEngine.Debug.LogError("GLNvSdi_Plugin could not setup sdi textures for input/output");
            }

            sdiEnabled = true;

            //while (true)
            //{
            //    // Wait until all frame rendering is done
            //    yield return new WaitForEndOfFrame();

            //    // Capture frame from device
            //    GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.CaptureFrame);
            //    // Present frame
            //    GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.PresentFrame);

            //}
        }

    }

   


    bool SetupOutputTextures(int texWidth, int texHeight, float aspect, bool interlaced, bool outputDual)
    {
        // Check the count of cameras. The plugin support until two cameras
        if (m_Camera.Length > 4)
        {
            UnityEngine.Debug.LogWarning("GLNvSdi plugin does not support more than four cameras.");
        }

        // Disable all cameras
        for (int i = 0; i < m_Camera.Length; ++i)
            m_Camera[i].enabled = false;

        // Verify the amount of render textures needed
        int lTexCount = 0;
        if (options.outputDual)
            lTexCount = 2;
        else
            lTexCount = 1;

        if (interlaced)
            lTexCount *= 2;

        // If dual progressive output, change the order of cameras. So, the first two are active
        if (options.outputDual && !interlaced)     // dual output progressive
        {
            Camera tmp = m_Camera[1];
            m_Camera[1] = m_Camera[2];
            m_Camera[2] = tmp;

            GLNvSdiRenderTexture rt1 = m_Camera[1].GetComponent<GLNvSdiRenderTexture>();
            GLNvSdiRenderTexture rt2 = m_Camera[2].GetComponent<GLNvSdiRenderTexture>();

            if (rt1 != null && rt2 != null)
            {
                int tmp_index = rt1.sdiTextureIndex;
                rt1.sdiTextureIndex = rt2.sdiTextureIndex;
                rt2.sdiTextureIndex = tmp_index;

                rt1.material = sdiCompositeMaterial[0];
                rt2.material = sdiCompositeMaterial[1];
            }
            else
            {
                UnityEngine.Debug.LogError("Error: Could not find GLNvSdiRenderTexture when switching camera parameters");
                return false;
            }
        }


        // Setting the cameras and directing the render to texture
        for (int i = 0; i < lTexCount; ++i)
        {
            if (m_Camera[i] != null)
            {
                m_Camera[i].aspect = aspect;
                m_Camera[i].enabled = true;


                GLNvSdiRenderTexture sdiTex = m_Camera[i].GetComponent<GLNvSdiRenderTexture>();
                if (sdiTex != null)
                {
                    sdiTex.CreateSdiTexture(texWidth, texHeight, 32, false, "SdiRenderTex_" + m_Camera[i].name);

                    if (sdiTex.sdiRenderTarget == null)
                    {
                        UnityEngine.Debug.LogError("Could not create SdiRenderTarget for GLNvSdiRenderTexture in " + m_Camera[i].name);
                        return false;
                    }

                    if (options.useInputVideoAsBackground)
                    {
                        if (options.inputCaptureFields)
                            sdiTex.backgroundTex = sdiTexture[i];
                        else
                            sdiTex.backgroundTex = sdiTexture[i / 2];
                    }

                    sdiTex.material = sdiCompositeMaterial[i];

#if true   // render with video
                    UtyGLNvSdi.SdiOutputSetTexturePtr(i, sdiTex.sdiRenderTarget.GetNativeTexturePtr(), sdiTex.sdiRenderTarget.width, sdiTex.sdiRenderTarget.height);
#else       // input video pasthrough
                    UtyGLNvSdi.SdiOutputSetTexturePtr(i, sdiTexture[i].GetNativeTexturePtr(), sdiTexture[i].width, sdiTexture[i].height);
#endif

                }
            }
            else
            {
                UnityEngine.Debug.LogError(m_Camera[i].name + " is null");
                return false;
            }
        }

        return true;
    }



    void CreateSdiInputTextures(int count, int texWidth, int texHeight)
    {
        // Remember: if input videoFormat == SdiVideoFormat.SD_487I_59_94, that is, texHeight == 243
        // the upper field has one line more than the lower field. 
        // UpperField = 244; LowerField = 243

        for (int i = 0; i < count; ++i)
        {
            if (options.inputCaptureFields && texHeight == 243 && i % 2 == 0)
                sdiTexture[i] = new RenderTexture(texWidth, texHeight + 1, 32, RenderTextureFormat.ARGB32);
            else
                sdiTexture[i] = new RenderTexture(texWidth, texHeight, 32, RenderTextureFormat.ARGB32);
            sdiTexture[i].name = name;
            sdiTexture[i].isPowerOfTwo = false;
            sdiTexture[i].useMipMap = false;
            sdiTexture[i].filterMode = FilterMode.Trilinear;
            sdiTexture[i].wrapMode = TextureWrapMode.Clamp;
            sdiTexture[i].Create();

            UtyGLNvSdi.SdiInputSetTexturePtr(i, sdiTexture[i].GetNativeTexturePtr(), sdiTexture[i].width, sdiTexture[i].height);

            // attach to texture only the existent video inputs
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
