#pragma once
#include <memory>
#include <vector>
#include <unordered_map>

#include "FrameResource.h"
#include "Common/FileLoader/ModelLoader.h"
#include "Common/Util/d3d12/MathHelper.h"

enum class ObjectType : int
{
	CharacterGeo = 0,
	LandGeo,
	Obstacle, // 장애물
	Destructible, // 파괴 가능한 장애물
	Equipment,
	DropItem,
	Effect_quad,
	UI
};

// Object 1 : Geometric 1 (UI는 SubMesh별로.)
struct Object
{
	std::string m_Name;
	ObjectType  m_Type;

	std::unordered_map<std::string, RenderItem*> m_RenderItems;

	// ObjectCB rel. data
	int NumObjectCBDirty = gNumFrameResources;
	DirectX::XMFLOAT4X4 m_WorldTransform = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_TexTransform = MathHelper::Identity4x4();

	// SkinnedCB rel. data
	aiModelData::aiSkeleton* m_Skeleton = nullptr;
	std::unique_ptr<aiModelData::AnimInfo> m_AnimInfo = nullptr;

	DirectX::BoundingBox m_Collider; // AABB

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;
	// Index into GPU constant buffer corresponding to the SkinndCB for this render item.
	UINT SkinCBIndex = -1;

	bool Activated = false;
};

class ObjectManager
{
public:
	Object* GenerateWorldObject(
		std::vector<std::unique_ptr<Object>>& AllObjects,
		std::vector<Object*>& WorldObjects,
		const UINT MaxWorldObj)
	{
		Object* retObj = nullptr;

		for (auto& world_obj : WorldObjects)
		{
			if (world_obj->Activated == false)
			{
				world_obj->Activated = true;
				retObj = world_obj;
				break;
			}
		}

		if (retObj == nullptr && (UINT)WorldObjects.size() < MaxWorldObj)
		{
			auto newObj = std::make_unique<Object>();
			newObj->Activated = true;

			retObj = newObj.get();
			WorldObjects.push_back(newObj.get());
			AllObjects.push_back(std::move(newObj));
		}

		return retObj;
	}

	// SkinnedCB를 지닌(Skeleton이 있는) 오브젝트는 동시에 WorldObject이므로
	// ObjCB도 지니게 된다.
	Object* GenerateSkinnedObject(
		std::vector<std::unique_ptr<Object>>& AllObjects,
		std::vector<Object*>& SkinnedObjects,
		std::vector<Object*>& WorldObjects,
		const UINT MaxSkinnedObj,
		const UINT MaxWorldObj)
	{
		Object* retObj = nullptr;

		for (auto& skinned_obj : SkinnedObjects)
		{
			if (skinned_obj->Activated == false)
			{
				skinned_obj->Activated = true;
				retObj = skinned_obj;
				break;
			}
		}

		if (retObj == nullptr
			&& (UINT)SkinnedObjects.size() < MaxSkinnedObj
			&& (UINT)WorldObjects.size() < MaxWorldObj)
		{
			auto newObj = std::make_unique<Object>();
			newObj->Activated = true;

			retObj = newObj.get();
			SkinnedObjects.push_back(newObj.get());
			WorldObjects.push_back(newObj.get());
			AllObjects.push_back(std::move(newObj));
		}

		return retObj;
	}

	bool DeActivateObj(Object* obj)
	{
		obj->m_Name = "";
		obj->NumObjectCBDirty = 0;
		obj->m_WorldTransform = MathHelper::Identity4x4();
		obj->m_TexTransform = MathHelper::Identity4x4();
		if(obj->m_AnimInfo != nullptr) obj->m_AnimInfo->Init();
		obj->m_RenderItems.clear();
		obj->m_Skeleton = nullptr;
		obj->Activated = false;
	}
};