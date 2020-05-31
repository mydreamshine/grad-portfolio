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
	// �������� �ؽ�Ʈ���� SpriteBatch�� ��������� �ϱ⿡
	// �� �ؽ�Ʈ�� ������ SpriteBatchIndex�� ���Ѵ�.
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
// TransformInfo�� ������Ʈ �� ������
// �ݵ�� NumObjectCBDirty = gNumFrameResources;
// ����� ObjectCB���� �ش� ObjectConstants�� ����ȴ�.
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
	// Center�� m_WorldPosition�� ������ �޴´�.
	DirectX::BoundingBox m_Bound;
	DirectX::BoundingBox m_OriginBound;
	enum class BoundPivot { Center, Bottom };
	BoundPivot m_BoundPivot = BoundPivot::Center;

	// ������������ ����
	// ��Ȯ���� Update ObjectConstants�� �� ��
	// ObjectConstants�� WorldM�� m_WorldTransform * m_LocalTransform�� �Ͽ�
	// WorldM�� ��������.
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
	AttachingTargetBoneID�� StaticMesh�� ���ؼ��� ����Ѵ�.
	StaticMesh�� Ż���� ������
	�ش� �޽���	��� ���ؽ��� BoneID�� �����ϰ� �ϰ�
	BoneWeight�� 1�� �ϸ� ���� �����ϴ�.
	(�׷��ٰ� ��Ÿ�� �߿� ��� ���ؽ��� ���� ���� �ٲٴ� ����
	���ؽ���ü�� const�� ����� �ش� ���ؽ����� �����ϴ�
	������ ������ �������� ���� ����̴�.
	������ Attach�� �ϸ� �ش� �޽��� BoneID�� ��� ���ؽ����� �����ϱ� ������
	���̴����� BoneID 1���� ���޹ް� �ϸ� ��� ���ؽ��鿡 ����
	BoneAnimation����� �� �� �ִ�.
	�׷��Ƿ� ObjectConstant�� AttachingTargetBoneID�� �߰��ϰ�
	���̴����� �ش� AttachingTargetBoneID �Ķ���Ϳ� ���� BoneAnimation��길
	���ؽ����� �����ϰ� ���ָ� �ȴ�.)

	������ SkinnedMesh�� ���ؽ����� BoneID�� BoneWeight�� �ٸ��Ƿ�
	��Ÿ�� �߿� �̸� �������ֱⰡ ����� ��ٷӴ�.
	SkinnedMesh�� ���� Attach ���� �������δ� ������ ����.
		"���� Attach�� �Ǵ� ����� Skeleton��
		SkinnedMesh�� ó�� ����Ʈ���� ���� Skeleton�� �����ϰų� ���ٸ�
		�ش� SkinnedMesh�� BoneID�� BoneWeight�� ������ �� �� ������
		Attach�Ǵ� Skeleton�� SkinneMesh �ʱ� ����Ʈ �� ���� Skeleton��
		�ʹ� �ٸ��ٸ� Attach�� �� �� ���� ���̴�."

	�̸� �����Ϸ��� Skeleton�� ���缺�� ����ؾ� �ϴ� ��
	�� ����� �����ϱⰡ �����Ƿ�,
	�ᱹ AttachingTargetBoneID�� StaticMesh�� ���ؼ��� ����ϱ�� �Ѵ�.
	*/

	void SetBound(const DirectX::BoundingBox& newBound, BoundPivot pivot);

	void SetWorldScale(const DirectX::XMFLOAT3& newScale);
	// degree angle
	// roll: +z���� �������� ���̴� ������ �ð�ݴ�������� ȸ��
	// pitch: +x���� �������� ���̴� ������ �ð�������� ȸ��
	// yaw: +y���� �������� ���̴� ������ �ð�������� ȸ��
	// ���̴��� WorldTransform�� ��ġ���Ѽ� �Ѱ��ֹǷ� ȸ�������� �ݴ�.
	// �� ���̴� ������ �޼���ǥ�� ȸ��.
	// ���̴� �ѱ�� �� ��ġ���� ���� WorldTransform�� ��������ǥ���� Ʈ������.
	void SetWorldRotationEuler(const DirectX::XMFLOAT3& newRot);
	void SetWorldPosition(const DirectX::XMFLOAT3& newPos);

	void SetLocalScale(const DirectX::XMFLOAT3& newScale);
	// degree angle
	// roll: +z���� �������� �ð�ݴ�������� ȸ��
	// pitch: +x���� �������� �ð�������� ȸ��
	// yaw: +y���� �������� �ð�������� ȸ��
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
	// ���̴��� WorldTransform�� ��ġ���� �����ϱ� ������
	// ���̴� ������ �������Ͱ� WorldTransform���� �ٸ���.
	// �̸� �����Ͽ� GetRight, GetUp, GetLook��
	// ���̴��� �Ѱ��� (��ġ��) WorldTransform�� �������� �Ѵ�.
	// ��, WorldTransform�� ��ġ��Ų Transform�� ��������
	// �������͸� ����Ѵ�.
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

	// ���̴��� WorldTransform�� ��ġ���� �����ϱ� ������
	// ���̴� ������ �������Ͱ� WorldTransform���� �ٸ���.
	// �̸� �����Ͽ� GetRight, GetUp, GetLook��
	// ���̴��� �Ѱ��� (��ġ��) WorldTransform�� �������� �Ѵ�.
	// ��, WorldTransform�� ��ġ��Ų Transform�� ��������
	// �������͸� ����Ѵ�.
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
	// Ritem�� ���ؼ� �ν��Ͻ��� ���� �����Ƿ�
	// Object���� unique�ϰ� �ٷ� �ʿ� ����.
	RenderItem*   m_RenderItem = nullptr;

	bool Activated = false;
	bool SelfDeActivated = false;
	float DeActivatedTime = 0.0f;
	float DeActivatedDecrease = 0.0f;
	bool DisappearForDeAcTime = false; // DeActivatedTime���� Alpha���� ������ �پ��� �ϴ� �÷���

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
	// ĳ���� ������Ʈ�� ���������Ʈ�̸鼭 ���ÿ� Skinned ������Ʈ�̹Ƿ�
	// FindDeactiveObject�� ���������Ʈ����Ʈ���� ã�´�.
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
		// Attaching �� ID ����
		AttachingTargetBoneID = skeleton_deset->FindJointIndex(AttachingTargetBoneName);

		// ���� ���� ����
		// Attaching�� ���� ��쿡�� �����Ѵ�.
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
