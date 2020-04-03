#include "Object.h"
#include <iostream>
#include <math.h>


Object::Object()
{
	m_visible = false;
	InitPhysics();
}
Object::~Object()
{
}

void Object::InitPhysics()
{
	m_type = TYPE_NORMAL;

	m_pos = Vector2d(0, 0);
	m_vel = Vector2d(0, 0);
	m_acc = Vector2d(0, 0);
	m_vol = Vector2d(0, 0);

	m_color[0] = 1; m_color[1] = 1; m_color[2] = 1; m_color[3] = 1;

	m_mass = 1.0f;
	m_friction = 0.0f;
}

void Object::SetPos(Vector2d pos)
{
	m_pos = pos;
}

void Object::SetPos(float x, float y)
{
	m_pos.x = x;
	m_pos.y = y;
}

void Object::SetVol(Vector2d vol)
{
	m_vol = vol;
}

void Object::SetColor(float r, float g, float b, float a)
{
	m_color[0] = r;
	m_color[1] = g;
	m_color[2] = b;
	m_color[3] = a;
}

void Object::SetVol(float x, float y)
{
	m_vol.x = x;
	m_vol.y = y;
}

void Object::SetVel(Vector2d vel)
{
	m_vel = vel;
}

void Object::SetVel(float x, float y)
{
	m_vel.x = x;
	m_vel.y = y;
}

void Object::SetMass(float mass)
{
	m_mass = mass;
}

void Object::SetFriction(float frict)
{
	m_friction = frict;
}

void Object::SetType(int type)
{
	m_type = type;
}

void Object::SetTex(int tex)
{
	m_texID = tex;
}

///////////////////////////////////////////////////////////////////////////////////////
void Object::Update(float fTimeElapsed)
{
	if (m_visible == false) return;

	// Apply Friction
	float velSize = m_vel.length();
	if (velSize > 0.0f)
	{
		Vector2d acc = m_vel;
		acc.normalize();
		//Calc friction size
		float nForce = m_mass * GRAVITY;
		float frictionSize = nForce * m_friction;

		acc *= -frictionSize;
		acc /= m_mass;
		Vector2d newVel = m_vel + acc * fTimeElapsed;

		if (newVel.x * m_vel.x < 0.f)
		{
			m_vel.x = 0;
		}
		else
		{
			m_vel.x = newVel.x;
		}
		if (newVel.y * m_vel.y < 0.f)
		{
			m_vel.y = 0;
		}
		else
		{
			m_vel.y = newVel.y;
		}
	}
	m_pos += m_vel * fTimeElapsed;
}
void Object::AddForce(Vector2d force, float fElapsedTime)
{
	Vector2d acc = force / m_mass;
	m_vel += acc * fElapsedTime;
}

bool Object::isOverlap(const Object & other)
{
	float aMinX, aMaxX, aMinY, aMaxY;
	aMinX = m_pos.x - m_vol.x / 2.0f; aMaxX = m_pos.x + m_vol.x / 2.0f;
	aMinY = m_pos.y - m_vol.y / 2.0f; aMaxY = m_pos.y + m_vol.y / 2.0f;

	float bMinX, bMaxX, bMinY, bMaxY;
	bMinX = other.m_pos.x - other.m_vol.x / 2.0f; bMaxX = other.m_pos.x + other.m_vol.x / 2.0f;
	bMinY = other.m_pos.y - other.m_vol.y / 2.0f; bMaxY = other.m_pos.y + other.m_vol.y / 2.0f;

	if (aMinX > bMaxX) return false;
	if (aMaxX < bMinX) return false;

	if (aMinY > bMaxY) return false;
	if (aMaxY < bMinY) return false;

	return true;
}

//Player
Player::Player() : Object()
{
	m_visible = true;
	weapon = 0;
	m_remainingBulletCoolTime = m_defaultBulletCoolTime[weapon];
}
Player::~Player() {};

void Player::Update(float fTimeElapsed)
{
	// Reduce Bullet Cooltime
	m_remainingBulletCoolTime -= fTimeElapsed;

	Object::Update(fTimeElapsed);
	for (int i = 0; i < MAX_BULLET; ++i)
		bullets[i].Update(fTimeElapsed);
}

void Player::keyMove(float fTimeElapsed)
{
	Vector2d keyDir(0, 0);
	if (m_keyW)
	{
		keyDir.y += 1.0f;
	}
	if (m_keyS)
	{
		keyDir.y -= 1.0f;
	}
	if (m_keyA)
	{
		keyDir.x -= 1.0f;
	}
	if (m_keyD)
	{
		keyDir.x += 1.0f;
	}
	if (keyDir.length() > 0.0000001f)
	{
		float fAmount = 10.0f;
		keyDir.normalize();
		keyDir *= fAmount;
		AddForce(keyDir, fTimeElapsed);
	}
}

bool Player::CanShootBullet()
{
	return m_remainingBulletCoolTime <= 0.0000001f;
}

void Player::ResetShootBulletCoolTime()
{
	m_remainingBulletCoolTime = m_defaultBulletCoolTime[weapon];
}

int Player::AddBullet(Vector2d pos, Vector2d vol, Vector2d vel, float r, float g, float b, float a, float mass, float fricCoef)
{
	int idx = -1;
	for (int i = 0; i < MAX_BULLET; ++i)
	{
		if (bullets[i].m_visible == false)
		{
			idx = i;
			break;
		}
	}

	if (idx == -1)
	{
		std::cout << "No more remaining object" << std::endl;
		return idx;
	}

	bullets[idx].m_visible = true;
	bullets[idx].SetPos(pos);
	bullets[idx].SetColor(r, g, b, a);
	bullets[idx].SetVol(vol);
	bullets[idx].SetVel(vel);
	bullets[idx].SetMass(mass);
	bullets[idx].SetFriction(fricCoef);

	return idx;
}

void Player::ShootBullet(Vector2d MousePos)
{
	if (CanShootBullet() == false) return;

	float fAmountBullet = 8.0f, mass = 1.0f, fricCoef= 0.9f;

	Vector2d bulletDir = MousePos / 100;
	bulletDir -= m_pos;
	bulletDir.normalize();
	Vector2d hVel = m_vel + bulletDir * fAmountBullet;
	Vector2d vol(0.05f, 0.05f);

	int idx = AddBullet(m_pos, vol, hVel, 1, 0, 0, 1, mass, fricCoef);
	bullets[idx].type = weapon;

	ResetShootBulletCoolTime();
}