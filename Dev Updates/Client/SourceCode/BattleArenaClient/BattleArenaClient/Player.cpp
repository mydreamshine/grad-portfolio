#include "stdafx.h"
#include "Player.h"

using namespace DirectX;

void Player::SetCreateSkillObjRef(
	std::vector<std::unique_ptr<Object>>& AllObjects,
	std::vector<Object*>& WorldObjects,
	const UINT MaxWorldObj,
	std::unordered_map<std::string, std::unique_ptr<RenderItem>>& AllRitems,
	UINT& CurrSkillObjInstanceNUM)
{
	AllObjectsRef = &AllObjects;
	WorldObjectsRef = &WorldObjects;
	MaxCreateWorldObj = MaxWorldObj;
	AllRitemsRef = &AllRitems;
	CurrSkillObjInstanceNUMRef = &CurrSkillObjInstanceNUM;
}

void Player::ProcessInput(const bool key_state[], const POINT& oldCursorPos,
	const CD3DX12_VIEWPORT& ViewPort, CTimer& gt)
{
	if (m_ObjectRef == nullptr) return;

	// Set Velocity
	if (((m_CurrAction & ActionType::Idle) == ActionType::Idle)
		|| ((m_CurrAction & ActionType::Walking) == ActionType::Walking))
	{
		XMFLOAT3 newVelocity = { 0.0f, 0.0f, 0.0f };
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

		if (key_state[0x57] || key_state[0x41] || key_state[0x53] || key_state[0x44])
		{
			m_ObjectRef->m_TransformInfo->SetWorldRotationEuler({ 0.0f, yaw_angle, 0.0f });
			m_ObjectRef->m_TransformInfo->UpdateWorldTransform();


			std::string objName = m_ObjectRef->m_Name;
			if (objName.find("Meshtint Free Knight") != std::string::npos)
			{
				newVelocity = m_ObjectRef->m_TransformInfo->GetLook();
				newVelocity.x *= CharacterSpeed::MeshtintFreeKnightSpeed;
				newVelocity.y *= CharacterSpeed::MeshtintFreeKnightSpeed;
				newVelocity.z *= CharacterSpeed::MeshtintFreeKnightSpeed;

				auto& animInfo = m_ObjectRef->m_SkeletonInfo->m_AnimInfo;
				std::string currAnimName = animInfo->CurrPlayingAnimName;
				if (currAnimName != "Meshtint Free Knight@Stride Walking")
				{
					animInfo->AnimStop(currAnimName);
					animInfo->AnimPlay("Meshtint Free Knight@Stride Walking");
					animInfo->AnimLoop("Meshtint Free Knight@Stride Walking");
				}
			}

			m_CurrAction = ActionType::Walking;
		}
		else
		{
			std::string objName = m_ObjectRef->m_Name;
			if (objName.find("Meshtint Free Knight") != std::string::npos)
			{
				auto& animInfo = m_ObjectRef->m_SkeletonInfo->m_AnimInfo;
				std::string currAnimName = animInfo->CurrPlayingAnimName;
				if (currAnimName != "Meshtint Free Knight@Battle Idle")
				{
					animInfo->AnimStop(currAnimName);
					animInfo->AnimPlay("Meshtint Free Knight@Battle Idle");
					animInfo->AnimLoop("Meshtint Free Knight@Battle Idle");
				}
			}

			m_CurrAction = ActionType::Idle;
		}
		m_ObjectRef->m_TransformInfo->SetVelocity(newVelocity);
	}

	// Set Animation Action
	if (key_state[VK_LBUTTON] == true)
	{
		m_oldCursorPos = oldCursorPos;
		std::string objName = m_ObjectRef->m_Name;
		if (objName.find("Meshtint Free Knight") != std::string::npos)
		{
			auto& animInfo = m_ObjectRef->m_SkeletonInfo->m_AnimInfo;
			std::string currAnimName = animInfo->CurrPlayingAnimName;
			if (currAnimName != "Meshtint Free Knight@Sword And Shield Slash")
			{
				animInfo->AnimStop(currAnimName);
				animInfo->AnimPlay("Meshtint Free Knight@Sword And Shield Slash");

				Player::ProcessPicking(ViewPort, gt);

				m_ObjectRef->m_TransformInfo->SetVelocity({0.0f, 0.0f, 0.0f});
			}
		}

		m_CurrAction = ActionType::Attacking;
	}
}

