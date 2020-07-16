#pragma once
#include "Vector3d.h"
#include <DirectXCollision.h>
using namespace DirectX;

class DMRoom;
class SKILL;

class HERO
{
public:
	HERO(DMRoom* world, short object_id, char propensity);
	virtual ~HERO();

	void rotate(float yaw);
	virtual void move(float elapsedTime);
	void correct_position(DirectX::BoundingBox& other);
	void change_motion(char motion);
	void change_state(char state);
	bool is_die();
	void set_hp(int hp);
	void impact();
	void respawn();
	virtual void death();
	virtual void update(float elapsedTime);
	virtual void do_attack();
	virtual void do_skill() {};
	
	DMRoom* world;

	Vector3d pos;
	Vector3d rot;
	Vector3d dir;
	bool changed_transform;
	DirectX::BoundingBox AABB;
	float vel;

	char propensity;

	int hp;
	short object_id;
	char character_type;
	char character_state;
	char motion_type;
	float anim_time_pos;
	float remain_state_time;

private:
	void set_aabb();
	Vector3d get_offset(DirectX::BoundingBox& other);
};

class WARRIOR : public HERO
{
public:
	WARRIOR(DMRoom* world, short object_id, char propensity);
	virtual ~WARRIOR();
	virtual void do_skill();
};

class PRIEST : public HERO
{
public:
	PRIEST(DMRoom* world, short object_id, char propensity);
	virtual ~PRIEST();
	virtual void do_skill();
};

class BERSERKER : public HERO
{
	BERSERKER(DMRoom* world, short object_id, char propensity);
	virtual ~BERSERKER();
	virtual void do_skill();

	virtual void move(float elapsedTime);
	virtual void death();
	virtual void update(float elapsedTime);

private:
	bool roar_mode;
	float roar_time;
	SKILL* roar_skill;
};

class ASSASSIN : public HERO
{
public:
	ASSASSIN(DMRoom* world, short object_id, char propensity);
	virtual ~ASSASSIN();
	virtual void do_skill();
	virtual void death();
	virtual void update(float elapsedTime);

private:
	bool stelth_mode;
	float stelth_time;
	SKILL* stelth_skill;
};