#pragma once

#include"Scene.h"

class ClientTest : public DXSample
{
public:
    ClientTest(UINT width, UINT height, std::wstring name);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender(HWND hwnd);
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
    
    void PopulateCommandList();
    void DrawObjRenderLayer(ID3D12GraphicsCommandList* cmdList, const std::vector<Object*>& ObjLayer);
    void DrawSceneToShadowMap();
    void DrawSceneToBackBuffer();
    void DrawSceneToUI();
    void DrawSceneToTexts(HWND hwnd);

    void WaitForPreviousFrame();

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

private:
    static const UINT FrameCount = 2;

    DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT; // DXGI_FORMAT_D32_FLOAT;

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12Resource> m_depthStencil;
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12DescriptorHeap> m_srvHeap;
    UINT m_rtvDescriptorSize = 0;
    UINT m_cbvsrvuavDescriptorSize = 0;
    UINT m_DsvDescriptorSize = 0;


private:
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_SkinnedInputLayout;
    std::unordered_map<std::string, ComPtr<ID3DBlob>> m_Shaders;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_PSOs;

    // 광원에서 바라본 시점(Pos, View, Proj, NearZ, FarZ, etc, ...)에 대한 변환행렬을 기준으로
    // Shadow Map에 투영하여 렌더링한다.
    std::unique_ptr<ShadowMap> m_ShadowMap;
    CD3DX12_GPU_DESCRIPTOR_HANDLE m_NullSrv;

private:
    // Frame resource
    std::vector<std::unique_ptr<FrameResource>> m_FrameResources;
    FrameResource* m_CurrFrameResource = nullptr;
    int m_CurrFrameResourceIndex = 0;

    // Material의 diffuseSrvHeapIndex와 각 Scene에서의 Texture 생성 순서를 매칭하기 위해 m_Texture만 Vector로 정의
    std::vector<Texture*> m_Textures;

    UINT m_nSKinnedCB = 0;
    UINT m_nObjCB = 0;
    UINT m_nMatCB = 0;

private:
    std::unordered_map<std::string, std::unique_ptr<Scene>> m_Scenes;
    Scene* m_CurrScene = nullptr;
    std::string m_CurrSceneName;

private:
    // Extend resources
    CTimer m_Timer;

    // Synchronization objects.
    UINT m_frameIndex;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;
};
