#pragma once
#include <DirectXCollision.h>
#include "Vector3d.h"
using namespace DirectX;


class HERO;
class DMRoom;

struct RECTANGLE {
	int left, top, right, bottom;
};

class SKILL
{
public:
	SKILL(DMRoom* world, short owner_id);
	virtual ~SKILL();
	bool is_die();
	void destroy();
	virtual void update(float elapsedTime);
	virtual void effect(HERO* hero);
	virtual void collision_wall();
	void set_aabb();
	void set_aabb(float ex, float ey, float ez);

	DMRoom* world;
	short owner_id;
	Vector3d pos;
	Vector3d rot;
	Vector3d dir;
	bool changed_transform;
	DirectX::BoundingOrientedBox AABB;
	float vel;
	float duration;

	char propensity;
	char skill_type;
	float anim_time_pos;
};

class NORMAL_ATTACK : public SKILL 
{
public:
	NORMAL_ATTACK(DMRoom* world, short owner_id);
	virtual ~NORMAL_ATTACK();
	virtual void update(float elapsedTime);
	virtual void effect(HERO* hero);
	virtual void collision_wall();
	int damage{ 10 };
};

class HOLY_AREA : public SKILL
{
public:
	HOLY_AREA(DMRoom* world, short owner_id);
	virtual ~HOLY_AREA();
	virtual void update(float elapsedTime);
	virtual void effect(HERO* hero);

private:
	float effect_time;
};

class FURY_ROAR : public SKILL
{
public:
	FURY_ROAR(DMRoom* world, short owner_id);
	virtual ~FURY_ROAR();
};

class STELTH : public SKILL
{
public:
	STELTH(DMRoom* world, short owner_id);
	virtual ~STELTH();
};

class POISON_GAS {
public:
	POISON_GAS(DMRoom* world);
	void update(float elapsedTime);
	void effect(HERO* hero);
	
private:
	bool is_safe(HERO* hero);

	RECTANGLE safe_area;
	DMRoom* world;
	int cur_state;
	int max_state;
	float effect_time;
	float decrease_time;
	float GAS_DECREASE_TIME;
	int GAS_DECREASE_WIDTH;
	int GAS_DECREASE_HEIGHT;
};