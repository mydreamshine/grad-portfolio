#include "HERO.h"

HERO::HERO() :
	hp(100),
	vel(5.0f),
	rot(0, 0, -1.0f),
	AABB(pos, XMFLOAT3(0.5f, 0.5f, 0.5f))
{
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
	set_aabb();
}


void HERO::set_aabb()
{
	AABB.Center = pos;
}