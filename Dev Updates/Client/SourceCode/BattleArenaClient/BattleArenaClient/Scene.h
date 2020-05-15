#pragma once

#include "DXSample.h"
#include "FrameResource.h"
#include "Object.h"
#include "Common/Util/d3d12/GeometryGenerator.h"
#include "Common/Util/d3d12/Camera.h"
#include "Common/Util/d3d12/ShadowMap.h"
#include "Common/FileLoader/ModelLoader.h"
#include "Common/Timer/Timer.h"
#include "Common/FileLoader/TextureLoader.h"
#include "Player.h"

using namespace DirectX;

using Microsoft::WRL::ComPtr;

class Scene
{
public:
    Scene() = default;
    Scene(UINT width, UINT height);
    virtual ~Scene();

    virtual void OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
        int& objCB_index, int& skinnedCB_index, int& textBatch_index);
    virtual void OnInitProperties(CTimer& gt) = 0;
    virtual void OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
        const bool key_state[], const POINT& oldCursorPos,
        const RECT& ClientRect,
        CTimer& gt);

public:
    virtual void SetGeometriesRef(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>* Geometries) { m_GeometriesRef = Geometries; }
    virtual void SetMaterialsRef(std::unordered_map<std::string, std::unique_ptr<Material>>* Materials) { m_MaterialsRef = Materials; }
    virtual void SetTexturesRef(std::unordered_map<std::string, std::unique_ptr<Texture>>* Textures) { m_TexturesRef = Textures; }
    virtual void SetModelSkeletonsRef(std::unordered_map<std::string, std::unique_ptr<aiModelData::aiSkeleton>>* Skeletons) { m_ModelSkeltonsRef = Skeletons; }
    virtual void SetRederItemsRef(std::unordered_map<std::string, std::unique_ptr<RenderItem>>* RenderItems) { m_AllRitemsRef = RenderItems; }
    virtual void BuildObjects(int& objCB_index, int& skinnedCB_index, int& textBatch_index) = 0;

public:
    virtual void UpdateObjectCBs(UploadBuffer<ObjectConstants>* objCB, CTimer& gt);
    void aiM2dxM(XMFLOAT4X4& dest, const aiMatrix4x4& source);
    virtual void UpdateSkinnedCBs(UploadBuffer<SkinnedConstants>* skinnedCB, CTimer& gt);
    virtual void UpdateMaterialCBs(UploadBuffer<MaterialConstants>* matCB, CTimer& gt);
    virtual void UpdateMainPassCB(UploadBuffer<PassConstants>* passCB, CTimer& gt);
    virtual void UpdateShadowPassCB(UploadBuffer<PassConstants>* passCB, ShadowMap* shadow_map, CTimer& gt);
    virtual void UpdateShadowTransform(CTimer& gt);
    virtual void UpdateTextInfo(CTimer& gt) = 0;
    virtual void AnimateLights(CTimer& gt) = 0;
    virtual void AnimateSkeletons(CTimer& gt) = 0;
    virtual void AnimateCameras(CTimer& gt) = 0;
    
public:
    virtual void ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt) = 0;

public:
    const PassConstants& GetMainPassCB() { return m_MainPassCB; }
    const PassConstants& GetShadowPassCB() { return m_ShadowPassCB; }

    const std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>* GetGeometries() { return m_GeometriesRef; }
    const std::unordered_map<std::string, std::unique_ptr<Material>>* GetMaterials() { return m_MaterialsRef; }
    const std::unordered_map<std::string, std::unique_ptr<Texture>>* GetTextures() { return m_TexturesRef; }
    const std::unordered_map<std::string, std::unique_ptr<aiModelData::aiSkeleton>>* GetModelSkeltons() { return m_ModelSkeltonsRef; }
    
    const std::vector<Object*>& GetObjRenderLayer(RenderLayer layer) { return m_ObjRenderLayer[(int)layer]; }

    const std::vector<std::unique_ptr<Object>>& GetAllObjects() { return m_AllObjects; }
    const std::vector<Object*> GetCharacterObjects() { return m_CharacterObjects; }
    const std::vector<Object*> GetWorldObjects() { return m_WorldObjects; }
    const std::vector<Object*> GetUIObjects() { return m_UIObjects; }


