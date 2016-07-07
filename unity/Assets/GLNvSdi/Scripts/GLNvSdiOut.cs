/*
- Para que o script funcione corretamente, n�o esquec�a de colocar as DLLs na pasta
   onde est� o execut�vel: GLNvSdi.dll 
- Al�m disso, n�o esque�a de executar o programa usando Opengl como api gr�fica: game.exe -force-opengl-core
- Foi testado e est� funcionando em HD1, HD Full e SD.
- Caso tenha problemas com a qualidade da imagem, verifique o tamanho da RenderTexture. Elas s�o dimensionadas
   de acordo com a resolu��o escolhida: SD=720x486, HD=1920,1080. 
   Diego Mazala 
   Criado em 19 de julho de 2010
   Atualizado em 24 de março de 2016
*/
using UnityEngine;
using System;
using System.IO;
using System.Collections;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Collections.Generic;
using System.Xml.Serialization;


[AddComponentMenu("Diem/Video Out SDI - GLNvSdiOut")]
public class GLNvSdiOut : MonoBehaviour
{
    public GLNvSdiOptions options;

    public Camera[] m_Camera = { null, null, null, null };

    private int m_TexWidth = 1920;	// HD=1920, SD=720
    private int m_TexHeight = 1080;	// HD=1080, SD=486

    private IEnumerator OutputCoroutine = null;


    void OnEnable()
    {
        if (SystemInfo.graphicsDeviceType != UnityEngine.Rendering.GraphicsDeviceType.OpenGLCore || !UtyGLNvSdi.SdiOutputInitialize())
        {
            this.enabled = false;
            return;
        }

        OutputCoroutine = SdiOutputCoroutine();
    }



    IEnumerator Start()
    {
#if !UNITY_EDITOR
        if (!GLNvSdiOptions.Load(UtyGLNvSdi.ConfigFileName, ref options))
            GLNvSdiOptions.Save(UtyGLNvSdi.ConfigFileName, options);
#endif


        if (options.logToFile)
            UtyGLNvSdi.SdiSetupLogFile();

        yield return StartCoroutine(OutputCoroutine);

    }

    void OnDisable()
    {
        StopCoroutine(OutputCoroutine);

        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.Shutdown);
    }


    private IEnumerator SdiOutputCoroutine()
    {
        UtyGLNvSdi.SdiOutputSetGlobalOptions();
        UtyGLNvSdi.SdiOutputSetVideoFormat(
            options.videoFormat,
            options.syncSource,
            options.outputHorizontalDelay,
            options.outputVerticalDelay,
            options.outputDual,
            options.outputFlipQueueLength);

        yield return new WaitForEndOfFrame();

        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.Initialize);
        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.Setup);

        if (!SetupOutputTextures())
        {
            UnityEngine.Debug.LogError("GLNvSdi_Plugin could not setup sdi textures for output");
        }

        while (true)
        {
            yield return new WaitForEndOfFrame();
            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.PresentFrame);
        }
    }




    bool SetupOutputTextures()
    {
        float lAspect = 0;
        bool lIsInterlaced = false;
        UtyGLNvSdi.GetSizeFromVideoFormat(options.videoFormat, ref m_TexWidth, ref m_TexHeight, ref lAspect, ref lIsInterlaced);

        // Check the count of cameras. The plugin support until two cameras
        if (m_Camera.Length > 4)
        {
            UnityEngine.Debug.LogWarning("GLNvSdi plugin does not support more than four cameras.");
        }

        // Disable all cameras
        for (int i = 0; i < m_Camera.Length; ++i)
            m_Camera[i].enabled = false;

        // Verify the amount of render textures needed
        int lTexCount = 0;
        if (options.outputDual)
            lTexCount = 2;
        else
            lTexCount = 1;

        if (lIsInterlaced)
            lTexCount *= 2;

        // If dual progressive output, change the order of cameras. So, the first two are active
        if (options.outputDual && !lIsInterlaced)     // dual output progressive
        {
            Camera tmp = m_Camera[1];
            m_Camera[1] = m_Camera[2];
            m_Camera[2] = tmp;

            GLNvSdiRenderTexture rt1 = m_Camera[1].GetComponent<GLNvSdiRenderTexture>();
            GLNvSdiRenderTexture rt2 = m_Camera[2].GetComponent<GLNvSdiRenderTexture>();

            if (rt1 != null && rt2 != null)
            {
                int tmp_index = rt1.sdiTextureIndex;
                rt1.sdiTextureIndex = rt2.sdiTextureIndex;
                rt2.sdiTextureIndex = tmp_index;
            }
            else
            {
                UnityEngine.Debug.LogError("Error: Could not find GLNvSdiRenderTexture when switching camera parameters");
                return false;
            }
        }


        // Setting the cameras and directing the render to texture
        for (int i = 0; i < lTexCount; ++i)
        {
            if (m_Camera[i] != null)
            {
                m_Camera[i].aspect = lAspect;
                m_Camera[i].enabled = true;


                GLNvSdiRenderTexture sdiTex = m_Camera[i].GetComponent<GLNvSdiRenderTexture>();
                if (sdiTex != null)
                {
                    sdiTex.CreateSdiTexture(m_TexWidth, m_TexHeight, 32, false, "RenderTex_" + m_Camera[i].name);

                    if (sdiTex.sdiRenderTarget == null)
                    {
                        UnityEngine.Debug.LogError("Could not create SdiRenderTarget for GLNvSdiRenderTexture in " + m_Camera[i].name);
                        return false;
                    }

                    UtyGLNvSdi.SdiOutputSetTexturePtr(i, sdiTex.sdiRenderTarget.GetNativeTexturePtr(), sdiTex.sdiRenderTarget.width, sdiTex.sdiRenderTarget.height);
                }
                else
                {
                    UnityEngine.Debug.LogError("Could not find GLNvSdiRenderTexture in " + m_Camera[i].name);
                    return false;
                }
            }
            else
            {
                UnityEngine.Debug.LogError(m_Camera[i].name + " is null");
                return false;
            }
        }

        // If the options to invert fields is marked, we have to invert 
        // the textures id already sent to plugin
        if (options.outputInvertFields)
        {
            UtyGLNvSdi.SdiOutputInvertFields(options.outputInvertFields);
        }

        return true;
    }




}

