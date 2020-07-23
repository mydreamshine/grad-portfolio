#include "stdafx.h"
#include "Player.h"

using namespace DirectX;
using namespace aiModelData;


void Player::SetTransform(const XMFLOAT3& Scale, const XMFLOAT3& RotationEuler, const XMFLOAT3& Position)
{
	auto& TransformInfo = m_CharacterObjRef->m_TransformInfo;
	TransformInfo->SetWorldTransform(Scale, RotationEuler, Position);
	TransformInfo->UpdateWorldTransform();
}

void Player::PlayMotion(MOTION_TYPE MotionType, SKILL_TYPE SkillMotionType)
{
	auto AnimInfo = m_CharacterObjRef->m_SkeletonInfo->m_AnimInfo.get();
	AnimActionType CurrPlayingAction = AnimInfo->CurrPlayingAction;
	AnimActionType newActionType = (AnimActionType)MotionType;

	if (newActionType == AnimActionType::Non) return;

	AnimInfo->AnimStop(CurrPlayingAction);
	AnimInfo->AnimPlay(newActionType);
	if (newActionType == AnimActionType::Idle || newActionType == AnimActionType::Walk)
		AnimInfo->AnimLoop(newActionType);
}

void Player::SetState(PLAYER_STATE PlayerState)
{
	m_CharacterObjRef->PlayerState = PlayerState;
}

void Player::SetHP(int HP)
{
	m_oldHP = m_CharacterObjRef->HP;
	m_CharacterObjRef->HP = HP;

	CHARACTER_TYPE CharacterType = m_CharacterObjRef->CharacterType;
	auto TextInfo = m_CharacterInfoTextObjRef->m_Textinfo.get();

	TextInfo->m_Text = m_Name + L"\nHP: ";
	TextInfo->m_Text += std::to_wstring(m_CharacterObjRef->HP) + L"/";
	std::wstring MaxHPText;
	switch (CharacterType)
	{
	case CHARACTER_TYPE::WARRIOR:   MaxHPText = L"100"; break;
	case CHARACTER_TYPE::BERSERKER: MaxHPText = L"150"; break;
	case CHARACTER_TYPE::ASSASSIN:  MaxHPText = L"100"; break;
	case CHARACTER_TYPE::PRIEST:    MaxHPText = L"80"; break;
	}
	TextInfo->m_Text += MaxHPText;
}

void Player::Init()
{
	ObjectManager ObjManager;
	for (auto& obj : m_CharacterObjRef->m_Childs)
		ObjManager.DeActivateObj(obj);
	ObjManager.DeActivateObj(m_CharacterObjRef);
	ObjManager.DeActivateObj(m_CharacterInfoTextObjRef);
	for (auto& obj : m_HP_BarObjRef)
		ObjManager.DeActivateObj(obj);
}

void Player::ProcessInput(const bool key_state[], const POINT& oldCursorPos,
	const CD3DX12_VIEWPORT& ViewPort, CTimer& gt,
	Object* GroundObject,
	std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
	EventManager eventManager;
	int Act_Object = m_CharacterObjRef->m_CE_ID;
	aiModelData::AnimActionType ActionType = m_CharacterObjRef->m_SkeletonInfo->m_AnimInfo->CurrPlayingAction;

	if (m_CharacterObjRef->PlayerState == PLAYER_STATE::ACT_DIE) return;

	// Set Moving Direction
	if (ActionType == aiModelData::AnimActionType::Idle
		|| ActionType == aiModelData::AnimActionType::Walk)
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
			else if(ActionType == aiModelData::AnimActionType::Walk)
				eventManager.ReservateEvent_TryMoveStopCharacter(GeneratedEvents, Act_Object);
		}
	}

	// Reservate Motion Events
	if (key_state[VK_LBUTTON] == true || key_state[VK_RBUTTON] == true)
	{
		if (ActionType == aiModelData::AnimActionType::Attack) return;
		if (ActionType == aiModelData::AnimActionType::SkillPose) return;

		float Yaw_angle = 0.0f;
		if (Player::ProcessPicking(oldCursorPos, ViewPort, gt, GroundObject, Yaw_angle, GeneratedEvents) == true)
		{

			if (key_state[VK_LBUTTON] == true)
			{
				eventManager.ReservateEvent_TryNormalAttack(GeneratedEvents, Act_Object, Yaw_angle);
			}
			else if (key_state[VK_RBUTTON] == true)
			{
				CHARACTER_TYPE CharacterType = m_CharacterObjRef->CharacterType;
				SKILL_TYPE SkillType = SKILL_TYPE::NON;
				switch (CharacterType)
				{
				case CHARACTER_TYPE::NON:       return;                             break;
				case CHARACTER_TYPE::WARRIOR:   SkillType = SKILL_TYPE::SWORD_WAVE; break;
				case CHARACTER_TYPE::BERSERKER: SkillType = SKILL_TYPE::FURY_ROAR;  break;
				case CHARACTER_TYPE::ASSASSIN:  SkillType = SKILL_TYPE::STEALTH;    break;
				case CHARACTER_TYPE::PRIEST:    SkillType = SKILL_TYPE::HOLY_AREA;  break;
				}
				eventManager.ReservateEvent_TryUseSkill(GeneratedEvents, Act_Object, Yaw_angle);
			}
		}
	}
}

