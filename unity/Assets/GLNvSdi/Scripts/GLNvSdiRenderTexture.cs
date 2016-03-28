using UnityEngine;
using System.Collections;


[RequireComponent(typeof(Camera))]
public class GLNvSdiRenderTexture : MonoBehaviour 
{
    public int sdiTextureIndex = 0;

    [HideInInspector]
    public  RenderTexture sdiRenderTarget = null;

    [HideInInspector]
    public RenderTexture backgroundTex = null;

    private Material m_Material;

    public float imageScale = 1.0f;

    public void CreateSdiTexture(int w, int h, int depth, bool is_power_of_two, string name)
    {
        sdiRenderTarget = new RenderTexture(w, h, depth, RenderTextureFormat.ARGB32);
        sdiRenderTarget.name = name;
        sdiRenderTarget.isPowerOfTwo = is_power_of_two;
        sdiRenderTarget.useMipMap = false;
        sdiRenderTarget.filterMode = FilterMode.Trilinear;
        sdiRenderTarget.wrapMode = TextureWrapMode.Clamp;
        sdiRenderTarget.antiAliasing = 1;
        sdiRenderTarget.anisoLevel = 0; // RenderTexture wit depth must have an Aniso Level of 0
        if (!sdiRenderTarget.Create())
            Debug.LogError("Could not create RenderTexture");

        this.GetComponent<Camera>().targetTexture = sdiRenderTarget;
    }
    

    void OnDisable()
    {
        if (sdiRenderTarget != null)
        {
            if (RenderTexture.active == sdiRenderTarget)
                RenderTexture.active = null;

            this.GetComponent<Camera>().targetTexture = null;
            if (sdiRenderTarget != null)
            {
                DestroyImmediate(sdiRenderTarget);
                sdiRenderTarget = null;
            }
        }
    }


    void OnRenderImage(RenderTexture source, RenderTexture destination)
    {
		if (backgroundTex != null && material != null)
		{
			material.SetTexture("_BgTex", backgroundTex);
            material.SetFloat("_Scale", imageScale);
            //Graphics.Blit(source, sdiRenderTarget, material);
            //Graphics.Blit(sdiRenderTarget, destination);
            Graphics.Blit(source, destination, material);
		}
		else
		{
            RenderTexture.active = destination;
            //Graphics.Blit(source, sdiRenderTarget);
			Graphics.Blit(source, destination);
            
		}
    }



    
    public Material material
    {
        get { return m_Material; }
        set { m_Material = value; }
    }


}
