using UnityEngine;
using UnityEngine.Assertions;
using System.Collections;


[RequireComponent(typeof(Camera))]
public class GLNvSdiCompositeFillKey : MonoBehaviour
{
    public Texture fillTexture = null;
    public Texture keyTexture = null;
    public Shader composite;
    private Material material;
    public float imageScale = 1.0f;

    void Awake()
    {
        Assert.IsNotNull(composite);
        material = new Material(composite);
    }

    void OnDestroy()
    {
        Destroy(material);
    }

    void OnRenderImage(RenderTexture source, RenderTexture destination)
    {
        if (fillTexture != null && keyTexture != null)
        {
            material.SetTexture("_FillTex", fillTexture);
            material.SetTexture("_KeyTex", keyTexture);
            material.SetFloat("_Scale", imageScale);
            Graphics.Blit(source, destination, material);
        }
        else
        {
            Graphics.Blit(source, destination);
        }
    }


}
