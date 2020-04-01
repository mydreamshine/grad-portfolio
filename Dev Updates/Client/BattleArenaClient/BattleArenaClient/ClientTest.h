#pragma once

#include "DXSample.h"
#include "FrameResource.h"
#include "Common/Util/d3d12/GeometryGenerator.h"
#include "Common/Util/d3d12/Camera.h"
#include "Common/Util/d3d12/ShadowMap.h"
#include "Common/FileLoader/ModelLoader.h"
#include "Common/Timer/Timer.h"

using namespace DirectX;

using Microsoft::WRL::ComPtr;

#include "Common/FileLoader/TextureLoader.h"

struct RenderItem
{
    XMFLOAT4X4 World = MathHelper::Identity4x4();

    XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

    // CPU에서 GPU로의 명령 전달에 대한 비용을 최소화하기 위해
    // 렌더item의 렌더 업데이트가 필요한 경우에만 업데이트를 하게 만든다.
    // 이를 처리하기 위한 로직으로는
    // FrameResource별로 해당 렌더 Item을 렌더 업데이트를 하되
    // NumFramesDirty의 값이 0 초과일 경우에만 하게 한다.
    // 그리고 해당 렌더 Item의 렌더 업데이트 예약을 완료한 후에
    // NumFramesDirty을 1 감소시킨다.
    // 통상적으로 FrameResource마다 동일한 렌더 Item에 대한 업데이트가
    // 보장되어야 하므로 렌더 업데이트가 필요한 렌더 Item의
    // NumFramesDirty를 gNumFrameResources으로 한다.
    int NumFramesDirty = gNumFrameResources;

    // Index into GPU constant buffer corresponding to the ObjectCB for this render item.
    UINT ObjCBIndex = -1;

    Material* Mat = nullptr;
    MeshGeometry* Geo = nullptr;

    // Primitive topology.
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // DrawIndexedInstanced parameters.
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;

    // Index into GPU constant buffer corresponding to the SkinndCB for this render item.
    UINT SkinCBIndex = -1;

    // nullptr if this render-item is not animated by skinned mesh.
    aiModelData::aiSkeleton* Skeleton = nullptr;
};

enum class RenderLayer : int
{
    Opaque = 0,
    SkinnedOpaque,
    Debug,
    Count
};

class ClientTest : public DXSample
{
public:
    ClientTest(UINT width, UINT height, std::wstring name);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();

private:
    void LoadPipeline();
    void LoadAssets();

    void BuildRootSignature();
    void BuildDescriptorHeaps();
    void BuildConstantBufferView();
    void BuildShadersAndInputLayout();
    void BuildPSOs();

    void BuildScene();
    void BuildFrameResources();
    void BuildShapeGeometry();
    void LoadSkinnedModels();
    void LoadSkinnedModelData(const std::string& mesh_filepath, const std::vector<std::string>& anim_filepaths);
    void LoadTextures();
    std::vector<UINT8> GenerateTextureData();
    void BuildMaterials();
    void BuildRenderItems();

    void AnimateLights(CTimer& gt);
    void AnimateSkeletons(CTimer& gt);
    void UpdateObjectCBs(CTimer& gt);
    void UpdateSkinnedCBs(CTimer& gt);
    void UpdateMaterialCBs(CTimer& gt);
    void UpdateMainPassCB(CTimer& gt);
    void UpdateShadowPassCB(CTimer& gt);
    void UpdateShadowTransform(CTimer& gt);
    
    void PopulateCommandList();
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawSceneToShadowMap();
    void DrawSceneToBackBuffer();

    void WaitForPreviousFrame();

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

private:
    static const UINT FrameCount = 2;

    DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    //DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D32_FLOAT;
    DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12Resource> m_depthStencil;
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12DescriptorHeap> m_srvHeap;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize = 0;
    UINT m_cbvsrvuavDescriptorSize = 0;
    UINT m_DsvDescriptorSize = 0;

private:
    // Frame resource
    std::vector<std::unique_ptr<FrameResource>> m_FrameResources;
    FrameResource* m_CurrFrameResource = nullptr;
    int m_CurrFrameResourceIndex = 0;
    PassConstants m_MainPassCB; // index 0 of pass cbuffer.
    PassConstants m_ShadowPassCB; // index 1 of pass cbuffer.

    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_Geometries;
    std::unordered_map<std::string, std::unique_ptr<Material>> m_Materials;
    std::unordered_map<std::string, std::unique_ptr<Texture>> m_Textures;
    std::unordered_map<std::string, ComPtr<ID3DBlob>> m_Shaders;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_PSOs;

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_SkinnedInputLayout;

    // List of all the render items.
    std::vector<std::unique_ptr<RenderItem>> m_AllRitems;

    // Render items divided by PSO.
    std::vector<RenderItem*> m_RitemLayer[(int)RenderLayer::Count];

    // Extend resources
    ModelLoader m_ModelLoader;
    std::unordered_map<std::string, std::unique_ptr<aiModelData::aiSkeleton>> m_ModelSkeltons;
    CTimer m_Timer;

    // View Objects
    Camera m_Camera;
    DirectX::BoundingSphere m_SceneBounds;

    // Shadow Objects
    // 광원에서 바라본 시점(Pos, View, Proj, NearZ, FarZ, etc, ...)에 대한 변환행렬을 기준으로
    // Shadow Map에 투영하여 렌더링한다.
    std::unique_ptr<ShadowMap> m_ShadowMap;
    float m_LightNearZ = 0.0f;
    float m_LightFarZ = 0.0f;
    XMFLOAT3   m_LightPosW;
    XMFLOAT4X4 m_LightView = MathHelper::Identity4x4();
    XMFLOAT4X4 m_LightProj = MathHelper::Identity4x4();
    XMFLOAT4X4 m_ShadowTransform = MathHelper::Identity4x4();
    CD3DX12_GPU_DESCRIPTOR_HANDLE m_NullSrv;

    // Light Objects
    float m_LightRotationAngle = 0.0f;
    XMFLOAT3 m_BaseLightDirections[3] = {
        XMFLOAT3(0.57735f, -0.57735f, 0.57735f),
        XMFLOAT3(-0.57735f, -0.57735f, 0.57735f),
        XMFLOAT3(0.0f, -0.707f, -0.707f)
    };
    XMFLOAT3 m_RotatedLightDirections[3];

    // Synchronization objects.
    UINT m_frameIndex;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;
};
