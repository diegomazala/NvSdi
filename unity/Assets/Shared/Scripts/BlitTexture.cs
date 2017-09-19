using UnityEngine;
using System;

[ExecuteInEditMode]
[RequireComponent(typeof(Camera))]
[AddComponentMenu("Image Effects/Blit Texture")]
public class BlitTexture : MonoBehaviour 
{
    public Shader shader;
    public Texture texture = null;
    public float fixAspectRatio = 1;
    public float gammaCorrection = 2.2f;


    void OnRenderImage(RenderTexture source, RenderTexture destination)
    {
        material.SetFloat("_GammaCorrection", gammaCorrection);
        material.SetFloat("_FixAspect", fixAspectRatio);
        material.SetTexture("_MainTex", texture);
        Graphics.Blit(texture, destination, material);

        RenderTexture.active = destination;
    }

    void OnEnable()
    {
        // Try to find the shader
        if (!shader)
            shader = Shader.Find("Diem/BlitTexture");


        if (!texture)
        {
            Debug.LogError("Missing reference to a texture.");
            enabled = false;
            return;
        }

        if (!shader)
            enabled = false;
    }

    protected virtual void OnDisable()
    {
        if (m_Material)
        {
            DestroyImmediate(m_Material);
        }
    }

    private Material m_Material;
    protected Material material
    {
        get
        {
            if (m_Material == null)
            {
                m_Material = new Material(shader);
                m_Material.hideFlags = HideFlags.HideAndDontSave;
            }
            return m_Material;
        }
    }
}
