using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using System.Net.Sockets;
using System.Threading;
using System.Text;
using System;
using System.Runtime.InteropServices;

public class PACKET_VECTOR
{
	public byte[] data;
    public int len;
    public int max_len;
    public PACKET_VECTOR()
    {
        data = new byte[100];
        len = 0;
        max_len = 100;
    }
    public void emplace_back(byte[] src, int len)
    {
        int require_len = this.len + len;
        if (require_len > max_len)
        {
            byte[] new_data = new byte[require_len];
            Array.Copy(data, new_data, this.len);
            data = new_data;
            max_len = require_len;
        }
        Array.Copy(src, 0, data, this.len, len);
        this.len += len;
    }
    public byte[] get_data()
    {
        if (this.len == 0) return null;
        byte[] return_data = new byte[this.len];
        Array.Copy(data, return_data, this.len);
        return return_data;
    }
    public void clear() { len = 0; }
};

public class PacketManager : MonoBehaviour
{
    Thread recvThread;
    TcpClient tcpClient;
    NetworkStream ns;
    readonly object packet_lock = new object();
    PACKET_VECTOR packet_vector = new PACKET_VECTOR();
    int MYUID = -1;

    Dictionary<int, GameObject> m_players = new Dictionary<int, GameObject>();
    Dictionary<int, GameObject> m_bullets = new Dictionary<int, GameObject>();
    float shot_interval = 0.0f;

    public GameObject player_model;
    public GameObject bullet_model;
    public CameraController cam;

    void recvFunc()
    {
        tcpClient = new TcpClient("127.0.0.1", 15600);
        ns = tcpClient.GetStream();

        byte[] buffer = new byte[2048];
        byte[] saved_packet = new byte[2048];
        int need_size = 0;
        int saved_size = 0;
        int buf_pos = 0;
        PACKET_VECTOR dummy_packet_vector = new PACKET_VECTOR();

        UnityEngine.Debug.Log("클라이언트 연결");

        while (true)
        {
            int retval = ns.Read(buffer, 0, 2048);
            buf_pos = 0;
            while (retval > 0)
            {
                if (need_size == 0) need_size = buffer[buf_pos];
                if (retval + saved_size >= need_size)
                {
                    int copy_size = (need_size - saved_size);
                    Array.Copy(buffer, buf_pos, saved_packet, saved_size, copy_size);

                    //패킷 완성
                    dummy_packet_vector.emplace_back(saved_packet, need_size);

                    buf_pos += copy_size;
                    retval -= copy_size;
                    saved_size = 0;
                    need_size = 0;
                }
                else
                {
                    Array.Copy(buffer, buf_pos, saved_packet, saved_size, retval);
                    saved_size += retval;
                    retval = 0;
                }
            }

            lock (packet_lock) { packet_vector.emplace_back(dummy_packet_vector.data, dummy_packet_vector.len); }
            dummy_packet_vector.clear();
        }
        ns.Close();
        tcpClient.Close();
    }

    // Start is called before the first frame update
    void Start()
    {
        recvThread = new Thread( () => recvFunc() );
        recvThread.Start();
    }

    // Update is called once per frame
    void Update()
    {
        shot_interval -= Time.deltaTime;
        process_packets();
        process_keyboard();
        process_mouse();
    }
    void process_mouse()
    {
    }
    void process_keyboard()
    {
        if (Input.GetKeyDown(KeyCode.W))
        {
            send_keyInput((char)PACKET_TYPE.CS_KEYDOWN, 'w');
        }
        if (Input.GetKeyUp(KeyCode.W))
        {
            send_keyInput((char)PACKET_TYPE.CS_KEYUP, 'w');
        }
        if (Input.GetKeyDown(KeyCode.A))
        {
            send_keyInput((char)PACKET_TYPE.CS_KEYDOWN, 'a');
        }
        if (Input.GetKeyUp(KeyCode.A))
        {
            send_keyInput((char)PACKET_TYPE.CS_KEYUP, 'a');
        }
        if (Input.GetKeyDown(KeyCode.S))
        {
            send_keyInput((char)PACKET_TYPE.CS_KEYDOWN, 's');
        }
        if (Input.GetKeyUp(KeyCode.S))
        {
            send_keyInput((char)PACKET_TYPE.CS_KEYUP, 's');
        }
        if (Input.GetKeyDown(KeyCode.D))
        {
            send_keyInput((char)PACKET_TYPE.CS_KEYDOWN, 'd');
        }
        if (Input.GetKeyUp(KeyCode.D))
        {
            send_keyInput((char)PACKET_TYPE.CS_KEYUP, 'd');
        }

        Vector3 dir = new Vector3();
        if (Input.GetKey(KeyCode.UpArrow))
            dir += new Vector3(0, 0, 1);
        if (Input.GetKey(KeyCode.LeftArrow))
            dir += new Vector3(-1, 0, 0);
        if (Input.GetKey(KeyCode.RightArrow))
            dir += new Vector3(1, 0, 0);
        if (Input.GetKey(KeyCode.DownArrow))
            dir += new Vector3(0, 0, -1);
        dir = dir.normalized;
        if ((dir.magnitude != 0) && (shot_interval <= 0.0f))
        {
            shot_interval = 0.5f;
            send_cs_attack(dir);
        }
    }
    void process_packets()
    {
        byte[] packets;
        lock (packet_lock)
        {
            packets = packet_vector.get_data();
            packet_vector.clear();
        }
        if (packets == null) return;

        int packet_pos = 0;
        int packet_size = 0;
        int packet_length = packets.Length;
        while (packet_length > 0)
        {
            default_packet dp = ByteToStruct<default_packet>(packets, packet_pos);
            packet_size = dp.size;
            process_type_packet(packets, packet_pos, dp.type);
            packet_length -= packet_size;
            packet_pos += packet_size;
        }
    }

