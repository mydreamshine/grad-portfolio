#pragma once
#define BATTLESERVER_PORT 15600

#define CS_PACKET_MATCH_ENQUEUE 0
#define CS_PACKET_MATCH_DEQUEUE 1

#define SC_PACKET_MATCH_ENQUEUE 0
#define SC_PACKET_MATCH_DEQUEUE 1
#define SC_PACKET_MATCH_ROOM_INFO 2

//struct common_default_packet
//{
//	CHAR size;
//	CHAR type;
//};
//
//struct sc_packet_match_room_info
//{
//	common_default_packet cdp;
//	int room_id;
//};