protected:
    UINT m_width, m_height;
    RECT m_ClientRect;

    PassConstants m_MainPassCB; // index 0 of pass cbuffer.
    PassConstants m_ShadowPassCB; // index 1 of pass cbuffer.

    // Original Resource
    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>* m_GeometriesRef = nullptr;
    std::unordered_map<std::string, std::unique_ptr<Material>>* m_MaterialsRef = nullptr;
    std::unordered_map<std::string, std::unique_ptr<Texture>>* m_TexturesRef = nullptr;
    std::unordered_map<std::string, std::unique_ptr<aiModelData::aiSkeleton>>* m_ModelSkeltonsRef = nullptr;

    // List of all render item.
    std::unordered_map<std::string, std::unique_ptr<RenderItem>>* m_AllRitemsRef = nullptr;

    // Object divided by PSO.
    std::vector<Object*> m_ObjRenderLayer[(int)RenderLayer::Count];

    // List of objects by type.
    std::vector<std::unique_ptr<Object>> m_AllObjects;
    std::vector<Object*> m_CharacterObjects; // SkinnedConstants + ObjConstants + MatConstants가 있는 오브젝트
    std::vector<Object*> m_WorldObjects; // ObjConstants + MatConstants가 있는 오브젝트
    std::vector<Object*> m_UIObjects; // ObjConstants + MatConstants만 있는 오브젝트

public:
    UINT m_nSKinnedCB = 0; // = m_CharacterObjects.size();
    UINT m_nObjCB = 0; // = m_WorldObjects.size() + m_UIObjects.size();
    UINT m_nTextBatch = 0; // = sum(ui_obj->m_UIinfos.size() in m_UIObjects)

protected:
    Camera m_MainCamera;

    /////////////////////////////////////// Shadow Pass Items ////////////////////////////////////
    float m_LightNearZ = 0.0f;
    float m_LightFarZ = 0.0f;
    XMFLOAT3   m_LightPosW;
    XMFLOAT4X4 m_LightView = MathHelper::Identity4x4();
    XMFLOAT4X4 m_LightProj = MathHelper::Identity4x4();
    XMFLOAT4X4 m_ShadowTransform = MathHelper::Identity4x4();
    // Shadow.hlsl에선 gShadowMap을 사용하진 않지만
    // Common.hlsl에서 gShadowMap을 기대하기 때문에
    // gShadowMap에 대한 등록을 nullSrv로 대체한다.
    // (정확히는 아무것도 가르키지 않는 Srv)
    //CD3DX12_GPU_DESCRIPTOR_HANDLE m_NullSrv;
    //
    DirectX::BoundingSphere m_SceneBounds;
    //////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////// Light Objects /////////////////
    float m_LightRotationAngle = 0.0f;
    XMFLOAT3 m_BaseLightDirections[3] = {
        XMFLOAT3(0.57735f, -0.57735f, 0.57735f),
        XMFLOAT3(-0.57735f, -0.57735f, 0.57735f),
        XMFLOAT3(0.0f, -0.707f, -0.707f)
    };
    XMFLOAT3 m_RotatedLightDirections[3] = {
        m_BaseLightDirections[0],
        m_BaseLightDirections[1],
        m_BaseLightDirections[2]
    };
    /////////////////////////////////////////////////
};



/////////// Scene's child classes ///////////
class LoginScene;
class LobyScene;
class PlayGameScene;
class GameOverScene;
/////////////////////////////////////////////
#include "LoginScene.h"
#include "LobyScene.h"
#include "PlayGameScene.h"
#include "GameOverScene.h"

#define SIZE_ShadowMap 2048