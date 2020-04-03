#pragma once
#include <iostream>
#include <cmath>
#define M_PI 3.14159265358979323846

class Vector2d {
public:
	float x, y;
	Vector2d() :x(0), y(0) {};
	Vector2d(float x, float y) :x(x), y(y) {};
	~Vector2d() {};

	Vector2d operator+(const Vector2d& other) { return Vector2d(this->x + other.x, this->y + other.y); };
	Vector2d operator-(const Vector2d& other) { return Vector2d(this->x - other.x, this->y - other.y); };
	Vector2d operator*(const float& value) { return Vector2d(this->x * value, this->y * value); };
	Vector2d operator/(const float& value) { return Vector2d(this->x / value, this->y / value); };

	Vector2d& operator=(const Vector2d& other) { this->x = other.x; this->y = other.y; return *this; };
	Vector2d& operator+=(const Vector2d& other) { this->x += other.x; this->y += other.y; return *this; };
	Vector2d& operator-=(const Vector2d& other) { this->x -= other.x; this->y -= other.y; return *this; };
	Vector2d& operator*=(const float& value) { this->x *= value; this->y *= value; return *this; };
	Vector2d& operator/=(const float& value) { this->x /= value; this->y /= value; return *this; };
	Vector2d& rotate(float degree) {
		float rad = degree * (float)M_PI / 180.0f;
		float x = this->x;
		float y = this->y;
		this->x = cos(rad)*x - sin(rad)*y;
		this->y = sin(rad)*x + cos(rad)*y;
		return *this;
	}
	bool operator==(const Vector2d& other) { return (this->x == other.x) && (this->y == other.y); };

	float dot(const Vector2d& other) const { return this->x*other.x + this->y*other.y; };
	float cross(const Vector2d& other) const { return this->x*other.y - this->y*other.x; };
	float length() { return sqrtf(x*x + y * y); };
	Vector2d& normalize() { if (isZero()) return *this;	float len = length();	x /= len; y /= len;	return *this; };
	bool isZero() { return length() < 0.000001f; };

	void clear() { x = 0; y = 0; };
};