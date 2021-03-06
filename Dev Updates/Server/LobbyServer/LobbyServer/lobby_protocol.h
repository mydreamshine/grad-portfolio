//#pragma once
//#define BATTLESERVER_PORT 15600
//
///*
//C : Client
//S : Lobby Server
//B : Battle Server
//
//ex) CS -> Client to Lobby
//	SB -> Lobby to Battle
//	CB -> Client to Battle
//	etc...
//*/
//
//////Client To Lobby
////enum CS_PACKET {
////	CS_PACKET_MATCH_ENQUEUE,
////	CS_PACKET_MATCH_DEQUEUE,
////	CB_PACKET_REQUEST_LOGIN,
////	CS_PACKET_REQUEST_LOGIN,
////	CS_PACKET_REQUEST_FRIEND,
////	CS_PACKET_ACCEPT_FRIEND,
////	CS_PACKET_COUNT
////};
////
//////Lobby To Client
////enum SC_PACKET {
////	SC_PACKET_LOGIN_OK,
////	SC_PACKET_LOGIN_FAIL,
////	SC_PACKET_MATCH_ENQUEUE,
////	SC_PACKET_MATCH_DEQUEUE,
////	SC_PACKET_MATCH_ROOM_INFO,
////	SC_PACKET_FRIEND_STATUS,
////	SC_PACKET_COUNT
////};
//
//
//
////enum CB_PAKCET {
////	//CLIENT TO SERVER
////	CB_KEYUP,                //플레이어 키보드 입력(UP), cs_key_info 구조체 사용
////	CB_KEYDOWN,              //플레이어 키보드 입력(DOWN), cs_key_info 구조체 사용
////	CB_ATTACK,                //플레이어의 공격, cs_attack 구조체 사용
////	CB_PACKET_COUNT
////};
////
////enum BC_PACKET {
////	//SERVER TO CLIENT
////	BC_PLAYER_UID,           //플레이어 UID 통보, sc_player_uid 구조체 사용
////	BC_CREATE_PLAYER,        //플레이어 생성(리스폰), sc_create_player 구조체 사용
////	BC_CREATE_BULLET,        //총알 생성, sc_create_bullet 구조체 사용
////	BC_DESTROY_PLAYER,       //플레이어 파괴(죽음), sc_destroy_player 구조체 사용
////	BC_DESTROY_BULLET,       //총알 오브젝트 파괴, sc_destroy_bullet 구조체 사용
////	BC_PLAYER_INFO,          //유저의 위치정보 패킷, sc_player_info 구조체 사용
////	BC_BULLET_INFO,          //총알의 위치정보 패킷, sc_bullet_info 구조체 사용
////	BC_HIT,                  //플레이어 타격, sc_hit 구조체 사용
////	BC_GAME_START,           //게임 시작, default_packet 구조체 사용
////	BC_GAME_END,             //게임 종료, default_packet 구조체 사용
////
////	BC_PACKET_COUNT
////};
//
////Game Modes
//enum GAME_MODES {
//	GAMEMODE_NGP
//};
//
//using PACKET_SIZE = unsigned char;
//using PACKET_TYPE = unsigned char;
//#pragma pack(push, 1)
////struct common_default_packet
////{
////	PACKET_SIZE size;
////	PACKET_TYPE type;
////};
//////
////struct cs_packet_request_login
////{
////	common_default_packet cdp;
////	char id[ID_LENGTH]; // null-terminated string
////};
////
////struct cs_packet_request_friend
////{
////	common_default_packet cdp;
////	char id[ID_LENGTH];
////};
////struct cs_packet_accept_friend
////{
////	common_default_packet cdp;
////	char id[ID_LENGTH];
////};
////struct sc_packet_friend_status
////{
////	common_default_packet cdp;
////	char id[ID_LENGTH];
////	char status;
////};
//
////enum FRIEND_STATUS {
////	FRIEND_ONLINE,
////	FRIEND_OFFLINE
////};
//
////struct sc_packet_match_room_info
////{
////	common_default_packet cdp;
////	int room_id;
////};
////
////struct sb_packet_request_room
////{
////	common_default_packet cdp;
////	char mode;
////};
////
////struct bs_packet_response_room
////{
////	common_default_packet cdp;
////	int room_id;
////};
////
////struct cb_packet_request_login
////{
////	common_default_packet cdp;
////	int room_id;
////};
//#pragma pack(pop)