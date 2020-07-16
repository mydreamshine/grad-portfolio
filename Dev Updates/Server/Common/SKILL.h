#pragma once
#include <DirectXCollision.h>
#include "Vector3d.h"
using namespace DirectX;


class HERO;
class SKILL
{
public:
	SKILL();
	virtual ~SKILL();
	bool is_die();
	void destroy();
	virtual void update(float elapsedTime);
	virtual void effect(HERO* hero);
	virtual void collision_wall();

	Vector3d pos;
	Vector3d rot;
	Vector3d dir;
	bool changed_transform;
	DirectX::BoundingBox AABB;
	float vel;
	float duration;

	char propensity;
	char skill_type;
	float anim_time_pos;
private:
	void set_aabb();
};

class NORMAL_ATTACK : public SKILL 
{
public:
	NORMAL_ATTACK();
	virtual ~NORMAL_ATTACK();
	virtual void update(float elapsedTime);
	virtual void effect(HERO* hero);
	virtual void collision_wall();
	int damage{ 10 };
};

class HOLY_AREA : public SKILL
{
public:
	HOLY_AREA();
	virtual ~HOLY_AREA();
	virtual void update(float elapsedTime);
	virtual void effect(HERO* hero);

private:
	float effect_time;
};

class FURY_ROAR : public SKILL
{
public:
	FURY_ROAR();
	virtual ~FURY_ROAR();
};