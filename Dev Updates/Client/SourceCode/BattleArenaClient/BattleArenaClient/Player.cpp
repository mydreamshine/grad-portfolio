#include "stdafx.h"
#include "Player.h"

using namespace DirectX;


void Player::SetTransform(const XMFLOAT3& Scale, const XMFLOAT3& RotationEuler, const XMFLOAT3& Position)
{
	auto& TransformInfo = m_ObjectRef->m_TransformInfo;
	TransformInfo->SetWorldTransform(Scale, RotationEuler, Position);
	TransformInfo->UpdateWorldTransform();
}

void Player::SetMotion(MOTION_TYPE MotionType, SKILL_TYPE SkillMotionType)
{
	auto AnimInfo = m_ObjectRef->m_SkeletonInfo->m_AnimInfo.get();
	CHARACTER_TYPE CharacterType = m_ObjectRef->CharacterType;
	std::string AnimName = AnimInfo->CurrPlayingAnimName;

	if (MotionType == MOTION_TYPE::NON) return;

	AnimInfo->AnimStop(AnimName);

	switch (MotionType)
	{
	case MOTION_TYPE::IDLE:
		m_CurrAction = ActionType::Idle;
		break;
	case MOTION_TYPE::WALK:
		break;
	case MOTION_TYPE::ATTACK:
		break;
	case MOTION_TYPE::IMPACT:
		break;
	case MOTION_TYPE::SKILL_POSE:
	{
		switch (SkillMotionType)
		{
		case SKILL_TYPE::NON: return; break;
		case SKILL_TYPE::SWORD_WAVE:
			break;
		case SKILL_TYPE::HOLY_AREA:
			break;
		case SKILL_TYPE::FURY_ROAR:
			break;
		case SKILL_TYPE::STEALTH:
			break;
		default:
			break;
		}
	}
		break;
	case MOTION_TYPE::DIEING:
		break;
	}
}

void Player::SetState(PLAYER_STATE PlayerState)
{
	m_ObjectRef->PlayerState = PlayerState;
}

void Player::SetHP(int HP)
{
	m_ObjectRef->HP = HP;
}

void Player::ProcessInput(const bool key_state[], const POINT& oldCursorPos,
	const CD3DX12_VIEWPORT& ViewPort, CTimer& gt,
	Object* GroundObject,
	std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
	EventManager eventManager;
	int Act_Object = m_ObjectRef->m_CE_ID;

	// Set Moving Direction
	if (((m_CurrAction & ActionType::Idle) == ActionType::Idle)
		|| ((m_CurrAction & ActionType::Walk) == ActionType::Walk))
	{
		float yaw_angle = 0.0f;
		float deg2rad = MathHelper::Pi / 180.0f;

		if (key_state[0x57] == true) // w
		{
			yaw_angle = 0.0f;
		}
		if (key_state[0x41] == true) // a
		{
			if (key_state[0x57] == true) yaw_angle = -45.0f;
			else yaw_angle = -90.0f;
		}
		if (key_state[0x53] == true) // s
		{
			if (key_state[0x41] == true) yaw_angle = -135.0f;
			else yaw_angle = -180.0f;
		}
		if (key_state[0x44] == true) // d
		{
			if (key_state[0x57] == true) yaw_angle = 45.0f;
			else if (key_state[0x53] == true) yaw_angle = 135.0f;
			else yaw_angle = 90.0f;
		}


		// Reservate Moving Events
		{
			if (key_state[0x57] || key_state[0x41] || key_state[0x53] || key_state[0x44])
				eventManager.ReservateEvent_TryMoveCharacter(GeneratedEvents, Act_Object, yaw_angle);
		}
	}

	// Reservate Motion Events
	if (key_state[VK_LBUTTON] == true || key_state[VK_RBUTTON] == true)
	{
		if ((m_CurrAction & ActionType::Attack) == ActionType::Attack) return;
		if ((m_CurrAction & ActionType::SkillPose) == ActionType::SkillPose) return;

		Player::ProcessPicking(oldCursorPos, ViewPort, gt, GroundObject, GeneratedEvents);

		if (key_state[VK_LBUTTON] == true)
		{
			eventManager.ReservateEvent_TryNormalAttack(GeneratedEvents, Act_Object);
		}
		else if (key_state[VK_RBUTTON] == true)
		{
			CHARACTER_TYPE CharacterType = m_ObjectRef->CharacterType;
			SKILL_TYPE SkillType = SKILL_TYPE::NON;
			switch (CharacterType)
			{
			case CHARACTER_TYPE::NON:       return;                             break;
			case CHARACTER_TYPE::WARRIOR:   SkillType = SKILL_TYPE::SWORD_WAVE; break;
			case CHARACTER_TYPE::BERSERKER: SkillType = SKILL_TYPE::FURY_ROAR;  break;
			case CHARACTER_TYPE::ASSASSIN:  SkillType = SKILL_TYPE::STEALTH;    break;
			case CHARACTER_TYPE::PRIEST:    SkillType = SKILL_TYPE::HOLY_AREA;  break;
			}
			eventManager.ReservateEvent_TryUseSkill(GeneratedEvents, Act_Object, SkillType);
		}
	}
}

