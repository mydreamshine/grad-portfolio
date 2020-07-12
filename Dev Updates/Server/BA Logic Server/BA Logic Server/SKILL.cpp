#include "SKILL.h"
#include "HERO.h"

SKILL::SKILL() :
	pos(0, 0, 0),
	rot(0, 0, 0),
	dir(0, 0, -1.0f),
	AABB(pos.ToXMFloat3(), XMFLOAT3(0.5f, 0.5f, 0.5f)),
	vel(5.0f),

	skill_type(0),
	anim_time_pos(0)
{
}

SKILL::~SKILL()
{
}

void SKILL::update(float elapsedTime)
{
	anim_time_pos += elapsedTime;
	set_aabb();
}

void SKILL::effect(HERO* hero)
{
}

void SKILL::set_aabb()
{
	AABB.Center = pos.ToXMFloat3();
}

/////////////////////////////////////////////////////////////////////////////////////
NORMAL_ATTACK::NORMAL_ATTACK() : 
	SKILL(),
	damage(10)
{
}

NORMAL_ATTACK::~NORMAL_ATTACK()
{
}

void NORMAL_ATTACK::update(float elapsedTime)
{
	pos += dir * vel * elapsedTime;
	SKILL::update(elapsedTime);
}

void NORMAL_ATTACK::effect(HERO* hero)
{
	hero->hp -= damage;
}

