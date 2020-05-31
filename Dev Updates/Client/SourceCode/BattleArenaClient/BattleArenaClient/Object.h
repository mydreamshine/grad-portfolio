#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <string>

#include "FrameResource.h"
#include "Common/FileLoader/ModelLoader.h"
#include "Common/Util/d3d12/MathHelper.h"
#include "Common/Timer/Timer.h"
#include "Common/FileLoader/SpriteFontLoader.h"

#define MAX_WORLD_OBJECT 1024
#define MAX_CHARACTER_OBJECT 100

// rel. UI
struct TextInfo
{
	std::wstring m_FontName;
	DirectX::XMFLOAT2 m_TextPos = { 0.0f, 0.0f };
	DirectX::XMVECTOR m_TextColor = DirectX::Colors::Black;
	std::wstring m_Text;
	// 렌더링할 텍스트마다 SpriteBatch를 지정해줘야 하기에
	// 각 텍스트는 고유한 SpriteBatchIndex를 지닌다.
	UINT TextBatchIndex = 0;

	void Init()
	{
		m_FontName.clear();
		m_TextPos = { 0.0f, 0.0f };
		m_TextColor = DirectX::Colors::Black;
		m_Text.clear();
	}
};

// rel. SkinnedConstatns
struct SkeletonInfo
{
	// SkinnedCB rel. data
	int NumSkinnedCBDirty = gNumFrameResources;
	aiModelData::aiSkeleton* m_Skeleton = nullptr;
	std::unique_ptr<aiModelData::AnimInfo> m_AnimInfo = nullptr;

	// Index into GPU constant buffer corresponding to the SkinndCB for this render item.
	UINT SkinCBIndex = -1;

	void Init()
	{
		NumSkinnedCBDirty = gNumFrameResources;
		m_Skeleton = nullptr;
		if(m_AnimInfo) m_AnimInfo->Init();
	}
};

// rel. ObjectConstants
// TransformInfo를 업데이트 할 때마다
// 반드시 NumObjectCBDirty = gNumFrameResources;
// 해줘야 ObjectCB내에 해당 ObjectConstants가 변경된다.
struct TransformInfo
{
	// ObjectCB rel. data
	int NumObjectCBDirty = gNumFrameResources;
	DirectX::XMFLOAT4X4 m_WorldTransform = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_TexTransform = MathHelper::Identity4x4();
	float m_TexAlpha = 1.0f;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;

	// AABB
	// Center는 m_WorldPosition에 영향을 받는다.
	DirectX::BoundingBox m_Bound;
	DirectX::BoundingBox m_OriginBound;
	enum class BoundPivot { Center, Bottom };
	BoundPivot m_BoundPivot = BoundPivot::Center;

	// 렌더링에서만 쓰임
	// 정확히는 Update ObjectConstants를 할 때
	// ObjectConstants의 WorldM에 m_WorldTransform * m_LocalTransform을 하여
	// WorldM을 지정해줌.
	DirectX::XMFLOAT4X4 m_LocalTransform = MathHelper::Identity4x4();

