using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

[AddComponentMenu("Diem/Video IO SDI - GLNvSdiIO")]
public class GLNvDvpUI : MonoBehaviour
{
    public DvpIO dvp;

    public UnityEngine.UI.Text videoFormat;
    public UnityEngine.UI.Text syncSource;
    public UnityEngine.UI.Text ringBufferIn;
    public UnityEngine.UI.Text ringBufferOut;
    public UnityEngine.UI.Text hSync;
    public UnityEngine.UI.Text vSync;
    public UnityEngine.UI.Text droppedFramesIn;
    public UnityEngine.UI.Text droppedFramesOut;
    public UnityEngine.UI.Text captureTime;

    void Start()
    {
        dvp = FindObjectOfType<DvpIO>();
        if (dvp == null)
        {
            Debug.LogError("Could not find dvp object. Disabling dvp UI");
            enabled = false;
            return;
        }

        Invoke("OnReloadUI", 2);
    }

    void Update()
    {
        droppedFramesIn.text = DvpIO.Plugin.DvpInputDroppedFrames().ToString();
        droppedFramesOut.text = DvpIO.Plugin.DvpDroppedFrames().ToString();
        captureTime.text = DvpIO.Plugin.DvpInputCaptureElapsedTime().ToString("0.0000");
    }


    public void OnResetDropFramesIn()
    {
    }


    public void OnResetDropFramesOut()
    {
    }


    public void OnReloadUI()
    {
        Debug.Log("reload");
        videoFormat.text = dvp.options.videoFormat.ToString();
        syncSource.text = dvp.options.syncSource.ToString();
        ringBufferIn.text = dvp.options.inputRingBufferSize.ToString();
        ringBufferOut.text = dvp.options.outputRingBufferSize.ToString();
        hSync.text = dvp.options.outputHorizontalDelay.ToString();
        vSync.text = dvp.options.outputVerticalDelay.ToString();
        droppedFramesIn.text = DvpIO.Plugin.DvpInputDroppedFrames().ToString();
        droppedFramesOut.text = DvpIO.Plugin.DvpDroppedFrames().ToString();
        captureTime.text = DvpIO.Plugin.DvpInputCaptureElapsedTime().ToString("0.0000");
    }

}
