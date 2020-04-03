#pragma once

#define EXTEND_PACKET_SIZE 12
#define GENERAL_PACKET_SIZE 4

class Vector2d;

enum packet_type {
	p_system = 0x00010000,
	p_input = 0x00020000,
	p_obj = 0x00040000,
	p_event = 0x00080000,
};
enum player_num {//ÈÄ¿¡ Color·Î
	player1 = 0x00100000,
	player2 = 0x00200000,
	player3 = 0x00400000,
	player4 = 0x00800000,
};
enum packet_system {
	system_login_fail = 0x00000000,
	system_login_ok = 0x00000001,
	system_start = 0x00000002,
	system_end = 0x00000004,
};
enum packet_input {
	input_Wdown = 0x00001000,
	input_Wup = 0x00002000,
	input_Sdown = 0x00000100,
	input_Sup = 0x00000200,
	input_Adown = 0x00000010,
	input_Aup = 0x00000020,
	input_Ddown = 0x00000001,
	input_Dup = 0x00000002,
	input_Mleft = 0x00000003,
};
enum packet_obj_type {
	obj_player = 0x00001000,
	obj_bullet = 0x00002000,
	obj_item = 0x00004000,

	//obj_create = 0x000100,
	//obj_id		= 0x0000??
};
enum packet_obj_info {
	obj_position = 0x00000100,
	obj_destroy = 0x00000200,
};
enum packet_event {
	event_hit = 0x00001000,
	//event_target	= 0x000?00,
	//event_damage	= 0x0000??,
	event_chat = 0x00002000,
	//event_chatsize= 0x0000??,
};


//temp
bool On(const int& p_iflag, int pt);
void pTurnOn(int& p_iflag, const int pt);
void pTurnOff(int& p_iflag, const int pt);

bool is_extend_packet_client(const int& packet);
bool is_extend_packet_server(const int& packet);

int make_packet_chat(const int& client, int key);
int make_packet_input(const int& client, int key);
int make_packet_destroy_bullet(const int& client, int idx);
int make_packet_destroy_item(int idx);
int make_packet_destroy_player(const int& client);
int make_packet_hit_player(const int& client, int damage);
int make_packet_game_end(const int& winner);
int make_packet_game_start();
int get_packet_chat(const int& packet);
int get_packet_type(const int& packet);
int get_packet_obj_info(const int& packet);
int get_packet_obj_type(const int& packet);
int get_packet_player_num(const int& packet);
int get_packet_input(const int& packet);
int get_packet_bullet_idx(const int& packet);
int get_packet_bullet_type(const int& packet);
int get_packet_event_type(const int& packet);
int get_packet_system_type(const int& packet);

int get_player_num(const int& num);

