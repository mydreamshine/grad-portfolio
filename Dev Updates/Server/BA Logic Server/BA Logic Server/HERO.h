#pragma once
#include "Vector3d.h"
#include "BULLET.h"
#include <DirectXCollision.h>
using namespace DirectX;

class HERO
{
public:
	HERO();
	void set_key(int updown, char key);
	void update(float fTime);

	int hp;

	Vector3d pos;
	Vector3d rot;
	Vector3d disp;
	float vel;
	
	bool wasd[4];
	bool isMove;

	BoundingBox AABB;

	BULLET get_bullet(char shooter, Vector3d& dir);
	void half_back();
	void roll_back();
private:
	void move(float fTime);
	void set_aabb();
};