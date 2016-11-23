using UnityEngine;
using System.Collections;
using System;
using System.Diagnostics;

public class ProcessPriority : MonoBehaviour 
{
	void Start () 
    {
        // Not recommended to change to RealTime
        System.Diagnostics.Process.GetCurrentProcess().PriorityClass = ProcessPriorityClass.High;
	}
}
