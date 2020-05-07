#pragma once
#include "Common/Util/d3d12/d3dUtil.h"
#include "Common/Util/d3d12/MathHelper.h"
#include "Common/Util/d3d12/UploadBuffer.h"
#include "Common/FileLoader/ModelLoader.h"

struct ObjectConstants
{
    DirectX::XMFLOAT4X4 Local = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
    float TexAlpha = 1.0f;
    DirectX::XMFLOAT3 cbPerObjectPad3 = { 0.0f, 0.0f, 0.0f };
};

#define MAX_BONE 100
struct SkinnedConstants
{
    DirectX::XMFLOAT4X4 BoneTransform[MAX_BONE];
    SkinnedConstants()
    {
        for (int i = 0; i < MAX_BONE; ++i)
        {
            BoneTransform[i] = MathHelper::Identity4x4();
        }
    }
};

struct PassConstants
{
    DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProjTex = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 ShadowTransform = MathHelper::Identity4x4();
    DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    float cbPerPassPad1 = 0.0f;
    DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
    float NearZ = 0.0f;
    float FarZ = 0.0f;
    float TotalTime = 0.0f;
    float DeltaTime = 0.0f;

    DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light Lights[MaxLights];
};

struct DXTexturedVertex
{
    DirectX::XMFLOAT3 xmf3Position;
    DirectX::XMFLOAT3 xmf3Normal;
    DirectX::XMFLOAT3 xmf3Tangent;
    DirectX::XMFLOAT2 xmf2TextureUV;
};

struct DXSkinnedVertex
{
    DirectX::XMFLOAT3 xmf3Position;
    DirectX::XMFLOAT3 xmf3Normal;
    DirectX::XMFLOAT3 xmf3Tangent;
    DirectX::XMFLOAT2 xmf2TextureUV;
    float             fBoneWeights[4]; // last Weight not used, calculated inside the vertex shader
    int32_t           i32BoneIndices[4];
};

struct RenderItem
{
    std::string Name;

    Material* Mat = nullptr;
    MeshGeometry* Geo = nullptr;

    // Primitive topology.
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // DrawIndexedInstanced parameters.
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;
};

enum class RenderLayer : int
{
    Opaque = 0,
    SkinnedOpaque,
    Transparent,
    SkinnedTransparent,
    UILayout_Background, // support Transparency
    Count
};

// Stores the resources needed for the CPU to build the command lists
// for a frame.  
struct FrameResource
{
public:

    FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT skinnedCount, UINT materialCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
    ~FrameResource() = default;

    // We cannot reset the allocator until the GPU is done processing the commands.
    // So each frame needs their own allocator.
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

    // We cannot update a cbuffer until the GPU is done processing the commands
    // that reference it.  So each frame needs their own cbuffers.
    std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
    std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
    std::unique_ptr<UploadBuffer<SkinnedConstants>> SkinnedCB = nullptr;
    std::unique_ptr<UploadBuffer<MaterialConstants>> MaterialCB = nullptr;

    // Fence value to mark commands up to this fence point.  This lets us
    // check if these frame resources are still in use by the GPU.
    UINT64 FenceValue = 0;
    HANDLE FenceEvent;
};