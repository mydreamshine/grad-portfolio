#pragma once
#include "Vector2d.h"
#include "Config.h"

class Object
{
public:
	bool m_visible;

	int m_type;
	Vector2d m_pos;
	Vector2d m_vel;
	Vector2d m_acc;
	Vector2d m_vol;
	float m_mass;
	float m_friction;
	int m_texID = -1;
	float m_color[4];

	Object();
	~Object();
	void InitPhysics();

	//Set
	void SetPos(Vector2d pos);
	void SetPos(float x, float y);
	void SetVol(Vector2d vol);
	void SetVol(float x, float y);
	void SetVel(Vector2d vel);
	void SetVel(float x, float y);
	void SetColor(float r, float g, float b, float a);
	void SetMass(float mass);
	void SetFriction(float frict);
	void SetType(int type);
	void SetTex(int tex);

	//Update
	void Update(float fTimeElapsed);
	void AddForce(Vector2d force, float fElapsedTime);

	//collision
	bool isOverlap(const Object& other);
	void correctpos(const Object& collider) {
		Vector2d dir = m_pos - collider.m_pos;
		if (dir.y * dir.y > dir.x * dir.x)
		{
			float len = (m_vol.y + collider.m_vol.y) / 2.0f;
			if (dir.y > 0)
				m_pos.y = collider.m_pos.y + len;
			else
				m_pos.y = collider.m_pos.y - len;
		}
		else
		{
			float len = (m_vol.x + collider.m_vol.x) / 2.0f;
			if (dir.x > 0)
				m_pos.x = collider.m_pos.x + len;
			else
				m_pos.x = collider.m_pos.x - len;
		}
	}
};

class Bullet : public Object
{
public:
	Bullet() : Object() {};
	~Bullet(){};

	int type;
};

class Item : public Object
{
public:
	Item() : Object() { type = rand()%3; };
	~Item() {};
	int type;
};

class Player : public Object
{
private:
	float m_remainingBulletCoolTime = 0.0f;
	float m_defaultBulletCoolTime[3] = { 0.1f, 0.075f, 0.125f };
	

public:
	int m_id;
	bool m_isConnect = false;
	int match_round = -1;
	int m_hp = 100;
	int weapon = 0;
	Bullet bullets[MAX_BULLET];

	int m_damage[3] = { 30, 20, 50 };

	//Key Inputs
	bool m_keyW = false;
	bool m_keyA = false;
	bool m_keyS = false;
	bool m_keyD = false;

	//Mouse Input
	bool m_mouseLeft = false;
	Vector2d m_mousepos;

	int m_socket;

	Player();
	~Player();
	void Update(float fTimeElapsed);
	void keyMove(float fTimeElapsed);
	bool CanShootBullet();
	void ResetShootBulletCoolTime();
	int AddBullet(Vector2d pos, Vector2d vol, Vector2d vel, float r, float g, float b, float a, float mass, float fricCoef);
	void ShootBullet(Vector2d MousePos);
	bool getDamage(const Bullet& bullet)
	{
		m_hp -= m_damage[bullet.type];
		return m_hp <= 0;
	}
	bool getDamage(const int& damage)
	{
		m_hp -= damage;
		return m_hp <= 0;
	}
	void die() {
		m_visible = false;
		for (auto& b : bullets)
			b.m_visible = false;
	}
};