#pragma once
#include "PacketMgr.h"

bool On(const int& p_iflag, int pt) { return (p_iflag & pt) == pt; }
void pTurnOn(int& p_iflag, const int pt) { p_iflag |= pt; }
void pTurnOff(int& p_iflag, const int pt) { if (On(p_iflag, pt))   p_iflag ^= pt; }

bool is_extend_packet_client(const int& packet)
{
	int packet_type = get_packet_type(packet);
	if (packet_type == p_obj) {
		int packet_info = get_packet_obj_info(packet);
		if (packet_info == obj_position)
			return true;
	}

	return false;
}

bool is_extend_packet_server(const int& packet)
{
	int packet_type = get_packet_type(packet);
	if (packet_type == p_input) {
		int packet_input = get_packet_input(packet);
		if (packet_input == input_Mleft)
			return true;
	}

	return false;
}

int make_packet_chat(const int& client, int key) {
	int packet = 0;
	pTurnOn(packet, p_event);
	pTurnOn(packet, event_chat);
	pTurnOn(packet, client);
	pTurnOn(packet, key);
	return packet;
}

int make_packet_input(const int& client, int key) {
	int packet = 0;
	pTurnOn(packet, p_input);
	pTurnOn(packet, client);
	pTurnOn(packet, key);
	return packet;
}
int make_packet_destroy_bullet(const int& client, int idx) {
	int packet = 0;
	pTurnOn(packet, p_obj);
	pTurnOn(packet, obj_bullet);
	pTurnOn(packet, obj_destroy);
	pTurnOn(packet, client);
	pTurnOn(packet, idx);
	return packet;
}
int make_packet_destroy_item(int idx) {
	int packet = 0;
	pTurnOn(packet, p_obj);
	pTurnOn(packet, obj_item);
	pTurnOn(packet, obj_destroy);
	pTurnOn(packet, idx);
	return packet;
}
int make_packet_destroy_player(const int& client) {
	int packet = 0;
	pTurnOn(packet, p_obj);
	pTurnOn(packet, obj_player);
	pTurnOn(packet, obj_destroy);
	pTurnOn(packet, client);
	return packet;
}
int make_packet_hit_player(const int& client, int damage) {
	int packet = 0;
	pTurnOn(packet, p_event);
	pTurnOn(packet, event_hit);
	pTurnOn(packet, client);
	pTurnOn(packet, damage);
	return packet;
}
int make_packet_game_end(const int& winner)
{
	int packet = 0;
	pTurnOn(packet, p_system);
	pTurnOn(packet, system_end);
	pTurnOn(packet, winner);
	return packet;
}
int make_packet_game_start()
{
	int packet = 0;
	pTurnOn(packet, p_system);
	pTurnOn(packet, system_start);
	return packet;
}

int get_packet_chat(const int& packet) {
	return packet & 0x000000FF;
}
int get_packet_type(const int& packet) {
	return packet & 0x000F0000;
}
int get_packet_obj_info(const int& packet) {
	return packet & 0x00000F00;
}
int get_packet_obj_type(const int& packet) {
	return packet & 0x0000F000;
}
int get_packet_player_num(const int& packet) {
	return packet & 0x00F00000;
}
int get_packet_input(const int& packet) {
	return packet & 0x0000FFFF;
}
int get_packet_event_type(const int& packet) {
	return packet & 0x0000F000;
}
int get_packet_bullet_type(const int& packet) {
	int type = packet >> 24;
	return type & 0x000000FF;
}
int get_packet_system_type(const int& packet)
{
	return packet & 0x0000000F;
}

int get_packet_bullet_idx(const int& packet)
{
	return packet & 0x000000FF;
}

int get_player_num(const int& num)
{
	int player = 0;
	switch (num) {
	case 1:
		pTurnOn(player, player1);
		break;
	case 2:
		pTurnOn(player, player2);
		break;
	case 3:
		pTurnOn(player, player3);
		break;
	}

	return player;
}