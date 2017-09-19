using UnityEngine;
using System;

[ExecuteInEditMode]
[RequireComponent(typeof(Camera))]
[AddComponentMenu("Image Effects/Blit Target Texture")]
public class BlitRenderTexture : MonoBehaviour 
{
    public RenderTexture renderTexture = null;

    void OnRenderImage(RenderTexture source, RenderTexture destination)
    {
        if (renderTexture != null)
        {
            Graphics.Blit(renderTexture, destination);
        }
        RenderTexture.active = destination;
    }
}