    public void process_type_packet(byte[] packets, int packet_pos, int type)
    {
        switch(type)
        {
            case (int)PACKET_TYPE.SC_PLAYER_UID:
                {
                    sc_player_uid packet = ByteToStruct<sc_player_uid>(packets, packet_pos);
                    MYUID = packet.uid;
                    UnityEngine.Debug.Log("UID 설정 : " + MYUID.ToString());
                    break;
                }
                
            case (int)PACKET_TYPE.SC_CREATE_PLAYER:
                {
                    sc_create_player packet = ByteToStruct<sc_create_player>(packets, packet_pos);
                    Vector3 pos = new Vector3(packet.pos[0], packet.pos[1], packet.pos[2]);
                    Quaternion rot = Quaternion.Euler(packet.rot[0], packet.rot[1], packet.rot[2]);
                    GameObject new_player = Instantiate(player_model, pos, rot);
                    m_players.Add(packet.uid, new_player);
                    UnityEngine.Debug.Log("플레이어 " + packet.uid + " 생성, 위치 : " + pos.ToString() +  "");
                    cam.target = new_player;
                    break;
                }

            case (int)PACKET_TYPE.SC_PLAYER_INFO:
                {
                    sc_player_info packet = ByteToStruct<sc_player_info>(packets, packet_pos);
                    if (!m_players.ContainsKey(packet.uid)) break;

                    Vector3 pos = new Vector3(packet.pos[0], packet.pos[1], packet.pos[2]);
                    Quaternion rot = Quaternion.Euler(packet.rot[0], packet.rot[1], packet.rot[2]);
                    GameObject target_player = m_players[packet.uid];
                    target_player.transform.position = pos;
                    target_player.transform.rotation = rot;
                    //UnityEngine.Debug.Log("플레이어 " + packet.uid + " 정보 패킷, 위치 : " + pos.ToString() + ", 회전값 : " + rot.eulerAngles.ToString());
                    break;
                }

            case (int)PACKET_TYPE.SC_CREATE_BULLET:
                {
                    sc_create_bullet packet = ByteToStruct<sc_create_bullet>(packets, packet_pos);
                    Vector3 pos = new Vector3(packet.pos[0], packet.pos[1], packet.pos[2]);
                    Quaternion rot = Quaternion.Euler(packet.rot[0], packet.rot[1], packet.rot[2]);
                    GameObject new_bullet = Instantiate(bullet_model, pos, rot);
                    m_bullets.Add(packet.uid, new_bullet);
                    UnityEngine.Debug.Log("탄환 " + packet.uid + " 생성, 위치 : " + pos.ToString() + "");
                    break;
                }

            case (int)PACKET_TYPE.SC_DESTROY_BULLET:
                {
                    sc_destroy_bullet packet = ByteToStruct<sc_destroy_bullet>(packets, packet_pos);
                    Destroy(m_bullets[packet.uid]);
                    m_bullets.Remove(packet.uid);
                    UnityEngine.Debug.Log("탄환 " + (int)packet.uid + " 제거");
                    break;
                }

            case (int)PACKET_TYPE.SC_BULLET_INFO:
                {
                    sc_player_info packet = ByteToStruct<sc_player_info>(packets, packet_pos);
                    if (!m_bullets.ContainsKey(packet.uid)) break;

                    Vector3 pos = new Vector3(packet.pos[0], packet.pos[1], packet.pos[2]);
                    Quaternion rot = Quaternion.Euler(packet.rot[0], packet.rot[1], packet.rot[2]);
                    GameObject target_bullet = m_bullets[packet.uid];
                    target_bullet.transform.position = pos;
                    target_bullet.transform.rotation = rot;
                    break;
                }

            case (int)PACKET_TYPE.SC_HIT:
                {
                    sc_hit packet = ByteToStruct<sc_hit>(packets, packet_pos);
                    UnityEngine.Debug.Log("탄환 " + packet.uid + " 데미지 : " + packet.damage);
                    break;
                }

                default:
                break;
        }
    }
    public static byte[] StructToByte(object obj)
    {
        int size = Marshal.SizeOf(obj);
        byte[] arr = new byte[size];
        IntPtr ptr = Marshal.AllocHGlobal(size);
        Marshal.StructureToPtr(obj, ptr, true);
        Marshal.Copy(ptr, arr, 0, size);
        Marshal.FreeHGlobal(ptr);
        return arr;
    }
    public static T ByteToStruct<T>(byte[] buffer, int offset) where T : struct
    {
        int size = Marshal.SizeOf(typeof(T));
        if (size > buffer.Length)
        {
            throw new Exception();
        }
        IntPtr ptr = Marshal.AllocHGlobal(size);
        Marshal.Copy(buffer, offset, ptr, size);
        T obj = (T)Marshal.PtrToStructure(ptr, typeof(T));
        Marshal.FreeHGlobal(ptr);
        return obj;
    }
    public void send_cs_attack(Vector3 dir)
    {
        cs_attack packet = new cs_attack();
        packet.size = (char)Marshal.SizeOf(packet);
        packet.type = (char)PACKET_TYPE.CS_ATTACK;
        packet.dir = new float[3];
        packet.dir[0] = dir.x; packet.dir[1] = dir.y; packet.dir[2] = dir.z;
        packet.uid = (char)MYUID;
        byte[] byte_packet = StructToByte(packet);
        ns.Write(byte_packet, 0, packet.size);
    }
    public void send_keyInput(char UpDown, char key)
    {
        cs_key_info packet = new cs_key_info();
        packet.size = (char)Marshal.SizeOf(packet);
        packet.type = UpDown;
        packet.uid = (char)MYUID;
        packet.key = key;
        byte[] byte_packet = StructToByte(packet);
        ns.Write(byte_packet, 0, packet.size);
    }
}

