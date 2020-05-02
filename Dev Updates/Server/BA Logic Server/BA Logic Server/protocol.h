#pragma once
#pragma pack(push, 1)

enum PACKET_TYPE {
    //SERVER TO CLIENT
    SC_PLAYER_UID,           //�÷��̾� UID �뺸, sc_player_uid ����ü ���
    SC_CREATE_PLAYER,        //�÷��̾� ����(������), sc_create_player ����ü ���
    SC_CREATE_BULLET,        //�Ѿ� ����, sc_create_bullet ����ü ���
    SC_DESTROY_PLAYER,       //�÷��̾� �ı�(����), sc_destroy_player ����ü ���
    SC_DESTROY_BULLET,       //�Ѿ� ������Ʈ �ı�, sc_destroy_bullet ����ü ���
    SC_PLAYER_INFO,          //������ ��ġ���� ��Ŷ, sc_player_info ����ü ���
    SC_BULLET_INFO,          //�Ѿ��� ��ġ���� ��Ŷ, sc_bullet_info ����ü ���
    SC_HIT,                  //�÷��̾� Ÿ��, sc_hit ����ü ���
    SC_GAME_START,           //���� ����, default_packet ����ü ���
    SC_GAME_END,             //���� ����, default_packet ����ü ���

    //CLIENT TO SERVER
    CS_KEYUP,                //�÷��̾� Ű���� �Է�(UP), cs_key_info ����ü ���
    CS_KEYDOWN,              //�÷��̾� Ű���� �Է�(DOWN), cs_key_info ����ü ���
    CS_ATTACK                //�÷��̾��� ����, cs_attack ����ü ���
};

struct default_packet
{
    char size;
    char type;
};

struct sc_player_uid
{
    char size;
    char type;
    char uid;   //�÷��̾��� UID
};

struct sc_create_player
{
    char size;
    char type;
    char uid;       //������ �÷��̾��� UID
    char hero;      //������ HERO Ÿ��
    int hp;         //ü��
    float pos[3];   //��ġ ����
    float rot[3];   //���� ���� - Degree
};

struct sc_create_bullet
{
    char size;
    char type;
    char shooter;   //�� ���
    char uid;       //������ �Ѿ��� UID
    float pos[3];   //��ġ ����
    float rot[3];   //���� ���� - Degree
};

struct sc_destroy_player
{
    char size;
    char type;
    char uid;       //�ش� UID�� �÷��̾� ���
};

struct sc_destroy_bullet
{
    char size;
    char type;
    char uid;       //�ش� UID�� �Ѿ� �ı�
};

struct sc_player_info
{
    char size;
    char type;
    char uid;       //�ش� UID �÷��̾� ��ġ����
    float pos[3];   //��ġ ����
    float rot[3];   //���� ���� - Degree
};

struct sc_bullet_info
{
    char size;
    char type;
    char uid;       //�ش� UID �Ѿ��� ��ġ����
    float pos[3];   //��ġ ����
    float rot[3];   //���� ���� - Degree
};

struct cs_key_info
{
    char size;
    char type;      //CS_KEYUP, CS_KEYDOWN
    char uid;       //�ش� UID �÷��̾� Ű �Է�
    char key;       //Ű(ASCII)
};

struct cs_attack
{
    char size;
    char type;
    char uid;       //�ش� UID �÷��̾��� ����
    float dir[3];   //Normalize�� ���ݹ���
};

struct sc_hit
{
    char size;
    char type;
    char uid;       //�ش� UID �÷��̾��� �ǰ�
    int damage;     //������
};

#pragma pack(pop)