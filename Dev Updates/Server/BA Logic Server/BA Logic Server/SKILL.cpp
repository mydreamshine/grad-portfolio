#include "BULLET.h"

BULLET::BULLET(char shooter) :
	shooter(shooter)
{
	AABB.Extents.x = 0.25f;
	AABB.Extents.y = 0.25f;
	AABB.Extents.z = 0.25f;
	damage = 0;
	vel = 0;
	isDestroy = false;
}

void BULLET::update(float fTime)
{
	move(fTime);
	set_aabb();
}

void BULLET::move(float fTime)
{
	pos += dir * vel * fTime;
}

void BULLET::set_aabb()
{
	AABB.Center = pos.ToXMFloat3();
}
