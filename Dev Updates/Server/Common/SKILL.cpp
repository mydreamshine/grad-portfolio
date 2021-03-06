#include "SKILL.h"
#include "HERO.h"
#include "..\Rooms\DMRoom.h"
#include "CharacterConfig.h"

SKILL::SKILL(DMRoom *world, short owner_id) :
	pos(0, 10, 0),
	rot(0, 0, 0),
	dir(0, 0, 1.0f),
	AABB(),
	vel(5.0f),
	duration(1.0f),
	skill_type(0),
	anim_time_pos(0),
	propensity(0),
	changed_transform(false),
	world(world),
	owner_id(owner_id)
{
}

SKILL::~SKILL()
{
}

bool SKILL::is_die()
{
	return anim_time_pos >= duration;
}

void SKILL::destroy()
{
	anim_time_pos = duration;
}

void SKILL::update(float elapsedTime)
{
	anim_time_pos += elapsedTime;
	anim_time_pos = (anim_time_pos > duration) ? duration : anim_time_pos;
	set_aabb();
}

void SKILL::effect(HERO* hero)
{
}

void SKILL::collision_wall()
{
}

void SKILL::set_aabb()
{
	AABB.Center.x = pos.x;
	AABB.Center.y = pos.y;
	AABB.Center.z = pos.z;
}

void SKILL::set_aabb(float ex, float ey, float ez)
{
	set_aabb();
	AABB.Extents.x = ex;
	AABB.Extents.y = ey;
	AABB.Extents.z = ez;
	XMStoreFloat4(&AABB.Orientation, DirectX::XMQuaternionRotationRollPitchYaw(rot.x * TORAD, rot.y * TORAD, rot.z * TORAD));
}

/////////////////////////////////////////////////////////////////////////////////////
NORMAL_ATTACK::NORMAL_ATTACK(DMRoom* world, short owner_id) :
	SKILL(world, owner_id),
	damage(NORMAL_ATTACK_DAMAGE)
{
	duration = NORMAL_ATTACK_DURATION;
	vel = NORMAL_ATTACK_VELOCITY;
}

NORMAL_ATTACK::~NORMAL_ATTACK()
{
}

void NORMAL_ATTACK::update(float elapsedTime)
{
	changed_transform = true;
	pos += dir * vel * elapsedTime;
	SKILL::update(elapsedTime);
}

void NORMAL_ATTACK::effect(HERO* hero)
{
	if (hero->propensity == propensity) return;
	if (hero->character_state == (char)PLAYER_STATE::ACT_DIE) return;
	if (hero->character_state == (char)PLAYER_STATE::ACT_SEMI_INVINCIBILITY) {
		destroy();
		return;
	}

	int delta = hero->set_hp(hero->hp - damage);
	world->update_score_damage(owner_id, delta);

	if (true == hero->is_die()) {
		hero->death();
		hero->update_assister(owner_id);
		world->update_score_kill(owner_id);
		world->update_score_death(hero->object_id);
		csss_packet_send_kill_message kill_msg{ owner_id, hero->object_id };
		world->event_data.emplace_back(&kill_msg, kill_msg.size);

		csss_packet_send_in_game_team_score update_team_score{ (unsigned char)world->kill_count[propensity] };
		world->socket_lock.lock();
		for (auto& hero : world->m_heros) {
			if (hero.second->propensity == propensity && world->sockets.count(hero.second->object_id) != 0)
				world->send_packet(world->sockets[hero.second->object_id], &update_team_score, update_team_score.size);
		}
		world->socket_lock.unlock();
	}
	else {
		hero->impact();
		hero->record_attack(owner_id);
	}

	destroy();
}

void NORMAL_ATTACK::collision_wall()
{
	destroy();
}

/////////////////////////////////////////////////////////////////////////////////////

HOLY_AREA::HOLY_AREA(DMRoom* world, short owner_id) : SKILL(world, owner_id)
{
	duration = HOLY_AREA_DURATION;
	effect_time = HOLY_AREA_EFFECT_RATE;
	AABB.Extents.x = 1000.0f;
	AABB.Extents.z = 1000.0f;
	AABB.Extents.y = 50.0f;
	set_aabb();
}

