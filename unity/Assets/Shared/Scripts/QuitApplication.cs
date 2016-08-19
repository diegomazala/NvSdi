using UnityEngine;
using System.Collections;

public class QuitApplication : MonoBehaviour 
{
	// Update is called once per frame
	void Update () 
	{
#if UNITY_ANDROID || UNITY_IPHONE
        if (Input.GetKeyDown(KeyCode.Escape)) 
            Application.Quit(); 
#else
        if (Input.GetKeyDown(KeyCode.Q) && Input.GetKey(KeyCode.LeftControl))
            Application.Quit();
#endif
	}
}
