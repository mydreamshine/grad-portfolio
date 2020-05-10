#pragma once
#define LOBBYSERVER_PORT 15500
#define BATTLESERVER_PORT 15600
#define ID_LENGTH 11
/*
C : Client
S : Lobby Server
B : Battle Server

ex) CS -> Client to Lobby
	SB -> Lobby to Battle
	CB -> Client to Battle
	etc...
*/



enum PACKET_TYPE {
	//Client -> Other
	CS_PACKET_MATCH_ENQUEUE,
	CS_PACKET_MATCH_DEQUEUE,
	CB_PACKET_REQUEST_LOGIN,
	CS_PACKET_REQUEST_LOGIN,
	CS_PACKET_REQUEST_FRIEND,
	CS_PACKET_ACCEPT_FRIEND,

	////Lobby -> Other
	SC_PACKET_LOGIN_OK,
	SC_PACKET_LOGIN_FAIL,
	SC_PACKET_MATCH_ENQUEUE,
	SC_PACKET_MATCH_DEQUEUE,
	SC_PACKET_MATCH_ROOM_INFO,
	SC_PACKET_FRIEND_STATUS,
	BS_PACKET_RESPONSE_ROOM,

	SB_PACKET_REQUEST_ROOM
};

//Game Modes
enum GAME_MODES {
	GAMEMODE_NGP
};

struct common_default_packet
{
	CHAR size;
	CHAR type;
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