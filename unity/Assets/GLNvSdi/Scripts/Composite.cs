using UnityEngine;
using UnityEngine.Assertions;
using System.Collections;

[ExecuteInEditMode]
[RequireComponent(typeof(Camera))]
public class Composite : MonoBehaviour 
{
    public RenderTexture backgroundTex = null;
    public RenderTexture foregroundTex = null;
    public Shader composite;
    private Material material;
    public float imageScale = 1.0f;
    public float gammaCorrection = 2.2f;

    void Awake()
    {
        material = new Material(composite);
    }

    void OnEnable()
    {
        if (backgroundTex == null || foregroundTex == null)
            enabled = false;

    }

    void OnRenderImage(RenderTexture source, RenderTexture destination)
    {
        material.SetTexture("_BgTex", backgroundTex);
        material.SetTexture("_MainTex", foregroundTex);
        material.SetFloat("_Scale", imageScale);
        material.SetFloat("_GammaCorrection", gammaCorrection);
        Graphics.Blit(foregroundTex, destination, material);
    }

}

