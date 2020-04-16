#pragma once
#define LOBBYSERVER_PORT 15500


enum PACKET_TYPE {
	//Client -> Other
	CS_PACKET_MATCH_ENQUEUE,
	CS_PACKET_MATCH_DEQUEUE,
	CB_PACKET_REQUEST_LOGIN,
	CS_PACKET_REQUEST_LOGIN,

	////Lobby -> Other
	SC_PACKET_LOGIN_OK,
	SC_PACKET_LOGIN_FAIL,
	SC_PACKET_MATCH_ENQUEUE,
	SC_PACKET_MATCH_DEQUEUE,
	SC_PACKET_MATCH_ROOM_INFO,
	BS_PACKET_RESPONSE_ROOM,

	SB_PACKET_REQUEST_ROOM
};

//#define CS_PACKET_MATCH_ENQUEUE 0
//#define CS_PACKET_MATCH_DEQUEUE 1
//#define CB_PACKET_REQUEST_LOGIN 2
//#define CS_PACKET_REQUEST_LOGIN 3
//
////Lobby -> Other
//#define SC_PACKET_MATCH_ENQUEUE 0
//#define SC_PACKET_MATCH_DEQUEUE 1
//#define SC_PACKET_MATCH_ROOM_INFO 2
//#define BS_PACKET_RESPONSE_ROOM 3
//
//#define SB_PACKET_REQUEST_ROOM 0

//Game Modes
#define GAMEMODE_NGP 0

struct common_default_packet
{
	CHAR size;
	CHAR type;
};

struct cs_packet_request_login
{
	common_default_packet cdp;
	char id[11]; // null-terminated string
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