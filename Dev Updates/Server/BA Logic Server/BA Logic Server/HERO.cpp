#include "HERO.h"
#include "SKILL.h"

HERO::HERO() :
	pos(0, 0, 0),
	rot(0, 0, 0),
	dir(0, 0, -1.0f),
	AABB(pos.ToXMFloat3(), XMFLOAT3(0.5f, 0.5f, 0.5f)),
	vel(5.0f),

	hp(100),
	motion_type(0),
	anim_time_pos(0)
{
}

HERO::~HERO() 
{
}

void HERO::rotate(float yaw)
{
	rot.y += yaw;
	dir.rotY(yaw);
}

void HERO::update(float elapsedTime)
{
	anim_time_pos += elapsedTime;
	set_aabb();
}

void HERO::set_aabb()
{
	AABB.Center = pos.ToXMFloat3();
}

////////////////////////////////////////////////////////////

WARRIOR::WARRIOR()
{
}

WARRIOR::~WARRIOR()
{
}

void WARRIOR::update(float elapsedTime)
{
	HERO::update(elapsedTime);
}

SKILL* WARRIOR::do_attack()
{
	SKILL* normal_attack = new NORMAL_ATTACK{};
	return normal_attack;
}

SKILL* WARRIOR::do_skill()
{
	SKILL* normal_attack = new NORMAL_ATTACK{};
	return normal_attack;
}
