using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class LookAtTest : MonoBehaviour
{
    public Transform _target;

    readonly int COUNT = 100000;

    void Update()
    {
                this.transform.LookAt(_target.position);
    }
}