bool Player::ProcessPicking(const POINT& oldCursorPos, const CD3DX12_VIEWPORT& ViewPort, CTimer& gt,
	Object* GroundObject, float& Yaw_angle, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
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
		int Act_Object = m_CharacterObjRef->m_CE_ID;

		XMVECTOR PlayerPOS = XMLoadFloat3(&m_CharacterObjRef->m_TransformInfo->GetWorldPosition());
		XMFLOAT3 playerPos; XMStoreFloat3(&playerPos, PlayerPOS);

		intersectPos.y = playerPos.y + m_CharacterObjRef->m_TransformInfo->m_Bound.Extents.y;
		XMVECTOR PickedPos = XMLoadFloat3(&intersectPos);
		XMFLOAT3 pickedPos; XMStoreFloat3(&pickedPos, PickedPos);

		eventManager.ReservateEvent_SpawnPickingEffectObj(GeneratedEvents, pickedPos);


		XMVECTOR PlayerNewLOOK = XMVector3Normalize(PickedPos - PlayerPOS);
		XMFLOAT3 playerNewLook; XMStoreFloat3(&playerNewLook, PlayerNewLOOK);

		XMVECTOR Z_Axis = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		XMFLOAT3 Dot; XMStoreFloat3(&Dot, XMVector3Dot(PlayerNewLOOK, Z_Axis));

		float rad2deg = 180.0f / MathHelper::Pi;
		Yaw_angle = atanf(playerNewLook.x / (playerNewLook.z)) * rad2deg;
		if (Dot.x < 0.0f)// PickedPos가 Player 뒤에 있을 때
		{
			if (Yaw_angle < 0.0f) // PickedPos가 우측후방에 있을때
				Yaw_angle += 180.0f;
			else if (Yaw_angle > 0.0f) // PickedPos가 좌측후방에 있을때
				Yaw_angle -= 180.0f;
		}
	}

	return Picking;
}

void Player::ProcessSkeletonAnimNotify(std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
	EventManager eventManager;
	int Act_Object = m_CharacterObjRef->m_CE_ID;

	std::string objName = m_CharacterObjRef->m_Name;
	auto& animInfo = m_CharacterObjRef->m_SkeletonInfo->m_AnimInfo;
	AnimActionType CurrPlayingAction = animInfo->CurrPlayingAction;
	MOTION_TYPE CurrMotion = (MOTION_TYPE)CurrPlayingAction;

	if (CurrPlayingAction != AnimActionType::Idle && CurrPlayingAction != AnimActionType::Walk)
	{
		bool AnimIsSetted = true;
		if (animInfo->AnimOnceDone(CurrPlayingAction, AnimIsSetted) == true)
			eventManager.ReservateEvent_DoneCharacterMotion(GeneratedEvents, Act_Object, CurrMotion);
	}

	bool AnimNotifyIsSetted = false;
	if (animInfo->CheckAnimTimeLineNotify("Sword Slash Gen", AnimNotifyIsSetted) == true)
		eventManager.ReservateEvent_ActivatedAnimNotify(GeneratedEvents, Act_Object, ANIM_NOTIFY_TYPE::WARRIOR_SKILL_SWORD_WAVE_OBJ_GEN);
	/*else if (animInfo->CheckAnimTimeLineNotify("Holy Area Gen", AnimNotifyIsSetted) == true)
		eventManager.ReservateEvent_ActivatedAnimNotify(GeneratedEvents, Act_Object, ANIM_NOTIFY_TYPE::PRIEST_SKILL_HOLY_AREA_OBJ_GEN);
	else if (animInfo->CheckAnimTimeLineNotify("Fury Roar Act", AnimNotifyIsSetted) == true)
		eventManager.ReservateEvent_ActivatedAnimNotify(GeneratedEvents, Act_Object, ANIM_NOTIFY_TYPE::BERSERKER_SKILL_FURY_ROAR_ACT);
	else if (animInfo->CheckAnimTimeLineNotify("Stealth Act", AnimNotifyIsSetted) == true)
		eventManager.ReservateEvent_ActivatedAnimNotify(GeneratedEvents, Act_Object, ANIM_NOTIFY_TYPE::ASSASSIN_SKILL_STEALTH_ACT);*/
}

void Player::UpdateCamera(CTimer& gt, float aspect)
{
	if (m_CharacterObjRef == nullptr) return;

	auto& transformInfo = m_CharacterObjRef->m_TransformInfo;

	float deg2rad = MathHelper::Pi / 180.0f;
	float camAngle = -90.0f * deg2rad;
	/*float camAngle_offset = transformInfo->GetWorldEuler().y;
	camAngle += (camAngle_offset * deg2rad);*/
	

	XMFLOAT3 LookTargetWorldScale = transformInfo->m_WorldScale;
	XMFLOAT3 LookAtPosition = transformInfo->m_WorldPosition;
	// Scale 250.0f가 ZoomFactor 1이라 가정
	float Scale_average = (LookTargetWorldScale.x / 250.0f + LookTargetWorldScale.y / 250.0f + LookTargetWorldScale.z / 250.0f) * 0.333333f;
	float phi = 30.0f * deg2rad;
	float rad = 3000.0f * Scale_average;
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
