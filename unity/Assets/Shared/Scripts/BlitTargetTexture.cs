using UnityEngine;
using System;

[ExecuteInEditMode]
[RequireComponent(typeof(Camera))]
[AddComponentMenu("Image Effects/Blit Target Texture")]
public class BlitTargetTexture : MonoBehaviour 
{
    public Camera TargetCamera = null;

    void OnRenderImage(RenderTexture source, RenderTexture destination)
    {
        if (TargetCamera != null && TargetCamera.targetTexture != null)
        {
            Graphics.Blit(TargetCamera.targetTexture, destination);
        }
        RenderTexture.active = destination;
    }
      

}
