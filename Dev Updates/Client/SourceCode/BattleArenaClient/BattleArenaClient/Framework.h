#pragma once

#include"Scene.h"
#include "ResourceManager.h"
#include "FrameworkEvent.h"

class Framework : public DXSample
{
public:
    Framework() = default;
    Framework(UINT width, UINT height, std::wstring name, std::string* additionalAssetPath = nullptr);
    virtual ~Framework();

    virtual void OnInit(HWND hwnd, UINT width, UINT height, std::wstring name, ResourceManager* ExternalResource, std::string* additionalAssetPath = nullptr);
    void OnInitAllSceneProperties();
    virtual void OnUpdate(std::queue<std::unique_ptr<EVENT>>& GeneratedEvents, RECT* pClientRect = nullptr);
    virtual void OnRender();
    virtual void OnDestroy();

    virtual void OnKeyDown(UINT8 key, POINT* OldCursorPos = nullptr);
    virtual void OnKeyUp(UINT8 key, POINT* OldCursorPos = nullptr);

    BYTE* GetFrameData();

public:
    std::queue<std::unique_ptr<EVENT>> m_SavedEvents;
    virtual void ProcessEvents(std::queue<std::unique_ptr<EVENT>>& Events);
    virtual void ProcessEvent(std::unique_ptr<EVENT>& Event);

private:
    void LoadPipeline();
    void LoadAssets(ResourceManager* ExternalResource);

    void BuildRootSignature();
    void BuildDescriptorHeaps();
    void BuildConstantBufferView();
    void BuildShadersAndInputLayout();
    void BuildPSOs();

    void BuildScene(ResourceManager* ExternalResource);
    // BuildFont는 ResourceUploadBatch때문에 CommandQueue가 초기화 되므로,
    // CommandQueue가 비어있고 Fence가 업데이트된 상태에서 따로 호출해줘야 한다.
    void BuildFontSpriteBatchs();
    void BuildFrameResources();

    void PopulateCommandList();
    void DrawObjRenderLayer(ID3D12GraphicsCommandList* cmdList, const std::vector<Object*>& ObjLayer);
    // NonShadowRender로 지정되어 있는 오브젝트를 제외한 렌더 레이어 재구성
    void ExcludeNonShadowRenderObjects(std::vector<Object*>& newRednerLayer, const std::vector<Object*> sourceRenderLayer);
    void DrawSceneToShadowMap();
    void DrawSceneToBackBuffer();
    void DrawSceneToUI();
    void DrawSceneToBackground();

    void WaitForPreviousFrame();

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

private:
    std::string m_additionalAssetPath;

private:
    ComPtr<ID3D12Resource> m_ScreenshotBuffer = nullptr;
    std::unique_ptr<BYTE[]> m_FrameData = nullptr;
    UINT m_FrameDataLength = 0;

private:
    static const UINT FrameCount = 2;

    DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_BACKBUFFER;
    DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_DEPTHSTENCIL; // DXGI_FORMAT_D32_FLOAT;

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

    // 렌더링할 텍스트별 FontSprite VertexSegment에 대한 페이지를 중복 생성하는 것을 방지하기 위해
    // FontSpriteBatch를 렌더링할 텍스트마다 지니게 한다.
    // 단, 이는 commandQueue가 완전히 비어있는 상태(혹은 독립적인 commandQueue)에서 생성해야 하기에,
    // Object의 TextInfo에서는 FontSpriteBatch를 취급하진 않는다.
    std::unordered_map<std::wstring, std::vector<std::unique_ptr<DirectX::SpriteBatch>>> m_FontSpriteBatchs;
    std::unordered_map<std::wstring, std::unique_ptr<DXTK_FONT>>* m_FontsRef = nullptr;

    UINT m_nSKinnedCB = 0;
    UINT m_nObjCB = 0;
    UINT m_nMatCB = 0;
    UINT m_nTextBatch = 0;

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

private:
    bool m_KeyState[256]; // false: KeyUp, true: KeyDown
    POINT m_OldCursorPos;
};
