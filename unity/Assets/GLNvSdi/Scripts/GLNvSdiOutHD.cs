using UnityEngine;
using System;
using System.IO;
using System.Collections;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Collections.Generic;
using System.Xml.Serialization;


[AddComponentMenu("Diem/Video Out SDI HD - GLNvSdiOutHD")]
public class GLNvSdiOutHD : UtyGLNvSdi
{
    public RenderTexture[] outputTexture = { null, null, null, null };
       

    private IEnumerator OutputCoroutine = null;
    private bool sdiEnabled = false;

    void OnEnable()
    {
        if (!SystemInfo.graphicsDeviceVersion.StartsWith("OpenGL"))
        {
            enabled = false;
            return;
        }

        OutputCoroutine = SdiOutputCoroutine();
        sdiEnabled = false;

#if !UNITY_EDITOR
        if (!GLNvSdiOptions.Load(UtyGLNvSdi.ConfigFileName, ref options))
            GLNvSdiOptions.Save(UtyGLNvSdi.ConfigFileName, options);
#endif

        if (options.logToFile)
            UtyGLNvSdi.SdiSetupLogFile();

        StartCoroutine(OutputCoroutine);
    }



    void OnDisable()
    {
        if (sdiEnabled)
        {
            StopCoroutine(OutputCoroutine);

            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.Shutdown);
            sdiEnabled = false;
        }
    }


    private IEnumerator SdiOutputCoroutine()
    {

        UtyGLNvSdi.SdiOutputSetGlobalOptions();
        UtyGLNvSdi.SdiOutputSetVideoFormat(
            options.videoFormat,
            options.syncSource,
            options.outputDelay,
            options.outputHorizontalDelay,
            options.outputVerticalDelay,
            options.outputDual,
            options.outputRingBufferSize);

        yield return new WaitForEndOfFrame();

        GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.Initialize);
        yield return new WaitForEndOfFrame();


        if (UtyGLNvSdi.SdiError() == 0)
        {
            GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.Setup);

            if (!SetupOutputTextures())
            {
                UnityEngine.Debug.LogError("GLNvSdi_Plugin could not setup sdi textures for output");
            }

            sdiEnabled = true;

            while (true)
            {
                yield return new WaitForEndOfFrame();
                GL.IssuePluginEvent(UtyGLNvSdi.GetSdiOutputRenderEventFunc(), (int)SdiRenderEvent.PresentFrame);
            }
        }
       
    }




    bool SetupOutputTextures()
    {
        int texWidth = 1920;	// HD=1920, SD=720
        int texHeight = 1080;	// HD=1080, SD=486
        float lAspect = 0;
        bool lIsInterlaced = false;
        UtyGLNvSdi.GetSizeFromVideoFormat(options.videoFormat, ref texWidth, ref texHeight, ref lAspect, ref lIsInterlaced);

        // Check the count of cameras. The plugin support until two cameras
        if (outputTexture.Length > 4)
        {
            UnityEngine.Debug.LogWarning("GLNvSdi plugin does not support more than four cameras.");
        }

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
            RenderTexture tmp = outputTexture[1];
            outputTexture[1] = outputTexture[2];
            outputTexture[2] = tmp;
        }


        // Setting the cameras and directing the render to texture
        for (int i = 0; i < lTexCount; ++i)
        {
            if (outputTexture[i] != null)
            {
                //
                // Checking texture sizes
                //
                if (texWidth != outputTexture[i].width || texHeight != outputTexture[i].height)
                {
                    UnityEngine.Debug.LogError("The size of RenderTexture does not match with sdi video options");
                    return false;
                }

                //
                // Check if the texture has been created
                //
                if (!outputTexture[i].IsCreated())
                    if (!outputTexture[i].Create())
                        UnityEngine.Debug.LogError("Could not create Output Sdi RenderTexture");

                //
                // Sending texture id to plugin
                //
                UtyGLNvSdi.SdiOutputSetTexturePtr(i, outputTexture[i].GetNativeTexturePtr(), outputTexture[i].width, outputTexture[i].height);
            }
            else
            {
                UnityEngine.Debug.LogError("Sdi Output RenderTexture is null");
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

