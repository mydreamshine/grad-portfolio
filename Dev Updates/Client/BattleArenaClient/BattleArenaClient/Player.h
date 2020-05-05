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
	// PlayerObject�� ��ġ�� �ٲ𶧸��� Camera�� ��ġ�� ����ȴ�.
	void ProcessInput(CTimer& gt);

	// PlayGameScene���� �� ������Ʈ����
	// AnimateWorldObjectsTransform()�� ���� ������Ʈ�� Transform�� ������Ʈ�ϱ� ������
	// Player���� ���� Transform�� ������Ʈ ���� �ʿ�� ����.
	// �ٸ�, ���� AnimateWorldObjectsTransform() �޼ҵ��
	// WorldObject�� Velocity�� �������� Transform�� ������Ʈ�Ǵ� ����̱� ������
	// Player Object�� Transform�� ������Ʈ �ϱ� ���ؼ� Velocity�� �������� �ʿ䰡 �ִ�.
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