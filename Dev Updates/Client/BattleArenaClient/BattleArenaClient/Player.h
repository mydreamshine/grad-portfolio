#pragma once
#include "Object.h"
#include <string>
#include "Common/Util/d3d12/Camera.h"
#include "Common/Timer/Timer.h"

namespace ActionType
{
	constexpr int Idle = 0x00000001;
	constexpr int Walking = 0x00000010;
	constexpr int Running = 0x00000100;
	constexpr int Attacking = 0x00001000;
	constexpr int Dieing = 0x0010000;
	constexpr int Being_Impacted_Damage = 0x00100000;
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
	// PlayerObject의 위치가 바뀔때마다 Camera의 위치도 변경된다.
	void ProcessInput(CTimer& gt);

	// PlayGameScene에서 매 업데이트마다
	// AnimateWorldObjectsTransform()를 통해 오브젝트의 Transform을 업데이트하기 때문에
	// Player에서 따로 Transform을 업데이트 해줄 필요는 없다.
	// 다만, 현재 AnimateWorldObjectsTransform() 메소드는
	// WorldObject의 Velocity를 기준으로 Transform이 업데이트되는 방식이기 때문에
	// Player Object의 Transform을 업데이트 하기 위해선 Velocity를 갱신해줄 필요가 있다.
	//void UpdateMove(CTimer& gt);

	void ProcessSkeletonAnimDurationDone(
		std::vector<std::unique_ptr<Object>>& AllObjects,
		std::vector<Object*>& WorldObjects,
		const UINT MaxWorldObj,
		std::unordered_map<std::string, std::unique_ptr<RenderItem>>& AllRitems,
		UINT& CurrSkillObjInstanceNUM);

	void CreateSkillObject(std::vector<std::unique_ptr<Object>>& AllObjects,
		std::vector<Object*>& WorldObjects,
		const UINT MaxWorldObj,
		std::unordered_map<std::string, std::unique_ptr<RenderItem>>& AllRitems,
		UINT& CurrSkillObjInstanceNUM);

	void UpdateCamera(CTimer& gt, float aspect);

//public:
//	void ControlSetMovementSpeed(Player* player);
};