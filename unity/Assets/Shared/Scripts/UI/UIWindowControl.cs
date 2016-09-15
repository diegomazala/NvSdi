using UnityEngine;
using System.Collections;

[ExecuteInEditMode]
public class UIWindowControl : MonoBehaviour 
{
    protected UnityEngine.UI.Image imagePanel = null;
    
    public bool showUI = true;
    public RectTransform[] uiChildren;

    void Start()
    {
        imagePanel = GetComponent<UnityEngine.UI.Image>();

        ShowHide(showUI);
    }


    public void ShowHide()
    {
        showUI = !showUI;
        ShowHide(showUI);
    }


    public void ShowHide(bool show)
    {
        for (int i = 0; i < uiChildren.Length; ++i)
            uiChildren[i].gameObject.SetActive(show);

        if (imagePanel)
            imagePanel.enabled = show;
    }
}
