#include "DMRoom.h"
#include "CLIENT.h"

DMRoom::DMRoom() :
	max_player(4),
	player_num(0)
{
	packet_lock.lock();
	packet_vector.clear();
	packets.clear();
	packet_lock.unlock();

	event_data.clear();
	info_data.clear();
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
    process_packet_vector();
    bool retval = game_logic(elapsedTime);
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
	int packet_size = 0;
	int packet_length = packets.len;
	while (packet_length > 0)
	{
		default_packet* dp = reinterpret_cast<default_packet*>(packet_pos);
		packet_size = dp->size;
		process_type_packet(packet_pos, dp->type);
		packet_length -= packet_size;
		packet_pos += packet_size;
	}
}

bool DMRoom::game_logic(float elapsedTime)
{
	for (auto& hero : m_heros)
		hero.second->update(elapsedTime);
	for (auto& skill : m_skills)
		skill.second->update(elapsedTime);

	//Collision check and process.
	for (auto& hero : m_heros) {
		//Hero and Skill.
		for (auto& skill : m_skills) {
			if (true == hero.second->AABB.Intersects(skill.second->AABB))
				skill.second->effect(hero.second);

			//Skill and Wall
			for (auto& wall : m_walls)
				if (true == skill.second->AABB.Intersects(wall));
					//Deactivate Skill.
		}

		//Hero and Wall
		for (auto& wall : m_walls)
			if (true == hero.second->AABB.Intersects(wall));
				//Correct position.
	}
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
		//WSASend(socket, )
	}
	socket_lock.unlock();

	event_data.clear();
	info_data.clear();
}

void DMRoom::process_type_packet(void* packet, PACKET_TYPE type)
{
	switch (type) {
	case SSCS_TRY_MOVE_CHARACTER:
		break;

	case SSCS_TRY_ROTATION_CHARACTER:
		break;

	case SSCS_TRY_NORMAL_ATTACK:
		break;

	case SSCS_TRY_USE_SKILL:
		break;

	case SSCS_DONE_CHARACTER_MOTION:
		break;

	case SSCS_ACTIVATE_ANIM_NOTIFY:
		break;

	default:
		break;
	}
}
