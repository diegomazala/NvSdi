using UnityEngine;
using UnityEngine.Assertions;
using System.Collections;


[RequireComponent(typeof(Camera))]
public class BackgroundTexture : MonoBehaviour 
{
    public RenderTexture backgroundTex = null;
    private Material material;
    public float imageScale = 1.0f;

    void Awake()
    {
        Shader shader_comp = Shader.Find("GLNvSdi/SdiComposite");
        Assert.IsNotNull(shader_comp);
        material = new Material(shader_comp);
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
