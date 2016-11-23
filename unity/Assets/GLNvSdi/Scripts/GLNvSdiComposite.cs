using UnityEngine;
using UnityEngine.Assertions;
using System.Collections;


[RequireComponent(typeof(Camera))]
public class GLNvSdiComposite : MonoBehaviour 
{
    public RenderTexture backgroundTex = null;
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
		if (backgroundTex != null)
		{
			material.SetTexture("_BgTex", backgroundTex);
            material.SetFloat("_Scale", imageScale);
            Graphics.Blit(source, destination, material);
		}
		else
		{
            RenderTexture.active = destination;
			Graphics.Blit(source, destination);
		}
    }


}
