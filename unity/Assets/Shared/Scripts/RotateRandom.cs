using UnityEngine;
using System.Collections;

public class RotateRandom : MonoBehaviour 
{
    public float speed = 20.0f;
    public Vector3 axis = Vector3.one;

	void Update () 
    {
        transform.Rotate(Time.deltaTime * speed * axis);
	}
}
