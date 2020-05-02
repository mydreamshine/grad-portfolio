#include "HERO.h"

HERO::HERO()
{
	wasd[0] = false; wasd[1] = false; wasd[2] = false; wasd[3] = false;
	isMove = false;

	hp = 100;
	vel = 5.0f;

	AABB = BoundingBox(pos.ToXMFloat3(), XMFLOAT3(0.5f, 0.5f, 0.5f));
}

/*
0 for DOWN, 1 for UP
*/
void HERO::set_key(int updown, char key)
{
	if(updown == 0)
		switch (key)
		{
		case 'w':
			wasd[0] = true;
			break;
		case 'a':
			wasd[1] = true;
			break;
		case 's':
			wasd[2] = true;
			break;
		case 'd':
			wasd[3] = true;
			break;
		}
	else
		switch (key)
		{
		case 'w':
			wasd[0] = false;
			break;
		case 'a':
			wasd[1] = false;
			break;
		case 's':
			wasd[2] = false;
			break;
		case 'd':
			wasd[3] = false;
			break;
		}
}

void HERO::update(float fTime)
{
	move(fTime);
	set_aabb();
}

BULLET HERO::get_bullet(char shooter, Vector3d& dir)
{
	BULLET retval = BULLET(shooter);
	retval.damage = 10.0f;
	retval.pos = pos + dir * 0.5f;
	retval.dir = dir;
	retval.rot = Vector3d(0, dir.get_rotY(), 0);
	retval.vel = 10.0f;
	
	return retval;
}

void HERO::half_back()
{
	disp *= 0.5f;
	pos += disp;
	set_aabb();
}

void HERO::roll_back()
{
	pos += disp;
	disp = Vector3d();
	set_aabb();
}

void HERO::move(float fTime)
{
	Vector3d dir{0, 0, 0};
	if (wasd[0] == true) dir.z += 1;
	if (wasd[1] == true) dir.x -= 1;
	if (wasd[2] == true) dir.z -= 1;
	if (wasd[3] == true) dir.x += 1;

	if (dir.isZero() == false) {
		dir.normalize();
		dir *= vel * fTime;
		pos += dir;
		disp = dir * -1;
		rot.y = dir.get_rotY();
		isMove = true;
	}
	else isMove = false;
}

void HERO::set_aabb()
{
	AABB.Center = pos.ToXMFloat3();
}