#include "DMRoom.h"
#include "CLIENT.h"
#include "CharacterConfig.h"

DMRoom::DMRoom() :
	max_player(2),
	player_num(0),
	delta_time(0),
	left_time(GAME_TIME_LIMIT),
	skill_uid(4),
	kill_count(),
	game_end(false),
	total_score(0),
	win_team(0),
	m_walls( BBManager::instance().world_bb ),
	m_grass(BBManager::instance().grass_bb),
	poison_gas(this)
{
	packet_vector.clear();
	packets.clear();
	event_data.clear();
	info_data.clear();
	for (int i = 0; i < max_player; ++i) {
		m_heros[i] = nullptr;
		sockets[i] = INVALID_SOCKET;
	}
}

DMRoom::~DMRoom()
{
}

void DMRoom::init()
{
	//Insert changed scene packet.
	csss_packet_change_scene packet{ (char)SCENE_TYPE::PLAYGMAE_SCENE };
	event_data.emplace_back(&packet, packet.size);
	csss_packet_set_game_playtime_limit limit_packet{ (unsigned int) left_time };
	event_data.emplace_back(&limit_packet, limit_packet.size);
	csss_packet_update_poison_fog_deact_area gas_packet{
		SAFE_AREA_LEFT,
		SAFE_AREA_TOP,
		SAFE_AREA_RIGHT,
		SAFE_AREA_BOTTOM
	};
	event_data.emplace_back(&gas_packet, gas_packet.size);
}

bool DMRoom::regist(int uid, SOCKET client, void* buffer)
{
	sscs_packet_try_match_login* packet = reinterpret_cast<sscs_packet_try_match_login*>(buffer);

	socket_lock.lock();

	short cur_player = player_num++;
	uids[cur_player] = uid;
	sockets[cur_player] = client;
	sockets_index[client] = cur_player;
	csss_packet_login_ok login_ok{ cur_player };
	login_ok.type = CSSS_MATCH_LOGIN_OK;
	send_packet(client, &login_ok, login_ok.size);

	//Spawn HERO, Scoreboard and reserve it to event.
	m_heros[cur_player] = spawn_hero(cur_player, packet->selected_character, cur_player % 2);
	char selected_character = (packet->selected_character == (char)CHARACTER_TYPE::NON) ? (char)CHARACTER_TYPE::WARRIOR : packet->selected_character;
	m_score.emplace(std::make_pair(cur_player, packet->nickname));
	csss_packet_spawn_player spawn_player{ cur_player, packet->nickname, (int)wcslen(packet->nickname), selected_character,
		1.0f, 1.0f, 1.0f,
		m_heros[cur_player]->rot.x, m_heros[cur_player]->rot.y, m_heros[cur_player]->rot.z,
		m_heros[cur_player]->pos.x, m_heros[cur_player]->pos.y, m_heros[cur_player]->pos.z,
		m_heros[cur_player]->propensity };
	event_data.emplace_back(&spawn_player, spawn_player.size);

	socket_lock.unlock();
	return (cur_player+1) == max_player;
}

void DMRoom::disconnect(SOCKET client)
{
	socket_lock.lock();
	--player_num;
	short disconnect_player = sockets_index[client];
	sockets_index.erase(client);
	sockets.erase(disconnect_player);
	socket_lock.unlock();
}

void DMRoom::start()
{

}

void DMRoom::end()
{
	//DB update. Change Scene.
	db_manager.init();

	socket_lock.lock();
	for (auto& player : uids) {
		int rank = db_manager.get_rank(player.second);
		int bonus = 0;
		if(total_score != 0)
			bonus = 10 * ((float)(m_score[player.first].totalscore_damage + m_score[player.first].totalscore_heal) / (float)(total_score) );

		//Draw.
		if (win_team == 2)
			rank += bonus;
		else if (m_heros[player.first]->propensity == win_team)
			rank += (20 + bonus);
		else
			rank += (-20 + bonus);
		db_manager.update_rank(player.second, rank);

		//Send score to clients.
		if (sockets.count(player.first) != 0) {
			csss_packet_change_scene change_scene_packet{ (char)SCENE_TYPE::GAMEOVER_SCENE };
			csss_packet_send_match_statistic stat_packet{
				m_score[player.first].user_name,
				(int)wcslen(m_score[player.first].user_name),
				rank,
				m_score[player.first].count_kill,
				m_score[player.first].count_death,
				m_score[player.first].count_assistance,
				m_score[player.first].totalscore_damage,
				m_score[player.first].totalscore_heal,
				m_heros[player.first]->character_type
			};

			send_packet(sockets[player.first], &change_scene_packet, change_scene_packet.size);
			send_packet(sockets[player.first], &stat_packet, change_scene_packet.size);
		}
	}

	sockets.clear();
	socket_lock.unlock();

	for (auto& hero : m_heros)
		delete hero.second;
	m_heros.clear();
	m_score.clear();

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
	current_update_time = std::chrono::high_resolution_clock::now();
	delta_time = std::chrono::duration<float>(current_update_time - last_update_time).count();
	last_update_time = current_update_time;
    process_packet_vector();
    bool retval = game_logic();
    send_game_state();
    return retval;
}

