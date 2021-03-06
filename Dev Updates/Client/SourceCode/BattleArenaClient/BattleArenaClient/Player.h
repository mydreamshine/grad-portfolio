#pragma once
#include "Object.h"
#include "Ray.h"
#include <string>
#include <queue>
#include "FrameworkEvent.h"
#include "Common/Util/d3d12/Camera.h"
#include "Common/Timer/Timer.h"

struct Player
{
	std::wstring m_Name;

	Object* m_CharacterObjRef = nullptr;
	Object* m_CharacterInfoTextObjRef = nullptr; // Text Object
	Object* m_HP_BarObjRef[3] = { nullptr, nullptr, nullptr }; // 0: 테두리, 1: 증감HP Bar, 2: HP Bar

	Camera m_Camera;

	int m_oldHP = 0;

	SKILL_TYPE m_CurrUseSkill = SKILL_TYPE::NON;
	float m_SkillCoolTimeStack = 0.0f;

	const float StealthAlpha = 0.3f;
	const float BlinkEffectInterval = 0.2f;
	float semi_invincivilityRenderBlinkEffectTimeStack = 0.0f;

	OBJECT_PROPENSITY MainPlayerPropensity = OBJECT_PROPENSITY::NON;

	bool oldMouseButtonDown = false;

	bool HP_BarAcitvate = true;

public:
	void SetTransform(const DirectX::XMFLOAT3& Scale, const DirectX::XMFLOAT3& RotationEuler, const DirectX::XMFLOAT3& Position);
	void PlayMotion(MOTION_TYPE MotionType, float MotionSpeed = 1.0f, SKILL_TYPE SkillMotionType = SKILL_TYPE::NON);
	void SetState(PLAYER_STATE PlayerState);
	void SetHP(int HP);
	void SetHP_BarActivate(bool state);

	void Init();

public:
	// PlayerObject의 위치가 바뀔때마다 Camera의 위치도 변경된다.
	void ProcessInput(const bool key_state[], const POINT& oldCursorPos,
		const CD3DX12_VIEWPORT& ViewPort, CTimer& gt,
		Object* GroundObject,
		std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

	// return Yaw_angle
	bool ProcessPicking(const POINT& oldCursorPos, const CD3DX12_VIEWPORT& ViewPort, CTimer& gt,
		Object* GroundObject, float& Yaw_angle, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

	void ProcessSkeletonAnimNotify(std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

	void UpdateCamera(CTimer& gt, float aspect);

	void UpdateUITransform(Camera* MainCamera, const CD3DX12_VIEWPORT& ViewPort, CTimer& gt);

	void UpdateRenderEffect(CTimer& gt);
};