void Player::ProcessPicking(const CD3DX12_VIEWPORT& ViewPort, CTimer& gt)
{
	if (WorldObjectsRef == nullptr) return;
	Object* groundObj = nullptr;
	for (auto& obj : (*WorldObjectsRef))
	{
		if (obj->m_Name == "Floor1")
			groundObj = obj;
	}

	XMVECTOR PlayerPOS = XMLoadFloat3(&m_ObjectRef->m_TransformInfo->GetWorldPosition());
	XMFLOAT3 PlayerWorldEuler = m_ObjectRef->m_TransformInfo->GetWorldEuler();

	bool Picking = false;
	XMFLOAT3 intersectPos;
	if (groundObj != nullptr)
	{
		CRay CursorRay;
		CursorRay = CursorRay.RayAtWorldSpace(ViewPort, m_Camera.GetProj4x4f(), m_Camera.GetView4x4f(), m_oldCursorPos.x, m_oldCursorPos.y);

		Picking = CursorRay.RayAABBIntersect(CursorRay, groundObj->m_TransformInfo->m_Bound, intersectPos);
	}
	if (Picking == true)
	{
		XMFLOAT3 playerPos; XMStoreFloat3(&playerPos, PlayerPOS);
		intersectPos.y = playerPos.y + m_ObjectRef->m_TransformInfo->m_Bound.Extents.y;
		XMVECTOR PickedPos = XMLoadFloat3(&intersectPos);
		XMVECTOR PlayerNewLOOK = XMVector3Normalize(PickedPos - PlayerPOS);
		XMVECTOR Z_Axis = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		XMFLOAT3 Dot; XMStoreFloat3(&Dot, XMVector3Dot(PlayerNewLOOK, Z_Axis));
		XMFLOAT3 playerNewLook; XMStoreFloat3(&playerNewLook, PlayerNewLOOK);
		float rad2deg = 180.0f / MathHelper::Pi;
		PlayerWorldEuler.y = atanf(playerNewLook.x / (playerNewLook.z)) * rad2deg;
		if (Dot.x < 0.0f)// PickedPos가 Player 뒤에 있을 때
		{
			if (PlayerWorldEuler.y < 0.0f) // PickedPos가 우측후방에 있을때
				PlayerWorldEuler.y += 180.0f;
			else if (PlayerWorldEuler.y > 0.0f) // PickedPos가 좌측후방에 있을때
				PlayerWorldEuler.y -= 180.0f;
		}
		m_ObjectRef->m_TransformInfo->SetWorldRotationEuler(PlayerWorldEuler);
		m_ObjectRef->m_TransformInfo->UpdateWorldTransform();

		if (AllObjectsRef != nullptr && WorldObjectsRef != nullptr && AllRitemsRef != nullptr && CurrSkillObjInstanceNUMRef != nullptr)
		{
			ObjectManager objManager;

			auto newSkillEffectObj = objManager.FindDeactiveWorldObject(*AllObjectsRef, *WorldObjectsRef, MaxCreateWorldObj);
			if (newSkillEffectObj == nullptr)
			{
				MessageBox(NULL, L"No equipment object available in the world object list.", L"Object Generate Warning", MB_OK);
				return;
			}

			std::string objName = "CrossTarget - Instancing" + std::to_string(++(*CurrSkillObjInstanceNUMRef));
			XMFLOAT3 WorldPosition; XMStoreFloat3(&WorldPosition, PickedPos);
			WorldPosition.y = 20.0f;
			objManager.SetObjectComponent(newSkillEffectObj, objName,
				(*AllRitemsRef)["PickingEffect_CrossTarget"].get(), nullptr,
				nullptr, nullptr, nullptr,
				nullptr, nullptr, &WorldPosition);

			newSkillEffectObj->DeActivatedTime = 1.0f;
			newSkillEffectObj->DeActivatedDecrease = 1.0f;
			newSkillEffectObj->SelfDeActivated = true;
			newSkillEffectObj->DisappearForDeAcTime = true;
		}
	}
}

