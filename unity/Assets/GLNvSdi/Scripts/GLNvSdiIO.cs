using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

[AddComponentMenu("Diem/Video IO SDI - GLNvSdiIO")]
public class GLNvSdiIO : MonoBehaviour
{
    public const int MAX_COUNT = 8;

    public bool captureFields = true;

    public bool useVideoAsBackground = true;

    public GLNvSdiOptions options;

    public Material[] sdiMaterials = { null, null, null, null, null, null, null, null };
    public Material[] sdiCompositeMaterial = { null, null, null, null };

    private RenderTexture[] sdiTexture = { null, null, null, null, null, null, null, null };
        
    public Camera[] m_Camera = { null, null, null, null };

    private int[] timeCodeData;
    private GCHandle timeCodeHandle;



    public bool showGUI = false;


    void OnEnable()
    {
        if (!SystemInfo.graphicsDeviceVersion.Contains("OpenGL"))
        {
            this.enabled = false;
            return;
        }

        timeCodeData = new int[8] { 0, 0, 0, 0, 0, 0, 0, 0 };
        timeCodeHandle = GCHandle.Alloc(timeCodeData, GCHandleType.Pinned);
    }



    IEnumerator Start()
    {
        UtyGLNvSdi.SdiSetupLogFile();
        yield return StartCoroutine(SdiIOCoroutine());
    }

    void OnDisable()
    {
        timeCodeHandle.Free();

        StopCoroutine(SdiIOCoroutine());

        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.Shutdown);
        
        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.Shutdown);

        DestroyTextures();
    }


    private IEnumerator SdiIOCoroutine()
    {
        yield return new WaitForEndOfFrame();

        //
        // Input
        //
        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.Initialize);
        yield return new WaitForEndOfFrame();

        if (captureFields)
            CreateSdiInputTextures(8, UtyGLNvSdi.SdiInputWidth(), UtyGLNvSdi.SdiInputHeight() / 2);
        else
            CreateSdiInputTextures(4, UtyGLNvSdi.SdiInputWidth(), UtyGLNvSdi.SdiInputHeight());

        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.Setup);
        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.StartCapture);
        yield return new WaitForEndOfFrame();


        //
        // Output
        //
        UtyGLNvSdi.SdiOutputSetGlobalOptions();
        UtyGLNvSdi.SdiOutputSetVideoFormat(
            options.videoFormat,
            options.syncSource,
            options.hDelay,
            options.vDelay,
            options.dualOutput,
            options.flipQueueLength);

        yield return new WaitForEndOfFrame();

        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.Initialize);
        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.Setup);


#if passthru
        UtyGLNvSdi.SdiOutputSetTexture(0, sdiTexture[0].GetNativeTextureID());
        UtyGLNvSdi.SdiOutputSetTexture(1, sdiTexture[1].GetNativeTextureID());
#else
        int texWidth = 1920;
        int texHeight = 1080;
        bool interlaced = true;
        float aspect = 1.0f;

        UtyGLNvSdi.GetSizeFromVideoFormat(options.videoFormat, ref texWidth, ref texHeight, ref aspect, ref interlaced);
        if (!SetupOutputTextures(texWidth, texHeight, aspect, interlaced, options.dualOutput))
        {
            UnityEngine.Debug.LogError("GLNvSdi_Plugin could not setup sdi textures for input/output");
        }
