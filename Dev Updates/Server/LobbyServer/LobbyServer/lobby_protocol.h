#pragma once
#define LOBBYSERVER_PORT 15500
#define BATTLESERVER_PORT 15600
#define ID_LENGTH 11
using SIZE_TYPE = char;

/*
C : Client
S : Lobby Server
B : Battle Server

ex) CS -> Client to Lobby
	SB -> Lobby to Battle
	CB -> Client to Battle
	etc...
*/

//Client To Lobby
enum CS_PACKET {
	CS_PACKET_MATCH_ENQUEUE,
	CS_PACKET_MATCH_DEQUEUE,
	CB_PACKET_REQUEST_LOGIN,
	CS_PACKET_REQUEST_LOGIN,
	CS_PACKET_REQUEST_FRIEND,
	CS_PACKET_ACCEPT_FRIEND,
	CS_PACKET_COUNT
};

//Lobby To Client
enum SC_PACKET {
	SC_PACKET_LOGIN_OK,
	SC_PACKET_LOGIN_FAIL,
	SC_PACKET_MATCH_ENQUEUE,
	SC_PACKET_MATCH_DEQUEUE,
	SC_PACKET_MATCH_ROOM_INFO,
	SC_PACKET_FRIEND_STATUS,
	SC_PACKET_COUNT
};

//Lobby To Battle
enum SB_PACKET {
	SB_PACKET_REQUEST_ROOM,
	SB_PACKET_COUNT
};
//Battle To Lobby
enum BS_PACKET {
	BS_PACKET_RESPONSE_ROOM,
	BS_PACKET_COUNT
};

enum CB_PAKCET {
	//CLIENT TO SERVER
	CB_KEYUP,                //�÷��̾� Ű���� �Է�(UP), cs_key_info ����ü ���
	CB_KEYDOWN,              //�÷��̾� Ű���� �Է�(DOWN), cs_key_info ����ü ���
	CB_ATTACK,                //�÷��̾��� ����, cs_attack ����ü ���
	CB_PACKET_COUNT
};

enum BC_PACKET {
	//SERVER TO CLIENT
	BC_PLAYER_UID,           //�÷��̾� UID �뺸, sc_player_uid ����ü ���
	BC_CREATE_PLAYER,        //�÷��̾� ����(������), sc_create_player ����ü ���
	BC_CREATE_BULLET,        //�Ѿ� ����, sc_create_bullet ����ü ���
	BC_DESTROY_PLAYER,       //�÷��̾� �ı�(����), sc_destroy_player ����ü ���
	BC_DESTROY_BULLET,       //�Ѿ� ������Ʈ �ı�, sc_destroy_bullet ����ü ���
	BC_PLAYER_INFO,          //������ ��ġ���� ��Ŷ, sc_player_info ����ü ���
	BC_BULLET_INFO,          //�Ѿ��� ��ġ���� ��Ŷ, sc_bullet_info ����ü ���
	BC_HIT,                  //�÷��̾� Ÿ��, sc_hit ����ü ���
	BC_GAME_START,           //���� ����, default_packet ����ü ���
	BC_GAME_END,             //���� ����, default_packet ����ü ���

	BC_PACKET_COUNT
};

//Game Modes
enum GAME_MODES {
	GAMEMODE_NGP
};

#pragma pack(push, 1)
struct common_default_packet
{
	SIZE_TYPE size;
	char type;
};

struct cs_packet_request_login
{
	common_default_packet cdp;
	char id[ID_LENGTH]; // null-terminated string
};

struct cs_packet_request_friend
{
	common_default_packet cdp;
	char id[ID_LENGTH];
};
struct cs_packet_accept_friend
{
	common_default_packet cdp;
	char id[ID_LENGTH];
};
struct sc_packet_friend_status
{
	common_default_packet cdp;
	char id[ID_LENGTH];
	char status;
};
enum FRIEND_STATUS {
	FRIEND_ONLINE,
	FRIEND_OFFLINE
};

struct sc_packet_match_room_info
{
	common_default_packet cdp;
	int room_id;
};

struct sb_packet_request_room
{
	common_default_packet cdp;
	char mode;
};

struct bs_packet_response_room
{
	common_default_packet cdp;
	int room_id;
};

struct cb_packet_request_login
{
	common_default_packet cdp;
	int room_id;
};

struct bc_player_uid
{
	SIZE_TYPE size;
	char type;
	char uid;   //�÷��̾��� UID
};

struct bc_create_player
{
	SIZE_TYPE size;
	char type;
	char uid;       //������ �÷��̾��� UID
	char hero;      //������ HERO Ÿ��
	int hp;         //ü��
	float pos[3];   //��ġ ����
	float rot[3];   //���� ���� - Degree
};

struct bc_create_bullet
{
	SIZE_TYPE size;
	char type;
	char shooter;   //�� ���
	char uid;       //������ �Ѿ��� UID
	float pos[3];   //��ġ ����
	float rot[3];   //���� ���� - Degree
};

struct bc_destroy_player
{
	SIZE_TYPE size;
	char type;
	char uid;       //�ش� UID�� �÷��̾� ���
};

struct bc_destroy_bullet
{
	SIZE_TYPE size;
	char type;
	char uid;       //�ش� UID�� �Ѿ� �ı�
};

struct bc_player_info
{
	SIZE_TYPE size;
	char type;
	char uid;       //�ش� UID �÷��̾� ��ġ����
	float pos[3];   //��ġ ����
	float rot[3];   //���� ���� - Degree
};

struct bc_bullet_info
{
	SIZE_TYPE size;
	char type;
	char uid;       //�ش� UID �Ѿ��� ��ġ����
	float pos[3];   //��ġ ����
	float rot[3];   //���� ���� - Degree
};

struct bc_hit
{
	SIZE_TYPE size;
	char type;
	char uid;       //�ش� UID �÷��̾��� �ǰ�
	int damage;     //������
};


struct cb_key_info
{
	SIZE_TYPE size;
	char type;      //CS_KEYUP, CS_KEYDOWN
	char uid;       //�ش� UID �÷��̾� Ű �Է�
	char key;       //Ű(ASCII)
};

struct cb_attack
{
	SIZE_TYPE size;
	char type;
	char uid;       //�ش� UID �÷��̾��� ����
	float dir[3];   //Normalize�� ���ݹ���
};
#pragma pack(pop)