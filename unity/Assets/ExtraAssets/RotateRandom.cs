using UnityEngine;
using System.Collections;

public class RotateRandom : MonoBehaviour 
{
	void Update () 
    {
        transform.Rotate(Time.deltaTime * 50, Time.deltaTime * 20, -Time.deltaTime * 30);
	}
}
