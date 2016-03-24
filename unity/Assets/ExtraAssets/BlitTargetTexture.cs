using UnityEngine;
using System;

[ExecuteInEditMode]
[RequireComponent(typeof(Camera))]
[AddComponentMenu("Image Effects/Blit Target Texture")]
public class BlitTargetTexture : MonoBehaviour 
{
    public Camera m_TargetCamera = null;

    public float targetAspect = 1;


    public Camera TargetCamera
    {
        get { return m_TargetCamera; }
        set { m_TargetCamera = value; }
    }


    void CorrectAspectRatio(float target_aspect)
    {
        // set the desired aspect ratio (the values in this example are
        // hard-coded for 16:9, but you could make them into public
        // variables instead so you can set them at design time)
        //float targetaspect = 16.0f / 9.0f;

        // determine the game window's current aspect ratio
        float windowaspect = (float)Screen.width / (float)Screen.height;

        // current viewport height should be scaled by this amount
        float scaleheight = windowaspect / target_aspect;

        // obtain camera component so we can modify its viewport
        //Camera camera = GetComponent<Camera>();

        // if scaled height is less than current height, add letterbox
        if (scaleheight < 1.0f)
        {
            Rect rect = GetComponent<Camera>().rect;

            rect.width = 1.0f;
            rect.height = scaleheight;
            rect.x = 0;
            rect.y = (1.0f - scaleheight) / 2.0f;

            GetComponent<Camera>().rect = rect;
        }
        else // add pillarbox
        {
            float scalewidth = 1.0f / scaleheight;

            Rect rect = GetComponent<Camera>().rect;

            rect.width = scalewidth;
            rect.height = 1.0f;
            rect.x = (1.0f - scalewidth) / 2.0f;
            rect.y = 0;

            GetComponent<Camera>().rect = rect;
        }
    }


    
    void OnRenderImage(RenderTexture source, RenderTexture destination)
    {
        //CorrectAspectRatio(targetAspect);

        if (m_TargetCamera != null && m_TargetCamera.targetTexture != null)
        {
            Graphics.Blit(m_TargetCamera.targetTexture, destination);
        }
        RenderTexture.active = destination;
    }
      

}