void Player::ProcessPicking(const POINT& oldCursorPos, const CD3DX12_VIEWPORT& ViewPort, CTimer& gt,
	Object* GroundObject, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
	bool Picking = false;
	XMFLOAT3 intersectPos;

	if (GroundObject != nullptr)
	{
		CRay CursorRay;
		CursorRay = CursorRay.RayAtWorldSpace(ViewPort, m_Camera.GetProj4x4f(), m_Camera.GetView4x4f(), oldCursorPos.x, oldCursorPos.y);

		Picking = CursorRay.RayAABBIntersect(CursorRay, GroundObject->m_TransformInfo->m_Bound, intersectPos);
	}

	if (Picking == true)
	{
		EventManager eventManager;
		int Act_Object = m_ObjectRef->m_CE_ID;

		XMVECTOR PlayerPOS = XMLoadFloat3(&m_ObjectRef->m_TransformInfo->GetWorldPosition());
		XMFLOAT3 playerPos; XMStoreFloat3(&playerPos, PlayerPOS);

		intersectPos.y = playerPos.y + m_ObjectRef->m_TransformInfo->m_Bound.Extents.y;
		XMVECTOR PickedPos = XMLoadFloat3(&intersectPos);
		XMFLOAT3 pickedPos; XMStoreFloat3(&pickedPos, PickedPos);

		eventManager.ReservateEvent_SpawnPickingEffectObj(GeneratedEvents, pickedPos);


		XMVECTOR PlayerNewLOOK = XMVector3Normalize(PickedPos - PlayerPOS);
		XMFLOAT3 playerNewLook; XMStoreFloat3(&playerNewLook, PlayerNewLOOK);

		XMVECTOR Z_Axis = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		XMFLOAT3 Dot; XMStoreFloat3(&Dot, XMVector3Dot(PlayerNewLOOK, Z_Axis));

		float rad2deg = 180.0f / MathHelper::Pi;
		float Yaw_angle = atanf(playerNewLook.x / (playerNewLook.z)) * rad2deg;
		if (Dot.x < 0.0f)// PickedPos가 Player 뒤에 있을 때
		{
			if (Yaw_angle < 0.0f) // PickedPos가 우측후방에 있을때
				Yaw_angle += 180.0f;
			else if (Yaw_angle > 0.0f) // PickedPos가 좌측후방에 있을때
				Yaw_angle -= 180.0f;
		}

		eventManager.ReservateEvent_TryRotationCharacter(GeneratedEvents, Act_Object, Yaw_angle);
	}
}

void Player::ProcessSkeletonAnimNotify(std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
	EventManager eventManager;
	int Act_Object = m_ObjectRef->m_CE_ID;

	std::string objName = m_ObjectRef->m_Name;
	auto& animInfo = m_ObjectRef->m_SkeletonInfo->m_AnimInfo;
	std::string currAnimName = animInfo->CurrPlayingAnimName;

	if ((m_CurrAction & ActionType::SkillPose) == ActionType::SkillPose)
	{
		if (objName.find("Meshtint Free Knight") != std::string::npos)
		{
			if (currAnimName == "Meshtint Free Knight@Sword And Shield Slash")
			{
				bool AnimNotifyIsSetted = false;
				if (animInfo->CheckAnimTimeLineNotify("Meshtint Free Knight@Sword And Shield Slash-SlashGen", AnimNotifyIsSetted) == true)
					eventManager.ReservateEvent_TryUseSkill(GeneratedEvents, Act_Object, SKILL_TYPE::SWORD_WAVE);

				bool AnimIsSetted = true;
				if (animInfo->AnimOnceDone(currAnimName, AnimIsSetted) == true)
					eventManager.ReservateEvent_DoneCharacterMotion(GeneratedEvents, Act_Object, MOTION_TYPE::SKILL_POSE);
			}
		}
	}
}

void Player::UpdateCamera(CTimer& gt, float aspect)
{
	if (m_ObjectRef == nullptr) return;

	auto& transformInfo = m_ObjectRef->m_TransformInfo;

	float deg2rad = MathHelper::Pi / 180.0f;
	float camAngle = -90.0f * deg2rad;
	/*float camAngle_offset = transformInfo->GetWorldEuler().y;
	camAngle += (camAngle_offset * deg2rad);*/
	

	XMFLOAT3 LookTargetWorldScale = transformInfo->m_WorldScale;
	XMFLOAT3 LookAtPosition = transformInfo->m_WorldPosition;
	float Scale_average = (LookTargetWorldScale.x + LookTargetWorldScale.y + LookTargetWorldScale.z) * 0.333333f;
	float phi = 30.0f * deg2rad;
	float rad = 1500.0f * Scale_average;
	XMVECTOR Eye_Pos = MathHelper::SphericalToCartesian(rad, camAngle, phi);
	Eye_Pos = XMVectorAdd(Eye_Pos, XMLoadFloat3(&LookAtPosition));
	XMFLOAT3 EyePosition;
	XMStoreFloat3(&EyePosition, Eye_Pos);
	XMFLOAT3 UpDirection = { 0.0f, 1.0f, 0.0f };

	m_Camera.SetPosition(EyePosition);
	m_Camera.SetPerspectiveLens(DirectX::XM_PIDIV4, aspect, 1.0f, 10000.0f);
	m_Camera.LookAt(EyePosition, LookAtPosition, UpDirection);
	m_Camera.UpdateViewMatrix();
}