void Player::ProcessSkeletonAnimDurationDone()
{
	if ((m_CurrAction & ActionType::Attacking) == ActionType::Attacking)
	{
		std::string objName = m_ObjectRef->m_Name;
		if (objName.find("Meshtint Free Knight") != std::string::npos)
		{
			auto& animInfo = m_ObjectRef->m_SkeletonInfo->m_AnimInfo;
			std::string currAnimName = animInfo->CurrPlayingAnimName;

			if (currAnimName == "Meshtint Free Knight@Sword And Shield Slash")
			{
				bool AnimIsSetted = true;

				bool AnimNotifyIsSetted = false;
				if (animInfo->CheckAnimTimeLineNotify("Meshtint Free Knight@Sword And Shield Slash-SlashGen", AnimNotifyIsSetted) == true)
				{
					if(AllObjectsRef != nullptr && WorldObjectsRef != nullptr && AllRitemsRef != nullptr && CurrSkillObjInstanceNUMRef != nullptr)
						Player::CreateSkillObject(*AllObjectsRef, *WorldObjectsRef, MaxCreateWorldObj, *AllRitemsRef, *CurrSkillObjInstanceNUMRef);
				}

				if (animInfo->AnimOnceDone(currAnimName, AnimIsSetted) == true)
				{
					animInfo->AnimStop(currAnimName);
					animInfo->AnimPlay("Meshtint Free Knight@Battle Idle");
					animInfo->AnimLoop("Meshtint Free Knight@Battle Idle");
					m_CurrAction = ActionType::Idle;
				}
			}
		}
	}
}

void Player::CreateSkillObject(std::vector<std::unique_ptr<Object>>& AllObjects,
	std::vector<Object*>& WorldObjects,
	const UINT MaxWorldObj,
	std::unordered_map<std::string, std::unique_ptr<RenderItem>>& AllRitems,
	UINT& CurrSkillObjInstanceNUM)
{
	std::string objName = m_ObjectRef->m_Name;
	if (objName.find("Meshtint Free Knight") != std::string::npos)
	{
		XMVECTOR PlayerPOS = XMLoadFloat3(&m_ObjectRef->m_TransformInfo->GetWorldPosition());
		XMFLOAT3 PlayerWorldEuler = m_ObjectRef->m_TransformInfo->GetWorldEuler();		
		XMFLOAT3 SpawnWorldEuler[3] =
		{
			PlayerWorldEuler,
			{ PlayerWorldEuler.x, PlayerWorldEuler.y + 25.0f, PlayerWorldEuler.z + 30.0f },
			{ PlayerWorldEuler.x, PlayerWorldEuler.y - 25.0f, PlayerWorldEuler.z - 30.0f }
		};

		ObjectManager objManager;
		for (int i = 0; i < 3; ++i)
		{
			auto newSkillEffectObj = objManager.FindDeactiveWorldObject(AllObjects, WorldObjects, MaxWorldObj);
			if (newSkillEffectObj == nullptr)
			{
				MessageBox(NULL, L"No equipment object available in the world object list.", L"Object Generate Warning", MB_OK);
				return;
			}

			XMFLOAT3 WorldScale = { 1.0f, 1.0f, 1.0f };
			XMFLOAT3 WorldRotationEuler = SpawnWorldEuler[i];
			std::string objName = "SwordSlash - Instancing" + std::to_string(++CurrSkillObjInstanceNUM);
			objManager.SetObjectComponent(newSkillEffectObj, objName,
				AllRitems["SkillEffect_SwordSlash_a"].get(), nullptr,
				nullptr, nullptr, nullptr,
				&WorldScale, &WorldRotationEuler, nullptr);
			newSkillEffectObj->m_TransformInfo->UpdateWorldTransform();

			XMVECTOR SpawnLOOK = XMLoadFloat3(&newSkillEffectObj->m_TransformInfo->GetLook());
			XMVECTOR SpawnPOS = PlayerPOS + (SpawnLOOK * 320.0f) + XMVectorSet(0.0f, 30.0f, 0.0f, 0.0f);
			XMFLOAT3 WorldPosition; XMStoreFloat3(&WorldPosition, SpawnPOS);
			newSkillEffectObj->m_TransformInfo->SetWorldPosition(WorldPosition);

			XMFLOAT3 newVelocity; XMStoreFloat3(&newVelocity, SpawnLOOK);
			newVelocity.x *= CharacterSpeed::MeshtintFreeKnightSpeed * 5.5f;
			newVelocity.y *= CharacterSpeed::MeshtintFreeKnightSpeed * 5.5f;
			newVelocity.z *= CharacterSpeed::MeshtintFreeKnightSpeed * 5.5f;

			newSkillEffectObj->m_TransformInfo->SetVelocity(newVelocity);

			newSkillEffectObj->DeActivatedTime = 10.0f;
			newSkillEffectObj->DeActivatedDecrease = 10.0f;
			newSkillEffectObj->SelfDeActivated = true;
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
