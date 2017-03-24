using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

[AddComponentMenu("Diem/Video IO SDI - GLNvSdiIO")]
public class GLNvSdiUI : MonoBehaviour
{
    public UtyGLNvSdi utySdi;
    public GLNvSdiOptions options = null;

    public UnityEngine.UI.Text videoFormat;
    public UnityEngine.UI.Text syncSource;
    public UnityEngine.UI.Text ringBufferIn;
    public UnityEngine.UI.Text ringBufferOut;
    public UnityEngine.UI.Text hSync;
    public UnityEngine.UI.Text vSync;
    public UnityEngine.UI.Text droppedFramesIn;
    public UnityEngine.UI.Text droppedFramesOut;
    public UnityEngine.UI.Text gviTime;
    public UnityEngine.UI.InputField delay;



    private int dropFramesIn = 0;
    private int dropFramesOut = 0;

    void Start()
    {
        if (utySdi == null)
            utySdi = FindObjectOfType<UtyGLNvSdi>();

        if (utySdi == null)
        {
            Debug.LogError("Could not find sdi object. Disabling sdi UI");
            enabled = false;
            return;
        }

        options = utySdi.options;

        OnReloadUI();

        Invoke("OnResetDropFramesIn", 3.0f);
        Invoke("OnResetDropFramesOut", 3.0f);
        Invoke("OnReloadUI", 3.5f);
    }

    void Update()
    {
        int drop_in = UtyGLNvSdi.SdiInputDroppedFramesCount();
        int drop_out = UtyGLNvSdi.SdiOutputDuplicatedFramesCount();

        if (dropFramesIn != drop_in || dropFramesOut != drop_out)
        {
            dropFramesIn = drop_in;
            droppedFramesIn.text = dropFramesIn.ToString();

            dropFramesOut = drop_out;
            droppedFramesOut.text = dropFramesOut.ToString();

            gviTime.text = UtyGLNvSdi.SdiInputGviTime().ToString("0.0000");
        }
    }


    public void OnResetDropFramesIn()
    {
        UtyGLNvSdi.SdiInputResetDroppedFramesCount();
    }


    public void OnResetDropFramesOut()
    {
        UtyGLNvSdi.SdiOutputResetDuplicatedFramesCount();
    }

    public void OnDelayEdit()
    {
        options.outputDelay = float.Parse(this.delay.text);
        UtyGLNvSdi.SdiOutputSetDelay(options.outputDelay);
    }

    public void OnReloadUI()
    {
        videoFormat.text = options.videoFormat.ToString();
        syncSource.text = options.syncSource.ToString();
        ringBufferIn.text = options.inputRingBufferSize.ToString();
        ringBufferOut.text = options.outputRingBufferSize.ToString();
        hSync.text = options.outputHorizontalDelay.ToString();
        vSync.text = options.outputVerticalDelay.ToString();
        droppedFramesIn.text = UtyGLNvSdi.SdiInputDroppedFramesCount().ToString();
        droppedFramesOut.text = UtyGLNvSdi.SdiOutputDuplicatedFramesCount().ToString();
        gviTime.text = UtyGLNvSdi.SdiInputGviTime().ToString("0.0000");
        delay.text = options.outputDelay.ToString("0.0");
    }

}