#endif   

        
        while (true)
        {
            // Wait until all frame rendering is done
            yield return new WaitForEndOfFrame();

            // Get status of the capture (GL_SUCCESS_NV, GL_FAILURE_NV, GL_PARTIAL_SUCCESS_NV)
            //Debug.Log(UtyGLNvSdi.SdiInputCaptureStatus().ToString());

            // Capture frame from device
            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiInputRenderEventFunc(), (int)SdiRenderEvent.CaptureFrame);

            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.PresentFrame);
        }

    }


    bool SetupOutputTextures(int texWidth, int texHeight, float aspect, bool interlaced, bool dualOutput)
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
        if (options.dualOutput)
            lTexCount = 2;
        else
            lTexCount = 1;

        if (interlaced)
            lTexCount *= 2;

        // If dual progressive output, change the order of cameras. So, the first two are active
        if (options.dualOutput && !interlaced)     // dual output progressive
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

                    if (useVideoAsBackground)
                    {
                        if (captureFields)
                            sdiTex.backgroundTex = sdiTexture[i];
                        else
                            sdiTex.backgroundTex = sdiTexture[i / 2];
                    }

                    sdiTex.material = sdiCompositeMaterial[i];

                    UtyGLNvSdi.SdiOutputSetTexturePtr(i, sdiTex.sdiRenderTarget.GetNativeTexturePtr(), sdiTex.sdiRenderTarget.width, sdiTex.sdiRenderTarget.height);
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
            if (captureFields && texHeight == 243 && i % 2 == 0)
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
            int multiplier = captureFields ? 2 : 1;
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



    void Update()
    {
        if (Input.GetKeyDown(KeyCode.V))
            showGUI = !showGUI;
    }

    private Rect windowRect = new Rect(10, 10, 250, 200);
    private int windowId = 0;
    void OnGUI()
    {
        if (!showGUI)
            return;

        if (windowId == 0)
            Random.Range(1, 9999);
        windowRect = GUILayout.Window(windowId, windowRect, BuildWindow, "Sdi [Key 'V']");
    }


    void BuildWindow(int windowId)
    {
        GUILayout.BeginHorizontal("box");
        {
            GUILayout.Label("Video Format: ");
            GUILayout.Label(this.options.videoFormat.ToString(), "box");
        }
        GUILayout.EndHorizontal();

        GUILayout.BeginHorizontal("box");
        {
            GUILayout.Label("Sync Source: ");
            GUILayout.Label(this.options.syncSource.ToString(), "box");
        }
        GUILayout.EndHorizontal();

        GUILayout.BeginHorizontal("box");
        {
            GUILayout.Label("Input Ring Buffer: ");
            GUILayout.Label(this.options.inputRingBufferSize.ToString(), "box");
        }
        GUILayout.EndHorizontal();

        GUILayout.BeginHorizontal("box");
        {
            GUILayout.Label("Gvi Time: ");
            GUILayout.Label(UtyGLNvSdi.SdiInputGviTime().ToString(), "box");
        }
        GUILayout.EndHorizontal();

        GUILayout.BeginHorizontal("box");
        {
            if (GUILayout.Button("Reset"))
                UtyGLNvSdi.SdiInputResetDroppedFramesCount();
            GUILayout.Label("Dropped Frames Count: ");
            GUILayout.Label(UtyGLNvSdi.SdiInputDroppedFramesCount().ToString(), "box");
        }
        GUILayout.EndHorizontal();

        GUILayout.BeginHorizontal("box");
        {
            GUILayout.Label("Output Flip Queue Length: ");
            GUILayout.Label(this.options.flipQueueLength.ToString(), "box");
        }
        GUILayout.EndHorizontal();

        GUILayout.BeginHorizontal("box");
        {
            if (GUILayout.Button("Reset"))
                UtyGLNvSdi.SdiOutputResetDuplicatedFramesCount();
            GUILayout.Label("Duplicated Frames Count: ");
            GUILayout.Label(UtyGLNvSdi.SdiOutputDuplicatedFramesCount().ToString(), "box");
        }
        GUILayout.EndHorizontal();

        GUILayout.BeginHorizontal("box");
        {
            GUILayout.Label("Dual Output: ");
            GUILayout.Label(this.options.dualOutput.ToString(), "box");
        }
        GUILayout.EndHorizontal();

        GUILayout.BeginHorizontal("box");
        {
            GUILayout.Label("Horizontal Delay: ");
            GUILayout.Label(this.options.hDelay.ToString(), "box");
        }
        GUILayout.EndHorizontal();

        GUILayout.BeginHorizontal("box");
        {
            GUILayout.Label("Vertical Delay: ");
            GUILayout.Label(this.options.vDelay.ToString(), "box");
        }
        GUILayout.EndHorizontal();
    }
}