using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Looktarget : MonoBehaviour
{
    GameObject Character;
    UDPReceive script;
    // Start is called before the first frame update
    void Start()
    {
        Character= GameObject.Find("c_actor");
        script = Character.GetComponent<UDPReceive>();
    }

    // Update is called once per frame
    void Update()
    {
        float pitch = script.actor.pitch;
        Debug.Log(pitch);
        Transform myTransform = this.transform;
        Vector3 pos = myTransform.position;
        pos.x = script.actor.pitch;
        pos.y = script.actor.yaw;
        pos.z = 0;
        myTransform.position = pos;
    }
}
