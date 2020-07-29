#include "HERO.h"
#include "SKILL.h"
#include "..\Rooms\DMRoom.h"
#include "CharacterConfig.h"
#include "..\BattleServer\BattleServer\BBManager.h"

HERO::HERO(DMRoom* world, short object_id, char propensity) :
	world(world),
	pos(0, 0, 0),
	rot(0, 0, 0),
	dir(0, 0, 1.0f),
	origin_AABB(pos.ToXMFloat3(), XMFLOAT3(0.5f, 0.5f, 0.5f)),
	AABB(pos.ToXMFloat3(), XMFLOAT3(0.5f, 0.5f, 0.5f)),
	vel(BASIC_VELOCITY),

	max_hp(BASIC_HP),
	hp(BASIC_HP),
	motion_type((char)MOTION_TYPE::IDLE),
	character_type((char)CHARACTER_TYPE::NON),
	character_state((char)PLAYER_STATE::NON),
	anim_time_pos(0),
	remain_state_time(0),
	propensity(propensity),
	object_id(object_id),
	changed_transform(false),
	motion_speed(1.0f)
{
}

HERO::~HERO() 
{
}

void HERO::rotate(float yaw)
{
	rot.y = yaw;
	dir = Vector3d(0, 0, 1.0f).rotY(-yaw);
	changed_transform = true;
}

void HERO::move(float elapsedTime)
{
	pos += dir * vel * elapsedTime;
	set_aabb();
	changed_transform = true;
}

void HERO::correct_position(DirectX::BoundingBox& other)
{
	Vector3d offset = get_offset(other);
	pos += offset;
	set_aabb();
	changed_transform = true;
}

void HERO::change_motion(char motion, bool reset)
{
	if (motion == this->motion_type && false == reset)
		return;

	this->motion_type = motion;
	anim_time_pos = 0.0f;

	csss_packet_set_character_motion packet{ object_id, motion_type, 0, motion_speed };
	world->event_data.emplace_back(&packet, packet.size);
}

void HERO::change_state(char state)
{
	if (state == this->character_state)
		return;

	this->character_state = state;

	csss_pacekt_set_character_state packet{ object_id, character_state };
	world->event_data.emplace_back(&packet, packet.size);
}

bool HERO::is_die()
{
	return hp <= 0;
}

int HERO::set_hp(int hp)
{
	this->hp = (hp < 0) ? 0 : hp;
	this->hp = (hp > max_hp) ? max_hp : hp;

	csss_packet_set_character_hp packet{ object_id, this->hp };
	world->event_data.emplace_back(&packet, packet.size);

	return hp - this->hp;
}

void HERO::impact()
{
	change_state((char)PLAYER_STATE::ACT_SEMI_INVINCIBILITY);
	remain_state_time = SEMI_INVINCIBLE_TIME;
}

void HERO::death()
{
	change_motion((char)MOTION_TYPE::DIEING);
	change_state((char)PLAYER_STATE::ACT_DIE);
	remain_state_time = RESPAWN_TIME;
}

void HERO::respawn()
{
	set_hp(max_hp);
	change_motion((char)MOTION_TYPE::IDLE);
	change_state((char)PLAYER_STATE::NON);
}

void HERO::update(float elapsedTime)
{
	remain_state_time -= elapsedTime;

	switch (character_state) {
	case (char)PLAYER_STATE::ACT_SEMI_INVINCIBILITY:
		if (remain_state_time <= 0)
			change_state((char)PLAYER_STATE::NON);
		break;

	case (char)PLAYER_STATE::ACT_DIE:
		if (remain_state_time <= 0)
			respawn();
		break;
	}

	anim_time_pos += elapsedTime;
	set_aabb();
}

void HERO::do_attack()
{
	int object_id = world->skill_uid++;
	SKILL* normal_attack = new NORMAL_ATTACK{world, (short)object_id};
	normal_attack->pos = pos;
	normal_attack->pos.y += 50;
	normal_attack->rot = rot;
	normal_attack->dir = dir;
	normal_attack->propensity = propensity;
	world->m_skills[object_id] = normal_attack;

	csss_packet_spawn_normal_attack_obj packet{
		object_id, character_type, 
		1.0f, 1.0f, 1.0f, 
		rot.x, rot.y, rot.z, 
		pos.x, pos.y, pos.z, 
		propensity
	};
	world->event_data.emplace_back(&packet, packet.size);
}