enum PACKET_TYPE {
    //SERVER TO CLIENT
    SC_PLAYER_UID,           //플레이어 UID 통보
    SC_CREATE_PLAYER,        //플레이어 생성(리스폰)
    SC_CREATE_BULLET,        //총알 생성
    SC_DESTROY_PLAYER,       //플레이어 파괴(죽음)
    SC_DESTROY_BULLET,       //총알 오브젝트 파괴
    SC_PLAYER_INFO,          //유저의 위치정보 패킷
    SC_BULLET_INFO,          //총알의 위치정보 패킷
    SC_HIT,                  //플레이어 타격
    SC_GAME_START,           //게임 시작
    SC_GAME_END,             //게임 종료

    //CLIENT TO SERVER
    CS_KEYUP,                //플레이어 키보드 입력(UP)
    CS_KEYDOWN,              //플레이어 키보드 입력(DOWN)
    CS_ATTACK                //플레이어의 공격
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct default_packet
{
    public char size;
    public char type;
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct sc_player_uid
{
    public char size;
    public char type;
    public char uid;
}
[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct sc_create_player
{
    public char size;
    public char type;
    public char uid;
    public char hero;
    public int hp;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public float[] pos;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public float[] rot;
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct sc_create_bullet
{
    public char size;
    public char type;
    public char shooter;
    public char uid;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public float[] pos;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public float[] rot;
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct sc_destroy_player
{
    public char size;
    public char type;
    public char uid;
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct sc_destroy_bullet
{
    public char size;
    public char type;
    public char uid;
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct sc_player_info
{
    public char size;
    public char type;
    public char uid;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public float[] pos;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public float[] rot;
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct sc_bullet_info
{
    public char size;
    public char type;
    public char uid;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public float[] pos;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public float[] rot;
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct cs_key_info
{
    public char size;
    public char type;
    public char uid;
    public char key;
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct cs_attack
{
    public char size;
    public char type;
    public char uid;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public float[] dir;
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct sc_hit
{
    public char size;
    public char type;
    public char uid;
    public int damage;
}