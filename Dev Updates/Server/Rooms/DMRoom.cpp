#include "DMRoom.h"
#include "CLIENT.h"
#include "CharacterConfig.h"

DMRoom::DMRoom() :
	max_player(2),
	player_num(0),
	delta_time(0),
	skill_uid(4)
{
	packet_vector.clear();
	packets.clear();
	event_data.clear();
	info_data.clear();
	for (int i = 0; i < max_player; ++i)
		m_heros[i] = nullptr;
}

DMRoom::~DMRoom()
{
}

void DMRoom::init()
{
	// Loading config from resource manager - Client will update this.
}

bool DMRoom::regist(CLIENT* client)
{
	socket_lock.lock();
	int cur_player = ++player_num;
	sockets.emplace(client->socket);
	csss_packet_login_ok packet;
	packet.client_id = cur_player;
	packet.type = CSSS_LOGIN_OK;
	packet.size = sizeof(csss_packet_login_ok);
	send_packet(client->socket, &packet, packet.size);
	socket_lock.unlock();

	return cur_player == max_player;
}

void DMRoom::disconnect(CLIENT* client)
{
	socket_lock.lock();
	--player_num;
	sockets.erase(client->socket);
	socket_lock.unlock();
}

void DMRoom::start()
{
	//Insert changed scene packet.
	csss_packet_change_scene packet;
	packet.type = CSSS_CHANGE_SCENE;
	packet.size = sizeof(csss_packet_change_scene);
	//packet.scene_type = (char)
	event_data.emplace_back(&packet, packet.size);
}

void DMRoom::end()
{
	socket_lock.lock();
	sockets.clear();
	socket_lock.unlock();

	for (auto& hero : m_heros)
		delete hero.second;
	m_heros.clear();

	for (auto& skill : m_skills)
		delete skill.second;
	m_skills.clear();
	m_walls.clear();
}

void DMRoom::process_packet(CLIENT* client, int ReceivedBytes)
{
	packet_lock.lock();
	packet_vector.emplace_back(client->recv_over.data(), ReceivedBytes);
	packet_lock.unlock();
}

bool DMRoom::update(float elapsedTime)
{
	delta_time = elapsedTime;
    process_packet_vector();
    bool retval = game_logic();
    send_game_state();
    return retval;
}

void DMRoom::process_packet_vector()
{
	packets.clear();
	packet_lock.lock();
	packets.emplace_back(packet_vector.data, packet_vector.len);
	packet_vector.clear();
	packet_lock.unlock();

	char* packet_pos = packets.data;
	PACKET_SIZE packet_size = 0;
	size_t packet_length = packets.len;
	while (packet_length > 0)
	{
		default_packet* dp = reinterpret_cast<default_packet*>(packet_pos);
		packet_size = dp->size;
		process_type_packet(packet_pos, dp->type);
		packet_length -= packet_size;
		packet_pos += packet_size;
	}
}

bool DMRoom::game_logic()
{
	for (auto& hero : m_heros)
		hero.second->update(delta_time);
	for (auto& skill : m_skills)
		skill.second->update(delta_time);

	//Collision check and process.
    for (auto& skill : m_skills) {
		//Hero and Skill.
        for (auto& hero : m_heros) {
            if (true == hero.second->is_die()) continue;
            if (true == hero.second->AABB.Intersects(skill.second->AABB)) {
                skill.second->effect(hero.second);
            }
        }

        //Skill and Wall
        for (auto& wall : m_walls)
            if (true == skill.second->AABB.Intersects(wall))
                skill.second->collision_wall();
	}

	for (auto& hero : m_heros) {
		if (true == hero.second->is_die()) continue;
		//Hero and Wall
		for (auto& wall : m_walls)
			if (true == hero.second->AABB.Intersects(wall))
				hero.second->correct_position(wall);
	}

	//Garbage Collection.
	for (auto skill = m_skills.begin(); skill != m_skills.end(); ) {
		if (true == skill->second->is_die()) {
			csss_packet_deactivate_obj packet;
			packet.type = CSSS_DEACTIVATE_OBJ;
			packet.size = sizeof(csss_packet_deactivate_obj);
			packet.object_id = skill->first;
			event_data.emplace_back(&packet, packet.size);
			delete skill->second;
			m_skills.erase(skill++);
		}
		else
			++skill;
	}

	//Game End Check.

	return true;
}