void HERO::hide()
{
	if (motion_type != (char)MOTION_TYPE::WALK
		&& motion_type != (char)MOTION_TYPE::IDLE)
		return;

	if (character_state == (char)PLAYER_STATE::NON)
		change_state((char)PLAYER_STATE::ACT_STEALTH);
}

void HERO::unhide()
{
	if (character_state == (char)PLAYER_STATE::ACT_STEALTH)
		change_state((char)PLAYER_STATE::NON);
}

void HERO::set_aabb()
{
	AABB.Center.x = origin_AABB.Center.x + pos.x;
	AABB.Center.z = origin_AABB.Center.z + pos.z;
}

Vector3d HERO::get_offset(DirectX::BoundingBox& other)
{
		Vector3d A_min = { AABB.Center.x - AABB.Extents.x, AABB.Center.y - AABB.Extents.y, AABB.Center.z - AABB.Extents.z };
		Vector3d A_max = { AABB.Center.x + AABB.Extents.x, AABB.Center.y + AABB.Extents.y, AABB.Center.z + AABB.Extents.z };
		Vector3d B_min = { other.Center.x - other.Extents.x, other.Center.y - other.Extents.y, other.Center.z - other.Extents.z };
		Vector3d B_max = { other.Center.x + other.Extents.x, other.Center.y + other.Extents.y, other.Center.z + other.Extents.z };
		Vector3d Offset = { 0.0f, 0.0f, 0.0f };

		RECT intersect;
		RECT A_rect = { (LONG)A_min.x, (LONG)A_min.z, (LONG)A_max.x, (LONG)A_max.z };
		RECT B_rect = { (LONG)B_min.x, (LONG)B_min.z, (LONG)B_max.x, (LONG)B_max.z };
		if (::IntersectRect(&intersect, &A_rect, &B_rect))
		{
			int nInterW = intersect.right - intersect.left;
			int nInterH = intersect.bottom - intersect.top;

			// 위/아래 체크
			if (nInterW > nInterH)
			{
				// 위에서 충돌
				if (intersect.top == B_rect.top)
					Offset.z = -(float)nInterH;
				// 아래서 충돌
				else if (intersect.bottom == B_rect.bottom)
					Offset.z = +(float)nInterH;
			}
			// 좌/우 체크
			else
			{
				if (intersect.left == B_rect.left)
					Offset.x = -(float)nInterW;
				else if (intersect.right == B_rect.right)
					Offset.x = +(float)nInterW;
			}
		}

		return Offset;
}

////////////////////////////////////////////////////////////

WARRIOR::WARRIOR(DMRoom* world, short object_id, char propensity) :
	HERO(world, object_id, propensity)
{
	character_type = ((char)CHARACTER_TYPE::WARRIOR);
	origin_AABB = AABB = BBManager::instance().character_bb[(char)CHARACTER_TYPE::WARRIOR];
	set_aabb();
	max_hp = hp = WARRIOR_MAX_HP;
	vel = WARRIOR_MOVEMENT;
}

WARRIOR::~WARRIOR()
{
}

void WARRIOR::do_attack()
{
	int object_id = world->skill_uid++;
	auto normal_attack = new NORMAL_ATTACK{ world, (short)object_id };
	normal_attack->pos = pos;
	normal_attack->pos.y += 50;
	normal_attack->rot = rot;
	normal_attack->dir = dir;
	normal_attack->propensity = propensity;
	normal_attack->damage = WARRIOR_ATTACK_DAMAGE;
	normal_attack->set_aabb(163.0f, 10.0f, 100.0f);
	world->m_skills[object_id] = normal_attack;

	csss_packet_spawn_normal_attack_obj packet{
		object_id, character_type,
		1.0f, 1.0f, 1.0f,
		rot.x, rot.y, rot.z,
		pos.x, pos.y, pos.z,
		propensity
	};
	world->event_data.emplace_back(&packet, packet.size);
}

