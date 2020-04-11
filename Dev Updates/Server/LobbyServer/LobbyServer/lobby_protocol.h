#pragma once
#define LOBBYSERVER_PORT 15500

#define CS_PACKET_MATCH_ENQUEUE 0
#define CS_PACKET_MATCH_DEQUEUE 1
#define CB_PACKET_REQUEST_LOGIN 2

#define SC_PACKET_MATCH_ENQUEUE 0
#define SC_PACKET_MATCH_DEQUEUE 1
#define SC_PACKET_MATCH_ROOM_INFO 2
#define BS_PACKET_RESPONSE_ROOM 3

#define SB_PACKET_REQUEST_ROOM 0

#define GAMEMODE_NGP 0

struct common_default_packet
{
	CHAR size;
	CHAR type;
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