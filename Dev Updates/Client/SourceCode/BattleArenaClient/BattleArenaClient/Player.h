#pragma once
#include "Object.h"
#include "Ray.h"
#include <string>
#include <queue>
#include "FrameworkEvent.h"
#include "Common/Util/d3d12/Camera.h"
#include "Common/Timer/Timer.h"

namespace ActionType
{
	constexpr int Idle      = 0x00000001;
	constexpr int Walk      = 0x00000010;
	constexpr int Attack    = 0x00000100;
	constexpr int SkillPose = 0x00001000;
	constexpr int Dieing    = 0x00010000;
	constexpr int Impact    = 0x00100000;
}

namespace ActionNotifyTime
{
	constexpr float MeshtintFreeKnight_SwordSlashStart = 0.6f;
}

namespace CharacterSpeed
{
	constexpr float MeshtintFreeKnightSpeed = 360.0f;
}

struct Player
{
	std::wstring m_Name;

	Object* m_ObjectRef = nullptr;

	Camera m_Camera;
	int m_CurrAction = ActionType::Idle;

public:
	void SetTransform(const XMFLOAT3& Scale, const XMFLOAT3& RotationEuler, const XMFLOAT3& Position);
	void SetMotion(MOTION_TYPE MotionType, SKILL_TYPE SkillMotionType = SKILL_TYPE::NON);
	void SetState(PLAYER_STATE PlayerState);
	void SetHP(int HP);

public:
	// PlayerObject의 위치가 바뀔때마다 Camera의 위치도 변경된다.
	void ProcessInput(const bool key_state[], const POINT& oldCursorPos,
		const CD3DX12_VIEWPORT& ViewPort, CTimer& gt,
		Object* GroundObject,
		std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

	void ProcessPicking(const POINT& oldCursorPos, const CD3DX12_VIEWPORT& ViewPort, CTimer& gt,
		Object* GroundObject, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

	void ProcessSkeletonAnimNotify(std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

	void UpdateCamera(CTimer& gt, float aspect);
};