void WARRIOR::do_skill()
{
	static float Deg[3]{ -30.0f, 0.0f, 30.0f };
	int skill_id = 0;
	NORMAL_ATTACK* normal_attack;
	csss_packet_spawn_skill_obj packet{
		skill_id,
		(char)SKILL_TYPE::SWORD_WAVE,
		1.0f, 1.0f, 1.0f,
		rot.x, rot.y, rot.z,
		pos.x, pos.y, pos.z,
		propensity 
	};

	for (int i = 0; i < 3; ++i) {
		skill_id = world->skill_uid++;
		normal_attack = new NORMAL_ATTACK{ world, (short)object_id };
		normal_attack->pos = pos;
		normal_attack->pos.y += 50;
		normal_attack->rot = rot; normal_attack->rot.y -= Deg[i];
		normal_attack->set_aabb(163.0f, 10.0f, 100.0f);
		normal_attack->dir = dir.rotY(Deg[i]);
		normal_attack->propensity = propensity;
		normal_attack->damage = SWORD_WAVE_DAMAGE;
		normal_attack->duration = SWORD_WAVE_DURATION;
		world->m_skills[skill_id] = normal_attack;

		packet.object_id = skill_id;
		packet.position_x = normal_attack->pos.x; packet.position_y = normal_attack->pos.y; packet.position_z = normal_attack->pos.z;
		packet.rotation_euler_x = normal_attack->rot.x; packet.rotation_euler_y = normal_attack->rot.y; packet.rotation_euler_z = normal_attack->rot.z;
		world->event_data.emplace_back(&packet, packet.size);
	}
}

////////////////////////////////////////////////////////////

PRIEST::PRIEST(DMRoom* world, short object_id, char propensity) :
	HERO(world, object_id, propensity)
{
	character_type = ((char)CHARACTER_TYPE::PRIEST);
	origin_AABB = AABB = BBManager::instance().character_bb[(char)CHARACTER_TYPE::PRIEST];
	set_aabb();
	max_hp = hp = PRIEST_MAX_HP;
	vel = PRIEST_MOVEMENT;
}

PRIEST::~PRIEST()
{
}

void PRIEST::do_attack()
{
	int object_id = world->skill_uid++;
	auto normal_attack = new NORMAL_ATTACK{ world, (short)object_id };
	normal_attack->pos = pos;
	normal_attack->pos.y += 50;
	normal_attack->rot = rot;
	normal_attack->dir = dir;
	normal_attack->damage = PRIEST_ATTACK_DAMAGE;
	normal_attack->propensity = propensity;
	normal_attack->set_aabb(50.0f, 10.0f, 163.0f);
	world->m_skills[object_id] = normal_attack;

	csss_packet_spawn_normal_attack_obj packet{
		object_id, character_type,
		1.0f, 1.0f, 1.0f,
		rot.x, rot.y, rot.z,
		pos.x, pos.y, pos.z,
		propensity
	};
	world->event_data.emplace_back(&packet, packet.size);
}

void PRIEST::do_skill()
{
	int skill_id = world->skill_uid++;
	HOLY_AREA* holy_area = new HOLY_AREA{ world, (short)object_id };

	holy_area->pos = pos;
	holy_area->propensity = propensity;
	world->m_skills[skill_id] = holy_area;

	csss_packet_spawn_skill_obj packet{
		skill_id,
		(char)SKILL_TYPE::HOLY_AREA,
		1.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 0.0f,
		pos.x, pos.y, pos.z,
		propensity
	};
	world->event_data.emplace_back(&packet, packet.size);
}

////////////////////////////////////////////////////////////

BERSERKER::BERSERKER(DMRoom* world, short object_id, char propensity) :
	HERO(world, object_id, propensity),
	roar_mode(false),
	roar_time(0.0f)
{
	character_type = ((char)CHARACTER_TYPE::BERSERKER);
	origin_AABB = AABB = BBManager::instance().character_bb[(char)CHARACTER_TYPE::BERSERKER];
	set_aabb();
	max_hp = hp = BERSERKER_MAX_HP;
	vel = BERSERKER_MOVEMENT;
}

BERSERKER::~BERSERKER()
{
}

void BERSERKER::do_attack()
{
	int object_id = world->skill_uid++;
	auto normal_attack = new NORMAL_ATTACK{ world, (short)object_id };
	normal_attack->pos = pos;
	normal_attack->pos.y += 50;
	normal_attack->rot = rot;
	normal_attack->dir = dir;
	normal_attack->propensity = propensity;
	normal_attack->set_aabb(163.0f, 10.0f, 100.0f);
	normal_attack->damage = BERSERKER_ATTACK_DAMAGE;
	world->m_skills[object_id] = normal_attack;

	csss_packet_spawn_normal_attack_obj packet{
		object_id, character_type,
		1.0f, 1.0f, 1.0f,
		rot.x, rot.y, rot.z,
		pos.x, pos.y, pos.z,
		propensity
	};
	world->event_data.emplace_back(&packet, packet.size);
}

