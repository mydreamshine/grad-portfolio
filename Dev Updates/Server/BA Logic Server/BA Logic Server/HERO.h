#pragma once
#include "Vector3d.h"
#include <DirectXCollision.h>
using namespace DirectX;

class SKILL;

class HERO
{
public:
	HERO();
	virtual ~HERO();

	void rotate(float yaw);
	virtual void update(float elapsedTime);
	virtual SKILL* do_attack() {};
	virtual SKILL* do_skill() {};
	

	Vector3d pos;
	Vector3d rot;
	Vector3d dir;
	DirectX::BoundingBox AABB;
	float vel;

	char propensity;

	int hp;
	char motion_type;
	float anim_time_pos;

private:
	void set_aabb();
};

class WARRIOR : public HERO
{
public:
	WARRIOR();
	virtual ~WARRIOR();

	virtual void update(float elapsedTime);
	virtual SKILL* do_attack();
	virtual SKILL* do_skill();
};