HERO* DMRoom::spawn_hero(short object_id, char character_type, char propensity)
{
	HERO* new_hero = nullptr;
	switch (character_type)
	{
	case (char)CHARACTER_TYPE::WARRIOR:
		new_hero = new WARRIOR{this, object_id, propensity};
		break;

	case (char)CHARACTER_TYPE::PRIEST:
		new_hero = new PRIEST{ this, object_id, propensity };
		break;

	case (char)CHARACTER_TYPE::ASSASSIN:
		new_hero = new ASSASSIN{ this, object_id, propensity };
		break;

	case (char)CHARACTER_TYPE::BERSERKER:
		new_hero = new BERSERKER{ this, object_id, propensity };
		break;

	default:
		new_hero = new WARRIOR{ this, object_id, propensity };
		break;
	}
	return new_hero;
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
		packet_inheritance* dp = reinterpret_cast<packet_inheritance*>(packet_pos);
		packet_size = dp->size;
		process_type_packet(packet_pos, dp->type);
		packet_length -= packet_size;
		packet_pos += packet_size;
	}
}

bool DMRoom::game_logic()
{
	left_time -= delta_time;

	for (auto& hero : m_heros)
		hero.second->update(delta_time);
	for (auto& skill : m_skills)
		skill.second->update(delta_time);
	poison_gas.update(delta_time);

	//Hero and Gas.
	for (auto& hero : m_heros)
		poison_gas.effect(hero.second);

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

	//Hero and Grass.
	//Check hiding.
	bool is_bush{ false };
	for (auto& hero : m_heros) {
		is_bush = false;
		for (auto& grass : m_grass) {
			if (true == hero.second->AABB.Intersects(grass)) {
				hero.second->hide();
				is_bush = true;
				break;
			}
		}
		if (false == is_bush)
			hero.second->unhide();
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

	game_end = end_check();
	return game_end;
}

void DMRoom::send_game_state()
{
	if (false == game_end) {
		for (const auto& hero : m_heros) {
			if (false == hero.second->changed_transform) continue;
			//Send hero transform.
			csss_packet_set_obj_transform transform_packet{
				hero.first,
				1.0f, 1.0f, 1.0f,
				hero.second->rot.x, hero.second->rot.y, hero.second->rot.z,
				hero.second->pos.x, hero.second->pos.y, hero.second->pos.z
			};
			info_data.emplace_back(&transform_packet, transform_packet.size);
			hero.second->changed_transform = false;
		}

		for (const auto& skill : m_skills) {
			if (false == skill.second->changed_transform) continue;

			//Send skill transform.
			csss_packet_set_obj_transform transform_packet{
				skill.first,
				1.0f, 1.0f, 1.0f,
				skill.second->rot.x, skill.second->rot.y, skill.second->rot.z,
				skill.second->pos.x, skill.second->pos.y, skill.second->pos.z
			};
			info_data.emplace_back(&transform_packet, transform_packet.size);
			skill.second->changed_transform = false;
		}
		event_data.emplace_back(info_data.data, info_data.len);
	}

	//Send data to clients.
	socket_lock.lock();
	for (const auto& socket : sockets) {
		send_packet(socket.second, event_data.data, event_data.len);
	}
	socket_lock.unlock();

	event_data.clear();
	info_data.clear();
}

bool DMRoom::end_check()
{
	for (int i = 0; i < 2; ++i)
		if (kill_count[i] >= WIN_GOAL) {
			win_team = i;

			total_score = 0;
			for (auto& score : m_score) {
				total_score += score.second.totalscore_damage;
				total_score += score.second.totalscore_heal;
			}

			return true;
		}

	if (left_time <= 0) {
		if (kill_count[0] == kill_count[1])
			win_team = 2;
		else if (kill_count[0] > kill_count[1])
			win_team = 0;
		else
			win_team = 1;

		total_score = 0;
		for (auto& score : m_score) {
			total_score += score.second.totalscore_damage;
			total_score += score.second.totalscore_heal;
		}

		return true;
	}
		

	return false;
}

void DMRoom::process_type_packet(void* packet, PACKET_TYPE type)
{
	switch (type) {
	case SSCS_TRY_MOVE_CHARACTER:
		process_try_move_character(packet);
		break;

	case SSCS_TRY_MOVE_STOP_CHARACTER:
		process_try_move_stop_character(packet);
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
	if (m_heros[data->client_id]->motion_type == (char)MOTION_TYPE::IDLE
		|| m_heros[data->client_id]->motion_type == (char)MOTION_TYPE::WALK) {
		m_heros[data->client_id]->change_motion((char)MOTION_TYPE::WALK);
		m_heros[data->client_id]->rotate(data->MoveDirection_Yaw_angle);
		m_heros[data->client_id]->move(delta_time);
	}
}

void DMRoom::process_try_move_stop_character(void* packet)
{
	sscs_packet_try_movestop_character* data = reinterpret_cast<sscs_packet_try_movestop_character*>(packet);
	if(m_heros[data->client_id]->motion_type == (char)MOTION_TYPE::WALK)
		m_heros[data->client_id]->change_motion((char)MOTION_TYPE::IDLE);
}

void DMRoom::process_try_normal_attack(void* packet)
{
	sscs_packet_try_normal_attack* data = reinterpret_cast<sscs_packet_try_normal_attack*>(packet);
	m_heros[data->client_id]->rotate(data->character_yaw_angle);
	m_heros[data->client_id]->change_motion((char)MOTION_TYPE::ATTACK);
	if (m_heros[data->client_id]->character_state == (char)PLAYER_STATE::ACT_STEALTH)
		m_heros[data->client_id]->change_state((char)PLAYER_STATE::NON);
}

void DMRoom::process_try_use_skill(void* packet)
{
	sscs_packet_try_use_skill* data = reinterpret_cast<sscs_packet_try_use_skill*>(packet);
	m_heros[data->client_id]->rotate(data->character_yaw_angle);
	m_heros[data->client_id]->change_motion((char)MOTION_TYPE::SKILL_POSE);
	if (m_heros[data->client_id]->character_state == (char)PLAYER_STATE::ACT_STEALTH)
		m_heros[data->client_id]->change_state((char)PLAYER_STATE::NON);
}

void DMRoom::process_done_character_motion(void* packet)
{
	sscs_packet_done_character_motion* data = reinterpret_cast<sscs_packet_done_character_motion*>(packet);
	if(m_heros[data->client_id]->motion_type != (char)MOTION_TYPE::DIEING)
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

void DMRoom::update_score_packet(short obj_id)
{
	csss_packet_set_kda_score packet{
		m_score[obj_id].count_kill,
		m_score[obj_id].count_death,
		m_score[obj_id].count_assistance
	};

	socket_lock.lock();
	if (sockets[obj_id] != INVALID_SOCKET)
		send_packet(sockets[obj_id], &packet, packet.size);
	socket_lock.unlock();
}

void DMRoom::update_score_damage(short obj_id, int damage)
{
	m_score[obj_id].totalscore_damage += damage;
}

void DMRoom::update_score_heal(short obj_id, int heal)
{
	m_score[obj_id].totalscore_heal += heal;
}

void DMRoom::update_score_kill(short obj_id)
{
	m_score[obj_id].count_kill += 1;
	update_score_packet(obj_id);
}

void DMRoom::update_score_death(short obj_id)
{
	m_score[obj_id].count_death += 1;
	update_score_packet(obj_id);
}

void DMRoom::update_score_assist(short obj_id)
{
	m_score[obj_id].count_assistance += 1;
	update_score_packet(obj_id);
}