	DirectX::XMFLOAT3 m_WorldPosition = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4 m_WorldRotationQut = { 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 m_WorldRotationEuler = { 0.0f, 0.0f, 0.0f }; // degree angle
	DirectX::XMFLOAT3 m_WorldScale = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 m_LocalPosition = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4 m_LocalRotationQut = { 0.0f, 0.0f, 0.0f, 1.0f }; // degree angle;
	DirectX::XMFLOAT3 m_LocalRotationEuler = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_LocalScale = { 1.0f, 1.0f, 1.0f };

	DirectX::XMFLOAT3 m_Right = { 1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_Up = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 m_Look = { 0.0f, 0.0f, 1.0f };

	DirectX::XMFLOAT3 m_Velocity = { 0.0f, 0.0f, 0.0f };

	int m_AttachingTargetBoneID = -1;
	/*
	AttachingTargetBoneID는 StaticMesh에 대해서만 사용한다.
	StaticMesh의 탈착의 구현은
	해당 메쉬의	모든 버텍스의 BoneID를 동일하게 하고
	BoneWeight를 1로 하면 구현 가능하다.
	(그렇다고 런타임 중에 모든 버텍스에 대해 값을 바꾸는 것은
	버텍스자체를 const로 여기고 해당 버텍스들을 참조하는
	지금의 구조에 적합하지 않은 방법이다.
	어차피 Attach를 하면 해당 메쉬의 BoneID가 모든 버텍스마다 동일하기 때문에
	쉐이더에서 BoneID 1개만 전달받게 하면 모든 버텍스들에 대한
	BoneAnimation계산을 할 수 있다.
	그러므로 ObjectConstant에 AttachingTargetBoneID를 추가하고
	쉐이더에서 해당 AttachingTargetBoneID 파라미터에 대한 BoneAnimation계산만
	버텍스별로 동일하게 해주면 된다.)

	하지만 SkinnedMesh는 버텍스별로 BoneID와 BoneWeight가 다르므로
	런타임 중에 이를 지정해주기가 상당히 까다롭다.
	SkinnedMesh에 대한 Attach 구현 컨셉으로는 다음과 같다.
		"만약 Attach가 되는 대상의 Skeleton이
		SkinnedMesh를 처음 임포트했을 때의 Skeleton과 유사하거나 같다면
		해당 SkinnedMesh의 BoneID와 BoneWeight를 지정해 줄 수 있지만
		Attach되는 Skeleton이 SkinneMesh 초기 임포트 할 때의 Skeleton과
		너무 다르다면 Attach를 할 수 없을 것이다."

	이를 구현하려면 Skeleton의 유사성을 계산해야 하는 데
	그 방법을 구현하기가 어려우므로,
	결국 AttachingTargetBoneID는 StaticMesh에 대해서만 사용하기로 한다.
	*/

	void SetBound(const DirectX::BoundingBox& newBound, BoundPivot pivot);

	void SetWorldScale(const DirectX::XMFLOAT3& newScale);
	// degree angle
	// roll: +z축을 기준으로 쉐이더 내에서 시계반대방향으로 회전
	// pitch: +x축을 기준으로 쉐이더 내에서 시계방향으로 회전
	// yaw: +y축을 기준으로 쉐이더 내에서 시계방향으로 회전
	// 쉐이더에 WorldTransform을 전치시켜서 넘겨주므로 회전방향이 반대.
	// 즉 쉐이더 내에선 왼손좌표계 회전.
	// 쉐이더 넘기기 전 전치하지 않은 WorldTransform은 오른손좌표계의 트랜스폼.
	void SetWorldRotationEuler(const DirectX::XMFLOAT3& newRot);
	void SetWorldPosition(const DirectX::XMFLOAT3& newPos);

	void SetLocalScale(const DirectX::XMFLOAT3& newScale);
	// degree angle
	// roll: +z축을 기준으로 시계반대방향으로 회전
	// pitch: +x축을 기준으로 시계방향으로 회전
	// yaw: +y축을 기준으로 시계방향으로 회전
	void SetLocalRotationEuler(const DirectX::XMFLOAT3& newRot);
	void SetLocalPosition(const DirectX::XMFLOAT3& newPos);

	void SetWorldTransform(const DirectX::XMFLOAT3& newScale, const DirectX::XMFLOAT3& newRotationEuler, const DirectX::XMFLOAT3& newPosition);
	void SetWorldTransform(const DirectX::XMFLOAT4X4& newTransform);
	void SetLocalTransform(const DirectX::XMFLOAT3& newScale, const DirectX::XMFLOAT3& newRotationEuler, const DirectX::XMFLOAT3& newPosition);
	void SetLocalTransform(const DirectX::XMFLOAT4X4& newTransform);

	void SetVelocity(const DirectX::XMFLOAT3& newVelocity);

	void UpdateBound();
	void UpdateWorldTransform();
	void UpdateLocalTransform();
	// 쉐이더에 WorldTransform을 전치시켜 전달하기 때문에
	// 쉐이더 내에선 기저벡터가 WorldTransform과는 다르다.
	// 이를 감안하여 GetRight, GetUp, GetLook은
	// 쉐이더에 넘겨진 (전치된) WorldTransform을 기준으로 한다.
	// 즉, WorldTransform을 전치시킨 Transform을 기준으로
	// 기저벡터를 계산한다.
	void UpdateBaseAxis();
	void Animate(CTimer& gt);

	DirectX::XMFLOAT4X4 GetWorldTransform();
	DirectX::XMFLOAT4X4 GetLocalTransform();

	DirectX::XMFLOAT3 GetWorldScale() { return m_WorldScale; }
	DirectX::XMFLOAT3 GetLocalScale() { return m_LocalScale; }

	DirectX::XMFLOAT3 GetWorldEuler() { return m_WorldRotationEuler; }
	DirectX::XMFLOAT3 GetLocalEuler() { return m_LocalRotationEuler; }

	DirectX::XMFLOAT3 GetWorldPosition() { return m_WorldPosition; }
	DirectX::XMFLOAT3 GetLocalPosition() { return m_LocalPosition; }

	// 쉐이더에 WorldTransform을 전치시켜 전달하기 때문에
	// 쉐이더 내에선 기저벡터가 WorldTransform과는 다르다.
	// 이를 감안하여 GetRight, GetUp, GetLook은
	// 쉐이더에 넘겨진 (전치된) WorldTransform을 기준으로 한다.
	// 즉, WorldTransform을 전치시킨 Transform을 기준으로
	// 기저벡터를 계산한다.
	DirectX::XMFLOAT3 GetRight();
	DirectX::XMFLOAT3 GetUp();
	DirectX::XMFLOAT3 GetLook();

	void Init();
};


// Object 1 : RenderItem 1
struct Object
{
	std::string m_Name;

	Object* m_Parent = nullptr;
	std::vector<Object*> m_Childs;

	// rel. ObjectConstants
	std::unique_ptr<TransformInfo> m_TransformInfo = nullptr;
	// rel. SkinnedConstants
	std::unique_ptr<SkeletonInfo> m_SkeletonInfo = nullptr;
	// rel. UI (only used in UI Object)
	std::unordered_map<std::string, std::unique_ptr<TextInfo>> m_UIinfos;

	// rel. MaterialConstants
	// Ritem에 대해선 인스턴싱을 하지 않으므로
	// Object별로 unique하게 다룰 필욘 없다.
	RenderItem*   m_RenderItem = nullptr;

	bool Activated = false;
	bool SelfDeActivated = false;
	float DeActivatedTime = 0.0f;
	float DeActivatedDecrease = 0.0f;
	bool DisappearForDeAcTime = false; // DeActivatedTime동안 Alpha값이 서서히 줄어들게 하는 플래그

	bool ProcessSelfDeActivate(CTimer& gt);
};

class ObjectManager
{
private:
	enum class OBJ_TYPE
	{
		nonCharacterObj = 0,
		CharacterObj = 1
	};
	Object* FindDeactiveObject(
		std::vector<std::unique_ptr<Object>>& GenDestList,
		std::vector<Object*>& SearchList,
		const UINT GenerateMaximum, OBJ_TYPE objType = OBJ_TYPE::nonCharacterObj )
	{
		Object* retObj = nullptr;

		for (auto& obj : SearchList)
		{
			if (obj->Activated == false)
			{
				if (objType == OBJ_TYPE::nonCharacterObj && obj->m_SkeletonInfo != nullptr)
					continue;
				else if (objType == OBJ_TYPE::CharacterObj && obj->m_SkeletonInfo == nullptr)
					continue;

				obj->Activated = true;
				retObj = obj;
				break;
			}
		}

		if (retObj == nullptr && (UINT)SearchList.size() < GenerateMaximum)
		{
			auto newObj = std::make_unique<Object>();
			newObj->Activated = true;

			retObj = newObj.get();
			SearchList.push_back(newObj.get());
			GenDestList.push_back(std::move(newObj));
		}

		return retObj;
	}
public:
	// 캐릭터 오브젝트는 월드오브젝트이면서 동시에 Skinned 오브젝트이므로
	// FindDeactiveObject를 월드오브젝트리스트에서 찾는다.
	Object* FindDeactiveCharacterObject(
		std::vector<std::unique_ptr<Object>>& AllObjects,
		std::vector<Object*>& CharacterObjects,
		const UINT MaxCharacterObj,
		std::vector<Object*>& WorldObjects,
		const UINT MaxWorldObj)
	{
		Object* retObj = nullptr;

		for (auto& obj : CharacterObjects)
		{
			if (obj->Activated == false)
			{
				obj->Activated = true;
				retObj = obj;
				break;
			}
		}

		if (retObj == nullptr && CharacterObjects.size() < MaxCharacterObj)
		{
			retObj = ObjectManager::FindDeactiveObject(AllObjects, WorldObjects, MaxWorldObj, OBJ_TYPE::CharacterObj);
			if (retObj != nullptr) CharacterObjects.push_back(retObj);
		}

		return retObj;
	}

	Object* FindDeactiveWorldObject(
		std::vector<std::unique_ptr<Object>>& AllObjects,
		std::vector<Object*>& WorldObjects,
		const UINT MaxWorldObj)
	{
		return ObjectManager::FindDeactiveObject(AllObjects, WorldObjects, MaxWorldObj, OBJ_TYPE::nonCharacterObj);
	}

	Object* FindDeactiveUIObject(
		std::vector<std::unique_ptr<Object>>& AllObjects,
		std::vector<Object*>& UIObjects,
		const UINT MaxUIObj)
	{
		return ObjectManager::FindDeactiveObject(AllObjects, UIObjects, MaxUIObj, OBJ_TYPE::nonCharacterObj);
	}

	bool SetAttaching(Object* Source, Object* Dest, const std::string& AttachingTargetBoneName)
	{
		if (Source->m_TransformInfo == nullptr || Dest->m_SkeletonInfo == nullptr)
		{
			throw std::invalid_argument("TransformInfo or SkeletonInfo does not exist");
			return false;
		}

		auto objInfo_src = Source->m_TransformInfo.get();
		auto skeleton_deset = Dest->m_SkeletonInfo->m_Skeleton;
		auto& AttachingTargetBoneID = objInfo_src->m_AttachingTargetBoneID;
		// Attaching 본 ID 지정
		AttachingTargetBoneID = skeleton_deset->FindJointIndex(AttachingTargetBoneName);

		// 계층 관계 지정
		// Attaching인 됬을 경우에만 지정한다.
		if (AttachingTargetBoneID != -1)
		{
			Source->m_Parent = Dest;
			Dest->m_Childs.push_back(Source);
			return true;
		}
		else return false;
	}

	void SetObjectComponent(Object* obj,
		const std::string& Name,
		RenderItem* Ritem = nullptr,
		aiModelData::aiSkeleton* Skeleton = nullptr,
		const DirectX::XMFLOAT3* LocalScale = nullptr,
		const DirectX::XMFLOAT3* LocalRotationEuler = nullptr,
		const DirectX::XMFLOAT3* LocalPosition = nullptr,
		const DirectX::XMFLOAT3* WorldScale = nullptr,
		const DirectX::XMFLOAT3* WorldRotationEuler = nullptr,
		const DirectX::XMFLOAT3* WorldPosition = nullptr)
	{
		obj->m_Name = Name;
		obj->m_RenderItem = Ritem;

		if (obj->m_SkeletonInfo != nullptr)
			obj->m_SkeletonInfo->m_Skeleton = Skeleton;

		if (obj->m_TransformInfo != nullptr)
		{
			auto ObjInfo = obj->m_TransformInfo.get();
			ObjInfo->SetBound(Ritem->Geo->DrawArgs[Ritem->Name].Bounds, TransformInfo::BoundPivot::Center);
			if (LocalScale)
			{
				DirectX::XMFLOAT3 L_S = *LocalScale;
				ObjInfo->SetLocalScale(L_S);
			}
			if (LocalRotationEuler)
			{
				DirectX::XMFLOAT3 L_R_Euler = *LocalRotationEuler;
				ObjInfo->SetLocalRotationEuler(L_R_Euler);
			}
			if (LocalPosition)
			{
				DirectX::XMFLOAT3 L_T = *LocalPosition;
				ObjInfo->SetLocalPosition(L_T);
			}
			if (WorldScale)
			{
				DirectX::XMFLOAT3 W_S = *WorldScale;
				ObjInfo->SetWorldScale(W_S);
			}
			if (WorldRotationEuler)
			{
				DirectX::XMFLOAT3 W_R = *WorldRotationEuler;
				ObjInfo->SetWorldRotationEuler(W_R);
			}
			if (WorldPosition)
			{
				DirectX::XMFLOAT3 W_T = *WorldPosition;
				ObjInfo->SetWorldPosition(W_T);
			}
		}
	}

	Object* CreateCharacterObject(
		int objCB_index,
		int skinnedCB_index,
		std::vector<std::unique_ptr<Object>>& AllObjects,
		std::vector<Object*>& CharacterObjects,
		const UINT MaxCharacterObj,
		std::vector<Object*>& WorldObjects,
		const UINT MaxWorldObj)
	{
		Object* retObj = ObjectManager::FindDeactiveCharacterObject(
			AllObjects,
			CharacterObjects, MaxCharacterObj,
			WorldObjects, MaxWorldObj);

		if (retObj != nullptr)
		{
			retObj->m_TransformInfo = std::make_unique<TransformInfo>();
			retObj->m_TransformInfo->ObjCBIndex = objCB_index;
			retObj->m_SkeletonInfo = std::make_unique<SkeletonInfo>();
			retObj->m_SkeletonInfo->SkinCBIndex = skinnedCB_index;
			retObj->m_SkeletonInfo->m_AnimInfo = std::make_unique<aiModelData::AnimInfo>();
		}

		return retObj;
	}

	Object* CreateWorldObject(
		int objCB_index,
		std::vector<std::unique_ptr<Object>>& AllObjects,
		std::vector<Object*>& WorldObjects,
		const UINT MaxWorldObj)
	{
		Object* retObj = ObjectManager::FindDeactiveWorldObject(
			AllObjects,
			WorldObjects, MaxWorldObj);

		if (retObj != nullptr)
		{
			retObj->m_TransformInfo = std::make_unique<TransformInfo>();
			retObj->m_TransformInfo->ObjCBIndex = objCB_index;
		}

		return retObj;
	}

	Object* CreateUIObject(
		int objCB_index,
		std::vector<std::unique_ptr<Object>>& AllObjects,
		std::vector<Object*>& UIObjects,
		const UINT MaxUIObj)
	{
		Object* retObj = ObjectManager::FindDeactiveUIObject(
			AllObjects,
			UIObjects, MaxUIObj);

		if (retObj != nullptr)
		{
			retObj->m_TransformInfo = std::make_unique<TransformInfo>();
			retObj->m_TransformInfo->ObjCBIndex = objCB_index;
		}

		return retObj;
	}

	void DeActivateObj(Object* obj)
	{
		obj->m_Name.clear();
		obj->m_Parent = nullptr;
		obj->m_Childs.clear();
		obj->m_RenderItem = nullptr;
		obj->Activated = false;
		obj->SelfDeActivated = false;
		obj->DeActivatedTime = 0.0f;
		obj->DeActivatedDecrease = 0.0f;
		obj->DisappearForDeAcTime = false;
		if (obj->m_TransformInfo != nullptr)
			obj->m_TransformInfo->Init();
		if (obj->m_SkeletonInfo != nullptr)
			obj->m_SkeletonInfo->Init();
		if (obj->m_UIinfos.empty() != true)
		{
			for (auto& textInfo_iter : obj->m_UIinfos)
				textInfo_iter.second->Init();
		}
	}
};
