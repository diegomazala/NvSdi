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

    void Start()
    {
        UtyGLNvSdi sdi = FindObjectOfType<UtyGLNvSdi>();
        if (sdi == null)
        {
            Debug.LogError("Could not find sdi object. Disabling sdi UI");
            enabled = false;
            return;
        }

        options = sdi.options;

        OnReloadUI();
    }

    void Update()
    {
        droppedFramesIn.text = UtyGLNvSdi.SdiInputDroppedFramesCount().ToString();
        droppedFramesOut.text = UtyGLNvSdi.SdiOutputDuplicatedFramesCount().ToString();
        gviTime.text = UtyGLNvSdi.SdiInputGviTime().ToString("0.0000");
    }


    public void OnResetDropFramesIn()
    {
        UtyGLNvSdi.SdiInputResetDroppedFramesCount();
    }


    public void OnResetDropFramesOut()
    {
        UtyGLNvSdi.SdiOutputResetDuplicatedFramesCount();
    }


    public void OnReloadUI()
    {
        videoFormat.text = options.videoFormat.ToString();
        syncSource.text = options.syncSource.ToString();
        ringBufferIn.text = options.inputRingBufferSize.ToString();
        ringBufferOut.text = options.outputFlipQueueLength.ToString();
        hSync.text = options.outputHorizontalDelay.ToString();
        vSync.text = options.outputVerticalDelay.ToString();
        droppedFramesIn.text = UtyGLNvSdi.SdiInputDroppedFramesCount().ToString();
        droppedFramesOut.text = UtyGLNvSdi.SdiOutputDuplicatedFramesCount().ToString();
        gviTime.text = UtyGLNvSdi.SdiInputGviTime().ToString("0.0000");
    }

}