HOLY_AREA::~HOLY_AREA()
{
}

void HOLY_AREA::update(float elapsedTime)
{
	if (effect_time > HOLY_AREA_EFFECT_RATE) {
		effect_time -= HOLY_AREA_EFFECT_RATE;
	}
	effect_time += elapsedTime;
	SKILL::update(elapsedTime);
}

void HOLY_AREA::effect(HERO* hero)
{
	if (hero->propensity != propensity) return;
	if (hero->character_state == (char)PLAYER_STATE::ACT_DIE) return;

	Vector3d deltaPos = hero->pos - this->pos;
	if (deltaPos.square() <= 1000000.0f) {

			if (effect_time >= HOLY_AREA_EFFECT_RATE) {
				int delta = hero->set_hp(hero->hp + HOLY_AREA_HEAL_AMOUNT);
				if (delta != 0)
					world->update_score_heal(owner_id, -delta);
			}
	}
}

/////////////////////////////////////////////////////////////////////////////////////

FURY_ROAR::FURY_ROAR(DMRoom* world, short owner_id) : SKILL(world, owner_id)
{
	duration = FURY_ROAR_DURATION;
}

FURY_ROAR::~FURY_ROAR()
{
}


/////////////////////////////////////////////////////////////////////////////////////

STELTH::STELTH(DMRoom* world, short owner_id) : SKILL(world, owner_id)
{
	duration = STELTH_DURATION;
}

STELTH::~STELTH()
{
}

POISON_GAS::POISON_GAS(DMRoom* world) :
	world(world),
	safe_area{ SAFE_AREA_LEFT, SAFE_AREA_TOP, SAFE_AREA_RIGHT, SAFE_AREA_BOTTOM },
	effect_time (0),
	decrease_time (0),
	cur_state(0),
	max_state(BBManager::instance().play_time / BBManager::instance().gas_decrease_time),
	GAS_DECREASE_TIME(BBManager::instance().gas_decrease_time),
	GAS_DECREASE_WIDTH(BBManager::instance().gas_decrease_width),
	GAS_DECREASE_HEIGHT(BBManager::instance().gas_decrease_height)
{
}

void POISON_GAS::update(float elapsedTime)
{
	if (effect_time > GAS_EFFECT_TIME) {
		effect_time -= GAS_EFFECT_TIME;
	}
	effect_time += elapsedTime;

	if (decrease_time > GAS_DECREASE_TIME) {
		decrease_time -= GAS_DECREASE_TIME;
		if (cur_state++ < max_state) {
            safe_area.left += GAS_DECREASE_WIDTH;
            safe_area.right -= GAS_DECREASE_WIDTH;
            safe_area.top -= GAS_DECREASE_HEIGHT;
            safe_area.bottom += GAS_DECREASE_HEIGHT;

			if (safe_area.left > safe_area.right) {
				safe_area.left = (safe_area.left + safe_area.right) / 2;
				safe_area.right = safe_area.left;
			}
			if (safe_area.bottom > safe_area.top) {
				safe_area.bottom = (safe_area.bottom + safe_area.top) / 2;
				safe_area.top = safe_area.bottom;
			}

			csss_packet_update_poison_fog_deact_area packet{
				safe_area.left,
				safe_area.top,
				safe_area.right,
				safe_area.bottom
			};
			world->event_data.emplace_back(&packet, packet.size);
		}
	}
	decrease_time += elapsedTime;
}

void POISON_GAS::effect(HERO* hero)
{
	if (hero->character_state == (char)PLAYER_STATE::ACT_DIE) return;

	if (effect_time >= GAS_EFFECT_TIME) {
		if (true == is_safe(hero)) return;

		int left_hp = hero->hp - GAS_DAMAGE;
		if (left_hp <= 0)
			left_hp = 1;
		hero->set_hp(left_hp);
	}
}

bool POISON_GAS::is_safe(HERO* hero)
{
	if (hero->pos.x < safe_area.left) return false;
	if (hero->pos.x > safe_area.right) return false;
	if (hero->pos.z > safe_area.top) return false;
	if (hero->pos.z < safe_area.bottom) return false;
	return true;
}
