#include "HERO.h"
#include "SKILL.h"
#include "..\Rooms\DMRoom.h"
#include "CharacterConfig.h"

HERO::HERO(DMRoom* world, short object_id, char propensity) :
	world(world),
	pos(0, 0, 0),
	rot(0, 0, 0),
	dir(0, 0, 1.0f),
	AABB(pos.ToXMFloat3(), XMFLOAT3(0.5f, 0.5f, 0.5f)),
	vel(5.0f),

	hp(100),
	motion_type((char)MOTION_TYPE::IDLE),
	character_type((char)CHARACTER_TYPE::NON),
	character_state((char)PLAYER_STATE::NON),
	anim_time_pos(0),
	remain_state_time(0),
	propensity(propensity),
	object_id(object_id)
{
}

HERO::~HERO() 
{
}

void HERO::rotate(float yaw)
{
	rot.y += yaw;
	dir = dir.rotY(yaw);
}

void HERO::move(float elapsedTime)
{
	pos += dir * vel * elapsedTime;
	set_aabb();
}

void HERO::correct_position(DirectX::BoundingBox& other)
{
	Vector3d offset = get_offset(other);
	pos += offset;
	set_aabb();
}

void HERO::change_motion(char motion)
{
	if (motion == this->motion_type)
		return;

	this->motion_type = motion;
	anim_time_pos = 0.0f;

	csss_packet_set_character_motion packet;
	packet.type = CSSS_SET_CHARACTER_MOTION;
	packet.size = sizeof(csss_packet_set_character_motion);
	packet.motion_type = motion_type;
	packet.object_id = object_id;
	packet.anim_time_pos = anim_time_pos;
	world->event_data.emplace_back(&packet, packet.size);
}

void HERO::change_state(char state)
{
	if (state == this->character_state)
		return;

	this->character_state = state;

	csss_pacekt_set_character_state packet;
	packet.type = CSSS_SET_CHARACTER_STATE;
	packet.size = sizeof(csss_pacekt_set_character_state);
	packet.object_id = object_id;
	packet.character_state = character_state;
	world->event_data.emplace_back(&packet, packet.size);
}

bool HERO::is_die()
{
	return hp <= 0;
}

void HERO::set_hp(int hp)
{
	this->hp = (hp < 0) ? 0 : hp;

	csss_packet_set_character_hp packet;
	packet.type = CSSS_SET_CHARACTER_HP;
	packet.size = sizeof(csss_packet_set_character_hp);
	packet.object_id = object_id;
	packet.hp = this->hp;
	world->event_data.emplace_back(&packet, packet.size);
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
	set_hp(BASIC_HP);
	change_motion((char)MOTION_TYPE::IDLE);
	change_state((char)PLAYER_STATE::NON);
}

void HERO::update(float elapsedTime)
{
	remain_state_time -= elapsedTime;

	switch (character_state) {
	case (char)PLAYER_STATE::ACT_SEMI_INVINCIBILITY:
	case (char)PLAYER_STATE::ACT_STEALTH:
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
	short object_id = world->skill_uid++;
	SKILL* normal_attack = new NORMAL_ATTACK{};
	normal_attack->pos = pos;
	normal_attack->rot = rot;
	normal_attack->dir = dir;
	normal_attack->propensity = propensity;
	world->m_skills[object_id] = normal_attack;

	csss_packet_spawn_normal_attack_obj packet;

	packet.size = sizeof(csss_packet_spawn_normal_attack_obj);
	packet.type = CSSS_SPAWN_NORMAL_ATTACK_OBJ;

	packet.attack_order = character_type;
	packet.object_id = object_id;
	packet.propensity = propensity;

	packet.position_x = normal_attack->pos.x; packet.position_y = normal_attack->pos.y; packet.position_z = normal_attack->pos.z;
	packet.rotation_euler_x = normal_attack->rot.x; packet.rotation_euler_y = normal_attack->rot.y; packet.rotation_euler_z = normal_attack->rot.z;
	packet.scale_x = packet.scale_y = packet.scale_z = 1.0f;

	world->event_data.emplace_back(&packet, packet.size);
}

void HERO::set_aabb()
{
	AABB.Center.x = pos.x;
	AABB.Center.z = pos.z;
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
}

WARRIOR::~WARRIOR()
{
}

void WARRIOR::do_skill()
{
	static float Deg[3]{ -30.0f, 0.0f, 30.0f };
	short object_id;
	NORMAL_ATTACK* normal_attack;
	csss_packet_spawn_skill_obj packet;

	packet.size = sizeof(csss_packet_spawn_skill_obj);
	packet.type = CSSS_SPAWN_SKILL_OBJ;
	packet.skill_type = (char)SKILL_TYPE::SWORD_WAVE;
	packet.scale_x = packet.scale_y = packet.scale_z = 1.0f;
	
	packet.propensity = propensity;

	for (int i = 0; i < 3; ++i) {
		object_id = world->skill_uid++;
		normal_attack = new NORMAL_ATTACK{};
		normal_attack->pos = pos;
		normal_attack->rot = rot; normal_attack->rot.y += Deg[i];
		normal_attack->dir = dir.rotY(Deg[i]);
		normal_attack->propensity = propensity;
		normal_attack->damage = SWORD_WAVE_DAMAGE;
		normal_attack->duration = SWORD_WAVE_DURATION;
		world->m_skills[object_id] = normal_attack;

		packet.object_id = object_id;
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
}

PRIEST::~PRIEST()
{
}

void PRIEST::do_skill()
{
	short object_id = world->skill_uid++;
	HOLY_AREA* holy_area = new HOLY_AREA{};
	csss_packet_spawn_skill_obj packet;

	holy_area->pos = pos;
	holy_area->propensity = propensity;
	world->m_skills[object_id] = holy_area;

	packet.size = sizeof(csss_packet_spawn_skill_obj);
	packet.type = CSSS_SPAWN_SKILL_OBJ;

	packet.propensity = propensity;
	packet.skill_type = (char)SKILL_TYPE::HOLY_AREA;
	packet.object_id = object_id;
	packet.position_x = holy_area->pos.x; packet.position_y = holy_area->pos.y; packet.position_z = holy_area->pos.z;
	packet.rotation_euler_x = holy_area->rot.x; packet.rotation_euler_y = holy_area->rot.y; packet.rotation_euler_z = holy_area->rot.z;
	packet.scale_x = packet.scale_y = packet.scale_z = 1.0f;
	world->event_data.emplace_back(&packet, packet.size);
}