void BERSERKER::do_skill()
{
	roar_mode = true;
	roar_time = FURY_ROAR_DURATION;
	motion_speed = FURY_ROAR_ACCELERATE;

	int skill_id = world->skill_uid++;
	auto roar_skill = new FURY_ROAR{ world, (short)object_id };

	roar_skill->pos = pos;
	roar_skill->propensity = propensity;
	world->m_skills[skill_id] = roar_skill;

	csss_packet_spawn_skill_obj packet{
		skill_id,
		(char)SKILL_TYPE::FURY_ROAR,
		1.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 0.0f,
		pos.x, pos.y, pos.z,
		propensity
	};
	world->event_data.emplace_back(&packet, packet.size);
}

void BERSERKER::move(float elapsedTime)
{
	if (true == roar_mode)
		HERO::move(FURY_ROAR_ACCELERATE * elapsedTime);
	else
		HERO::move(elapsedTime);
}

void BERSERKER::death()
{
	if (true == roar_mode) {
		motion_speed = 1.0f;
		roar_mode = false;
	}
	HERO::death();
}

void BERSERKER::update(float elapsedTime)
{
	if (true == roar_mode) {
		roar_time -= elapsedTime;
		if (roar_time <= 0.0f) {
			roar_mode = false;
			motion_speed = 1.0f;
			change_motion(motion_type, true);
		}
	}
	HERO::update(elapsedTime);
}

////////////////////////////////////////////////////////////

ASSASSIN::ASSASSIN(DMRoom* world, short object_id, char propensity) :
	HERO(world, object_id, propensity),
	stelth_mode(false),
	stelth_time(0.0f)
{
	character_type = (char)CHARACTER_TYPE::ASSASSIN;
	origin_AABB = AABB = BBManager::instance().character_bb[(char)CHARACTER_TYPE::ASSASSIN];
	set_aabb();
	max_hp = hp = ASSASSIN_MAX_HP;
	vel = ASSASSIN_MOVEMENT;
}

ASSASSIN::~ASSASSIN()
{
}

void ASSASSIN::do_skill()
{
	stelth_mode = true;
	stelth_time = STELTH_DURATION;
	
	change_state((char)PLAYER_STATE::ACT_STEALTH);

	int skill_id = world->skill_uid++;
	auto stelth_skill = new STELTH{ world, (short)object_id };
	
	stelth_skill->pos = pos;
	stelth_skill->propensity = propensity;
	world->m_skills[skill_id] = stelth_skill;

	csss_packet_spawn_skill_obj packet{
		skill_id,
		(char)SKILL_TYPE::STEALTH,
		1.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 0.0f,
		pos.x, pos.y, pos.z,
		propensity
	};
	world->event_data.emplace_back(&packet, packet.size);
}

void ASSASSIN::death()
{
	HERO::death();
}

void ASSASSIN::update(float elapsedTime)
{
	if (true == stelth_mode) {
		stelth_time -= elapsedTime;
		if (stelth_time <= 0.0f) {
			change_state((char)PLAYER_STATE::NON);
			stelth_mode = false;
		}
	}
	HERO::update(elapsedTime);
}

void ASSASSIN::impact()
{
	if (true == stelth_mode)
		stelth_mode = false;
	HERO::impact();
}

void ASSASSIN::do_attack()
{
	if (true == stelth_mode)
		stelth_mode = false;

	int object_id = world->skill_uid++;
	auto normal_attack = new NORMAL_ATTACK{ world, (short)object_id };
	normal_attack->pos = pos;
	normal_attack->pos.y += 50;
	normal_attack->rot = rot;
	normal_attack->dir = dir;
	normal_attack->propensity = propensity;
	normal_attack->set_aabb(25.0f, 10.0f, 163.0f);
	normal_attack->damage = ASSASSIN_ATTACK_DAMAGE;
	world->m_skills[object_id] = normal_attack;

	csss_packet_spawn_normal_attack_obj packet{
		object_id, character_type,
		1.0f, 1.0f, 1.0f,
		rot.x, rot.y, rot.z,
		pos.x, pos.y, pos.z,
		propensity
	};
	world->event_data.emplace_back(&packet, packet.size);
}

void ASSASSIN::unhide()
{
	if (true == stelth_mode)
		return;
	HERO::unhide();
}
