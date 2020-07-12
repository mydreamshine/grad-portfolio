#pragma once
#include <DirectXCollision.h>
using namespace DirectX;

class HERO
{
public:
	HERO();

	void update(float fTime);
	void do_attack();
	void do_skill();

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 rot;
	DirectX::BoundingBox AABB;
	float vel;

	int hp;
	char anim_type;
	float anim_timepos;

private:
	void set_aabb();
};