void DMRoom::send_game_state()
{
	for (const auto& hero : m_heros) {
		//Send hero transform.
		csss_packet_set_obj_transform transform_packet;
		transform_packet.size = sizeof(csss_packet_set_obj_transform);
		transform_packet.type = CSSS_SET_OBJ_TRANSFORM;
		transform_packet.object_id = hero.first;
		transform_packet.position_x = hero.second->pos.x; transform_packet.position_y = hero.second->pos.y; transform_packet.position_z = hero.second->pos.z;
		transform_packet.rotation_euler_x = hero.second->rot.x; transform_packet.rotation_euler_y = hero.second->rot.y; transform_packet.rotation_euler_z = hero.second->rot.z;
		transform_packet.scale_x = transform_packet.scale_y = transform_packet.scale_z = 1.0f;
		info_data.emplace_back(&transform_packet, transform_packet.size);

		//Send hero animation.
		csss_packet_set_character_motion motion_packet;
		motion_packet.size = sizeof(csss_packet_set_character_motion);
		motion_packet.type = CSSS_SET_CHARACTER_MOTION;
		motion_packet.object_id = hero.first;
		motion_packet.motion_type = hero.second->motion_type;
		motion_packet.anim_time_pos = hero.second->anim_time_pos;
		motion_packet.skill_type = 0;
		info_data.emplace_back(&motion_packet, motion_packet.size);
	}

	for (const auto& skill : m_skills) {
		//Send skill transform.
		csss_packet_set_obj_transform packet;
		packet.size = sizeof(csss_packet_set_obj_transform);
		packet.type = CSSS_SET_OBJ_TRANSFORM;
		packet.object_id = skill.first;
		packet.position_x = skill.second->pos.x; packet.position_y = skill.second->pos.y; packet.position_z = skill.second->pos.z;
		packet.rotation_euler_x = skill.second->rot.x; packet.rotation_euler_y = skill.second->rot.y; packet.rotation_euler_z = skill.second->rot.z;
		packet.scale_x = packet.scale_y = packet.scale_z = 1.0f;
		info_data.emplace_back(&packet, packet.size);
	}
	event_data.emplace_back(info_data.data, info_data.len);

	//WSASend
	socket_lock.lock();
	for (const SOCKET& socket : sockets) {
		send_packet(socket, event_data.data, event_data.len);
	}
	socket_lock.unlock();

	event_data.clear();
	info_data.clear();
}

void DMRoom::process_type_packet(void* packet, PACKET_TYPE type)
{
	switch (type) {
	case SSCS_TRY_MOVE_CHARACTER:
		process_try_move_character(packet);
		break;

	case SSCS_TRY_ROTATION_CHARACTER:
		process_try_rotation_character(packet);
		break;

	case SSCS_TRY_NORMAL_ATTACK:
		process_try_normal_attack(packet);
		break;

	case SSCS_TRY_USE_SKILL:
		process_try_use_skill(packet);
		break;

	case SSCS_DONE_CHARACTER_MOTION:
		process_done_character_motion(packet);
		break;

	case SSCS_ACTIVATE_ANIM_NOTIFY:
		process_activate_anim_notify(packet);
		break;

	default:
		break;
	}
}

void DMRoom::process_try_move_character(void* packet)
{
	sscs_packet_try_move_character* data = reinterpret_cast<sscs_packet_try_move_character*>(packet);
	m_heros[data->client_id]->rotate(data->MoveDirection_Yaw_angle);
	m_heros[data->client_id]->move(delta_time);
}

void DMRoom::process_try_rotation_character(void* packet)
{
	sscs_packet_try_rotation_character* data = reinterpret_cast<sscs_packet_try_rotation_character*>(packet);
	m_heros[data->client_id]->rotate(data->Yaw_angle);
}

void DMRoom::process_try_normal_attack(void* packet)
{
	sscs_packet_try_normal_attack* data = reinterpret_cast<sscs_packet_try_normal_attack*>(packet);
	m_heros[data->client_id]->change_motion((char)MOTION_TYPE::ATTACK);
}

void DMRoom::process_try_use_skill(void* packet)
{
	sscs_packet_try_use_skill* data = reinterpret_cast<sscs_packet_try_use_skill*>(packet);
	m_heros[data->client_id]->change_motion((char)MOTION_TYPE::SKILL_POSE);
}

void DMRoom::process_done_character_motion(void* packet)
{
	sscs_packet_done_character_motion* data = reinterpret_cast<sscs_packet_done_character_motion*>(packet);
	m_heros[data->client_id]->change_motion((char)MOTION_TYPE::IDLE);
}

void DMRoom::process_activate_anim_notify(void* packet)
{
	sscs_packet_activate_anim_notify* data = reinterpret_cast<sscs_packet_activate_anim_notify*>(packet);
	switch (ANIM_NOTIFY_TYPE(data->anim_notify_type)) {
	case ANIM_NOTIFY_TYPE::WARRIOR_NORMAL_ATTACK_OBJ_GEN:
	case ANIM_NOTIFY_TYPE::BERSERKER_NORMAL_ATTACK_OBJ_GEN:
	case ANIM_NOTIFY_TYPE::ASSASSIN_NORMAL_ATTACK_OBJ_GEN:
	case ANIM_NOTIFY_TYPE::PRIEST_NORMAL_ATTACK_OBJ_GEN:
		m_heros[data->client_id]->do_attack();
		break;

	case ANIM_NOTIFY_TYPE::WARRIOR_SKILL_SWORD_WAVE_OBJ_GEN:
	case ANIM_NOTIFY_TYPE::BERSERKER_SKILL_FURY_ROAR_ACT:
	case ANIM_NOTIFY_TYPE::ASSASSIN_SKILL_STEALTH_ACT:
	case ANIM_NOTIFY_TYPE::PRIEST_SKILL_HOLY_AREA_OBJ_GEN:
		m_heros[data->client_id]->do_skill();
		break;
	}
}