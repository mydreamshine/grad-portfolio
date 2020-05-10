using System.Collections;
using System.Collections.Generic;
using UnityEngine;
public class CameraController : MonoBehaviour
{
	public GameObject target;
	public Vector3 offset;

	void Start()
	{
	}
	void LateUpdate()
	{
		if(target != null)
			transform.position = target.transform.position + offset;
	}

}