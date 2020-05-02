#pragma once
#include "Vector3d.h"
#include <DirectXCollision.h>
using namespace DirectX;

class BULLET
{
public:
	BULLET(char shooter);
	void update(float fTime);

	char shooter;
	int damage;
	bool isDestroy;

	Vector3d pos;
	Vector3d rot;
	Vector3d dir;
	float vel;
	BoundingBox AABB;

private:
	void move(float fTime);
	void set_aabb();
};