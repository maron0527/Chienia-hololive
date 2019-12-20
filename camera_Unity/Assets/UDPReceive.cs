using UnityEngine;
using System;
using System.Collections;
using System.Net;
using System.IO;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using MiniJSON;


[System.Serializable]
public class Face
{
    public float horizontal_pos;
    public float vertical_pos;
    public float yaw;
    public float pitch;
    public float roll;
    public float eye_L_X;
    public float eye_L_Y;
};

public class UDPReceive : MonoBehaviour
{
    [SerializeField]
    public Face actor;
    public GameObject Character;
    public GameObject LookTarget;
    int LOCAL_PORT = 22222;
    static UdpClient udp;
    Thread thread;
    public string parameter;

    public static Action<string> DataCallBack;

    void Start()
    {
        UDPReceive.DataCallBack += Method;
        UDPStart();
    }

    void Update()
    {
        /*if (Input.GetKey("W"))
        {
            //this.transform.position();
        }
        if (Input.GetKey("A"))
        {

        }
        if (Input.GetKey("S"))
        {

        }
        if (Input.GetKey("D"))
        {

        }*/
        Face actor = JsonUtility.FromJson<Face>(parameter);
        //Debug.Log(actor.pitch);
        Vector3 pos = GameObject.Find("Sphere").transform.position;
        //pos.x = actor.yaw;
        pos.y = actor.pitch;
        //pos.z = actor.roll;
        GameObject.Find("Sphere").transform.position = pos;
        //this.transform.LookAt(targetObject.transform);
    }

    public void UDPStart()
    {
        udp = new UdpClient(LOCAL_PORT);
        thread = new Thread(new ThreadStart(ThreadMethod));

        thread.Start();
    }

    public void Method(string data)
    {
        parameter = data;
        //Face actor = JsonUtility.FromJson<Face>(parameter);

        
        //Debug.Log(actor.pitch);

        //Debug.Log(parameter);
        //Debug.Log(parameter.LastIndexOf("horizontal_pos"));
    }

    void OnApplicationQuit()
    {
        thread.Abort();
    }

    private static void ThreadMethod()
    {
        while (true)
        {

            IPEndPoint remoteEP = null;
            byte[] data = udp.Receive(ref remoteEP);
            string text = Encoding.ASCII.GetString(data);

            DataCallBack(text);
            //Debug.Log(text);

        }
    }
}