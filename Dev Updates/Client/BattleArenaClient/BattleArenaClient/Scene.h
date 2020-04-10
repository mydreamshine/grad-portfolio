#pragma once

#include "DXSample.h"
#include "FrameResource.h"
#include "Common/Util/d3d12/GeometryGenerator.h"
#include "Common/Util/d3d12/Camera.h"
#include "Common/Util/d3d12/ShadowMap.h"
#include "Common/FileLoader/ModelLoader.h"
#include "Common/Timer/Timer.h"
#include "Common/FileLoader/TextureLoader.h"

using namespace DirectX;

using Microsoft::WRL::ComPtr;

class Scene
{
public:
    Scene() = default;
    Scene(UINT width, UINT height);
    virtual ~Scene();

    virtual void OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
        DXGI_FORMAT BackBufferFormat,
        int& matCB_index, int& diffuseSrvHeap_Index,
        int& objCB_index, int& skinnedCB_index);
    virtual void OnInitProperties() = 0;
    virtual void OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map, CTimer& gt);
    virtual void DisposeUploaders();

public:
    virtual void BuildShapeGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) = 0;
    virtual void LoadSkinnedModels(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) = 0;
    void LoadSkinnedModelData( ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
        ModelLoader& model_loader,
        const std::string& mesh_filepath, const std::vector<std::string>& anim_filepaths);
    virtual void LoadTextures(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DXGI_FORMAT BackBufferFormat) = 0;
    //std::vector<UINT8> GenerateTextureData();
    virtual void BuildMaterials(int& matCB_index, int& diffuseSrvHeap_Index) = 0;
    virtual void BuildRenderItems(int& objCB_index, int& skinnedCB_index) = 0;


public:
    virtual void UpdateObjectCBs(UploadBuffer<ObjectConstants>* objCB, CTimer& gt);
    void aiM2dxM(XMFLOAT4X4& dest, aiMatrix4x4& source);
    virtual void UpdateSkinnedCBs(UploadBuffer<SkinnedConstants>* skinnedCB, CTimer& gt);
    virtual void UpdateMaterialCBs(UploadBuffer<MaterialConstants>* matCB, CTimer& gt);
    virtual void UpdateMainPassCB(UploadBuffer<PassConstants>* passCB, CTimer& gt);
    virtual void UpdateShadowPassCB(UploadBuffer<PassConstants>* passCB, ShadowMap* shadow_map, CTimer& gt);
    virtual void UpdateShadowTransform(CTimer& gt);
    virtual void AnimateLights(CTimer& gt) = 0;
    virtual void AnimateSkeletons(CTimer& gt) = 0;
    virtual void AnimateCameras(CTimer& gt) = 0;
    
public:
    virtual void ProcessInput(CTimer& gt) = 0;

public:
    const std::vector<std::unique_ptr<RenderItem>>& GetAllRitems() { return m_AllRitems; }
    const std::vector<RenderItem*>& GetRitemLayer(RenderLayer layer) { return m_RitemLayer[(int)layer]; }

    const PassConstants& GetMainPassCB() { return m_MainPassCB; }
    const PassConstants& GetShadowPassCB() { return m_ShadowPassCB; }

    const std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& GetGeometries() { return m_Geometries; }
    const std::unordered_map<std::string, std::unique_ptr<Material>>& GetMaterials() { return m_Materials; }
    const std::unordered_map<std::string, std::unique_ptr<Texture>>& GetTextures() { return m_Textures; }
    const std::unordered_map<std::string, std::unique_ptr<aiModelData::aiSkeleton>>& GetModelSkeltons() { return m_ModelSkeltons; }

protected:
    UINT m_width, m_height;

    PassConstants m_MainPassCB; // index 0 of pass cbuffer.
    PassConstants m_ShadowPassCB; // index 1 of pass cbuffer.

    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_Geometries;
    std::unordered_map<std::string, std::unique_ptr<Material>> m_Materials;
    std::unordered_map<std::string, std::unique_ptr<Texture>> m_Textures;
    std::unordered_map<std::string, std::unique_ptr<aiModelData::aiSkeleton>> m_ModelSkeltons;

    // List of all the render items.
    std::vector<std::unique_ptr<RenderItem>> m_AllRitems;

    // Render items divided by PSO.
    std::vector<RenderItem*> m_RitemLayer[(int)RenderLayer::Count];

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

public:
    ComPtr<ID3D12Resource> m_BackBuffer = nullptr;
};



/////////// Scene's child classes ///////////
class LoginScene;
class LobyScene;
class PlayGameScene;
class GameOverScene;
/////////////////////////////////////////////
#include "LoginScene.h"
#include "PlayGameScene.h"
