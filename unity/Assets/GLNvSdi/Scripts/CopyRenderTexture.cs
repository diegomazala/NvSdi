using UnityEngine;
using System.Collections;

[RequireComponent(typeof(Camera))]
public class CopyRenderTexture : MonoBehaviour 
{
    public RenderTexture srcRenderTexture;
    public RenderTexture dstRenderTexture;

    public bool copy = false;

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.C))
            copy = !copy;

    }

    void OnRenderImage(RenderTexture src, RenderTexture dst)
    {
        if (copy)
        {
            RenderTexture active_rt = RenderTexture.active;
            RenderTexture.active = dstRenderTexture;
            Graphics.Blit(srcRenderTexture, dstRenderTexture);
            RenderTexture.active = active_rt;
        }
        Graphics.Blit(src, dst);
    }
}
