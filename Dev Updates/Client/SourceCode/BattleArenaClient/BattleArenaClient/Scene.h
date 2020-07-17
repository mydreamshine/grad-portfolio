#pragma once

#include "DXSample.h"
#include "FrameResource.h"
#include "FrameworkEvent.h"
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
        CTimer& gt,
        std::queue<std::unique_ptr<EVENT>>& GeneratedEvents);

public:
    virtual void SetGeometriesRef(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>* Geometries) { m_GeometriesRef = Geometries; }
    virtual void SetMaterialsRef(std::unordered_map<std::string, std::unique_ptr<Material>>* Materials) { m_MaterialsRef = Materials; }
    virtual void SetTexturesRef(std::unordered_map<std::string, std::unique_ptr<Texture>>* Textures) { m_TexturesRef = Textures; }
    virtual void SetModelSkeletonsRef(std::unordered_map<std::string, std::unique_ptr<aiModelData::aiSkeleton>>* Skeletons) { m_ModelSkeltonsRef = Skeletons; }
    virtual void SetCharacterModelBoundingBoxesRef(std::unordered_map<std::string, BoundingBox>* CharacterModelBoundingBoxes) { m_CharacterModelBoundingBoxesRef = CharacterModelBoundingBoxes; }
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
    virtual void UpdateTextInfo(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents) = 0;
    virtual void AnimateLights(CTimer& gt) = 0;
    virtual void AnimateSkeletons(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents) = 0;
    virtual void AnimateCameras(CTimer& gt) = 0;
    
public:
    virtual void ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents) = 0;

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
    const std::vector<Object*> GetUILayOutObjects() { return m_UILayOutObjects; }
    const std::vector<Object*> GetTextObjects() { return m_TextObjects; }

public:
    ///////////////////////////////////////////////////////////////////////////////// Processing Events /////////////////////////////////////////////////////////////////////////////////
    // Control Element ID, Player NickName, Character Type, Propensity, Transform(Scale, RotationEuler, Position)
    void SpawnPlayer(int New_CE_ID, std::wstring Name, CHARACTER_TYPE CharacterType, bool IsMainPlayer, OBJECT_PROPENSITY Propensity, XMFLOAT3 Scale, XMFLOAT3 RotationEuler, XMFLOAT3 Position) {}
    // Control Element ID, Attack Order(Chracter Type), Propensity, Transform(Scale, RotationEuler, Position)
    void SpawnNormalAttackObject(int New_CE_ID, CHARACTER_TYPE AttackOrder, OBJECT_PROPENSITY Propensity, XMFLOAT3 Scale, XMFLOAT3 RotationEuler, XMFLOAT3 Position) {}
    // Control Element ID, Skill Type, Propensity, Transform(Scale, RotationEuler, Position)
    void SpawnSkillObject(int New_CE_ID, SKILL_TYPE SkillType, OBJECT_PROPENSITY Propensity, XMFLOAT3 Scale, XMFLOAT3 RotationEuler, XMFLOAT3 Position) {}
    // Effect Type, Transform(Position)
    void SpawnEffectObjects(EFFECT_TYPE EffectType, XMFLOAT3 Position) {}
    // Control Element ID, Transform(Scale, RotationEuler, Position)
    void SetObjectTransform(int CE_ID, XMFLOAT3 Scale, XMFLOAT3 RotationEuler, XMFLOAT3 Position) {}
    // Control Element ID, MotionType, SkillType(스킬 모션일 경우에만 지정, 그 외의 경우에는 NON)
    void SetCharacterMotion(int CE_ID, MOTION_TYPE MotionType, SKILL_TYPE SkillType = SKILL_TYPE::NON) {}
    // Control Element ID, Player State
    void SetPlayerState(int CE_ID, PLAYER_STATE PlayerState) {}
    // Deactivated Poison Gas Area
    void UpdateDeActPoisonGasArea(RECT DeActPoisonGasArea) {}
    // Control Element ID
    void DeActivateObject(int CE_ID) {}

    // UserName, UserRank
    void SetUserInfo(std::wstring UserName, int UserRank) {}
    // Count Score(Kill, Death, Assistance)
    void SetKDAScore(unsigned char Count_Kill, unsigned char Count_Death, unsigned char Count_Assistance) {}
    // Message ([Do_UserName] Killed [Target_UserName])
    void SetKillLog(std::wstring Message) {}
    // Message ([Do_UserName]: Chat Message)
    void SetChatLog(std::wstring Message) {}
    // Remaining Sec
    void SetGamePlayTimeLimit(unsigned int Sec) {}
    // Remaining HP
    void SetPlayerHP(int CE_ID, int HP) {}
    // UserName, UserRank, Count Score(Kill, Death, Assistance, Total Damage, Total Heal), Played Character Type
    void SetMatchStatisticInfo(std::wstring UserName, int UserRank,
        unsigned char Count_Kill, unsigned char Count_Death, unsigned char Count_Assistance,
        int TotalScore_Damage, int TotalScore_Heal,
        CHARACTER_TYPE PlayedCharacterType) {}

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



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

    std::unordered_map<std::string, BoundingBox>* m_CharacterModelBoundingBoxesRef = nullptr;

    // List of all render item.
    std::unordered_map<std::string, std::unique_ptr<RenderItem>>* m_AllRitemsRef = nullptr;

    // Object divided by PSO.
    std::vector<Object*> m_ObjRenderLayer[(int)RenderLayer::Count];

    // List of objects by type.
    std::vector<std::unique_ptr<Object>> m_AllObjects;
    std::vector<Object*> m_CharacterObjects; // SkinnedConstants + ObjConstants + MatConstants가 있는 오브젝트
    std::vector<Object*> m_WorldObjects; // ObjConstants + MatConstants가 있는 오브젝트
    std::vector<Object*> m_UILayOutObjects; // ObjConstants + MatConstants만 있는 오브젝트
    std::vector<Object*> m_TextObjects; // TextInfo만 있는 오브젝트
    std::vector<Object*> m_EffectObjects; // ObjConstants + MatConstants (+ Billbord, Non-Shadow Rendering)

public:
    UINT m_nSKinnedCB = 0; // = m_CharacterObjects.size();
    UINT m_nObjCB = 0; // = m_WorldObjects.size() + m_UIObjects.size();
    UINT m_nTextBatch = 0; // = m_TextObjects.size();

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