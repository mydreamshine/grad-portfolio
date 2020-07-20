#pragma once
#include "ROOM.h"
#include "PACKET_VECTOR.h"
#include <mutex>
#include <map>
#include <set>

#include <DirectXCollision.h>
#include "..\Common\HERO.h"
#include "..\Common\SKILL.h"
#include "..\Common\SCOREBOARD.h"
#include "..\..\Streaming\Streaming_Server\Streaming_Server\packet_struct.h"

class DMRoom : public ROOM
{
public:
	DMRoom();
	virtual ~DMRoom();
	virtual void init();
	virtual bool regist(SOCKET client, void* buffer);
	virtual void disconnect(SOCKET client);
	virtual void start();
	virtual void end();
	virtual bool update(float elapsedTime);
	virtual void process_packet(CLIENT* client, int ReceivedBytes);

private:
	short max_player; ///< max connect user num.
	short player_num; ///< current user num

	std::mutex packet_lock;
	PACKET_VECTOR packet_vector;
	PACKET_VECTOR packets;

	PACKET_VECTOR event_data; //전송될 이벤트 패킷(플레이어 힛, 리스폰 등)
	PACKET_VECTOR info_data;  //전송될 위치정보 패킷

	float delta_time;
	std::mutex socket_lock;
	std::set<SOCKET> sockets;
	std::map<short, HERO*> m_heros;
	std::map<short, SCOREBOARD> m_score;
	int skill_uid;
	std::map<int, SKILL*> m_skills;
	std::vector<DirectX::BoundingBox> m_walls;

	HERO* spawn_hero(short object_id, char character_type, char propensity);

	void process_packet_vector();
	bool game_logic();
	void send_game_state();

	void process_type_packet(void* packet, PACKET_TYPE type);
	void process_try_move_character(void* packet);
	void process_try_move_stop_character(void* packet);
	void process_try_normal_attack(void* packet);
	void process_try_use_skill(void* packet);
	void process_done_character_motion(void* packet);
	void process_activate_anim_notify(void* packet);

	friend HERO;
	friend WARRIOR;
	friend BERSERKER;
	friend PRIEST;
	friend ASSASSIN;
};