#include "SKILL.h"
#include "HERO.h"
#include "..\Rooms\DMRoom.h"
#include "CharacterConfig.h"

SKILL::SKILL() :
	pos(0, 0, 0),
	rot(0, 0, 0),
	dir(0, 0, 1.0f),
	AABB(pos.ToXMFloat3(), XMFLOAT3(0.5f, 0.5f, 0.5f)),
	vel(5.0f),
	duration(1.0f),
	skill_type(0),
	anim_time_pos(0),
	propensity(0),
	changed_transform(false)
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
	AABB.Center.z = pos.z;
}

/////////////////////////////////////////////////////////////////////////////////////
NORMAL_ATTACK::NORMAL_ATTACK() : 
	SKILL(),
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
	if (hero->character_state == (char)PLAYER_STATE::ACT_SEMI_INVINCIBILITY) return;

	hero->set_hp(hero->hp - damage);
	if (true == hero->is_die())
		hero->death();
	else
		hero->impact();

	destroy();
}

void NORMAL_ATTACK::collision_wall()
{
	destroy();
}

/////////////////////////////////////////////////////////////////////////////////////

HOLY_AREA::HOLY_AREA() : SKILL()
{
	duration = HOLY_AREA_DURATION;
	effect_time = HOLY_AREA_EFFECT_RATE;
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

	if (effect_time >= HOLY_AREA_EFFECT_RATE) {
		hero->set_hp(hero->hp + HOLY_AREA_HEAL_AMOUNT);
	}
}

/////////////////////////////////////////////////////////////////////////////////////

FURY_ROAR::FURY_ROAR() : SKILL()
{
	duration = FURY_ROAR_DURATION;
}

FURY_ROAR::~FURY_ROAR()
{
}


/////////////////////////////////////////////////////////////////////////////////////

STELTH::STELTH() : SKILL()
{
	duration = STELTH_DURATION;
}

STELTH::~STELTH()
{
}
