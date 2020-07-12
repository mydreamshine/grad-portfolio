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
	virtual void update(float elapsedTime);
	virtual void effect(HERO* hero);

	Vector3d pos;
	Vector3d rot;
	Vector3d dir;
	DirectX::BoundingBox AABB;
	float vel;

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

private:
	int damage{ 10 };
};