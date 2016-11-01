using UnityEngine;
using UnityEngine.Assertions;
using System.Collections;


[RequireComponent(typeof(Camera))]
public class CompositeTextures : MonoBehaviour 
{
    private GLNvDvp dvp = null;
    
    public RenderTexture backgroundTex = null;
    public RenderTexture foregroundTex = null;
    public RenderTexture foregroundPrevTex = null;
    public RenderTexture[] outputTex = {null, null};
    public Shader composite;
    private Material material;
    public float imageScale = 1.0f;

    void Awake()
    {
        Assert.IsNotNull(composite);
        material = new Material(composite);

        dvp = FindObjectOfType<GLNvDvp>();
        Assert.IsNotNull(dvp);
    }

    void OnEnable()
    {
        if (backgroundTex == null || foregroundTex == null || outputTex[0] == null || outputTex[1] == null || dvp == null)
            enabled = false;

    }

    void OnPreRender()
    {
        Graphics.Blit(foregroundTex, foregroundPrevTex);
    }

    void OnRenderImage(RenderTexture source, RenderTexture destination)
    {
        material.SetTexture("_BgTex", backgroundTex);
        material.SetTexture("_MainTex", foregroundTex);
        material.SetFloat("_Scale", imageScale);

        Graphics.Blit(foregroundTex, outputTex[0], material);
        Graphics.Blit(foregroundPrevTex, outputTex[1], material);
                
        Graphics.Blit(source, destination);
    }

}
