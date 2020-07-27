#include "stdafx.h"
#include "Framework.h"

const int gNumFrameResources = 3;

Framework::Framework(UINT width, UINT height, std::wstring name, std::string* additionalAssetPath) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height))
{
    if (additionalAssetPath != nullptr) m_additionalAssetPath = *additionalAssetPath;
}

Framework::~Framework()
{
}

void Framework::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    for (int i = 0; i < gNumFrameResources; ++i)
        CloseHandle(m_FrameResources[i]->FenceEvent);
}

void Framework::OnKeyDown(UINT8 key, POINT* OldCursorPos)
{
    m_KeyState[key] = true;
    if (OldCursorPos != nullptr) m_OldCursorPos = *OldCursorPos;
}

void Framework::OnKeyUp(UINT8 key, POINT* OldCursorPos)
{
    m_KeyState[key] = false;
}

BYTE* Framework::GetFrameData()
{
    D3D12_RANGE d3dReadRange = { 0, 0 };
    UINT8* pBufferDataBegin = NULL;
    m_ScreenshotBuffer->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);
    memcpy(m_FrameData.get(), pBufferDataBegin, m_FrameDataLength);
    m_ScreenshotBuffer->Unmap(0, NULL);
    return m_FrameData.get();
}

void Framework::ProcessEvents(std::queue<std::unique_ptr<EVENT>>& Events)
{
    std::unique_ptr<EVENT> Event = nullptr;
    while (Events.size() != 0)
    {
        Event = std::move(Events.front());
        Events.pop();
        Framework::ProcessEvent(*Event);
        Event = nullptr;
    }
}

void Framework::ProcessEvent(EVENT& Event)
{
    Scene* Event_Act_Place = nullptr;
    if      (Event.Act_Place == FEP_LOGIN_SCENE)    Event_Act_Place = m_Scenes["LoginScene"].get();
    else if (Event.Act_Place == FEP_LOBY_SCENE)     Event_Act_Place = m_Scenes["LobyScene"].get();
    else if (Event.Act_Place == FEP_PLAYGMAE_SCENE) Event_Act_Place = m_Scenes["PlayGameScene"].get();
    else if (Event.Act_Place == FEP_GAMEOVER_SCENE) Event_Act_Place = m_Scenes["GameOverScene"].get();
    else
    {
        MessageBox(NULL, L"Unknown Event Actvation Place.", L"Event Error", MB_OK);
        while (true);
    }

    switch (Event.Command)
    {
    case FEC_CHANGE_SCENE:
    {
        if (Event.Act_Place == FEP_LOGIN_SCENE)         m_CurrSceneName = "LoginScene";
        else if (Event.Act_Place == FEP_LOBY_SCENE)     m_CurrSceneName = "LobyScene";
        else if (Event.Act_Place == FEP_PLAYGMAE_SCENE) m_CurrSceneName = "PlayGameScene";
        else if (Event.Act_Place == FEP_GAMEOVER_SCENE) m_CurrSceneName = "GameOverScene";
        m_CurrScene = m_Scenes[m_CurrSceneName].get();
        m_CurrScene->OnInitProperties(m_Timer);
    }
    break;
    case FEC_SPAWN_PLAYER:
    {
        EVENT_DATA_PLAYER_SPAWN_INFO* EventData = reinterpret_cast<EVENT_DATA_PLAYER_SPAWN_INFO*>(Event.Data.get());
        Event_Act_Place->SpawnPlayer(Event.Act_Object,
            EventData->Name, EventData->CharacterType, EventData->IsMainCharacter, EventData->Propensity,
            EventData->Scale, EventData->RotationEuler, EventData->Position);
    }
    break;
    case FEC_SPAWN_NORMAL_ATTACK_OBJ:
    {
        EVENT_DATA_NORMAL_ATTACK_OBJ_SPAWN_INFO* EventData = reinterpret_cast<EVENT_DATA_NORMAL_ATTACK_OBJ_SPAWN_INFO*>(Event.Data.get());
        Event_Act_Place->SpawnNormalAttackObject(Event.Act_Object,
            EventData->AttackOrder, EventData->Propensity,
            EventData->Scale, EventData->RotationEuler, EventData->Position);
    }
    break;
    case FEC_SPAWN_SKILL_OBJ:
    {
        EVENT_DATA_SKILL_OBJ_SPAWN_INFO* EventData = reinterpret_cast<EVENT_DATA_SKILL_OBJ_SPAWN_INFO*>(Event.Data.get());
        Event_Act_Place->SpawnSkillObject(Event.Act_Object,
            EventData->SkillType, EventData->Propensity,
            EventData->Scale, EventData->RotationEuler, EventData->Position);
    }
    break;
    case FEC_SPAWN_PICKING_EFFECT_OBJ:
    case FEC_SPAWN_EFFECT_OBJ:
    {
        EVENT_DATA_EFFECT_OBJ_SPAWN_INFO* EventData = reinterpret_cast<EVENT_DATA_EFFECT_OBJ_SPAWN_INFO*>(Event.Data.get());
        Event_Act_Place->SpawnEffectObjects(EventData->EffectType, EventData->Position);
    }
    break;
    case FEC_SET_TRANSFORM_WORLD_OBJECT:
    {
        EVENT_DATA_OBJ_TRANSFORM* EventData = reinterpret_cast<EVENT_DATA_OBJ_TRANSFORM*>(Event.Data.get());
        Event_Act_Place->SetObjectTransform(Event.Act_Object,
            EventData->Scale, EventData->RotationEuler, EventData->Position);
    }
    break;
    case FEC_SET_CHARACTER_MOTION:
    {
        EVENT_DATA_CHARACTER_MOTION_INFO* EventData = reinterpret_cast<EVENT_DATA_CHARACTER_MOTION_INFO*>(Event.Data.get());
        Event_Act_Place->SetCharacterMotion(Event.Act_Object,
            EventData->MotionType, EventData->MotionSpeed, EventData->SkillMotionType);
    }
    break;
    case FEC_SET_PLAYER_STATE:
    {
        EVENT_DATA_PLAYER_STATE_INFO* EventData = reinterpret_cast<EVENT_DATA_PLAYER_STATE_INFO*>(Event.Data.get());
        Event_Act_Place->SetPlayerState(Event.Act_Object, EventData->PlayerState);
    }
    break;
    case FEC_UPDATE_POISON_FOG_DEACT_AREA:
    {
        EVENT_DATA_POISON_FOG_DEACT_AREA* EventData = reinterpret_cast<EVENT_DATA_POISON_FOG_DEACT_AREA*>(Event.Data.get());
        Event_Act_Place->UpdateDeActPoisonGasArea(EventData->Area);
    }
    break;
    case FEC_DEACTIVATE_OBJ:
    {
        Event_Act_Place->DeActivateObject(Event.Act_Object);
    }
    break;
    case FEC_SET_USER_INFO:
    {
        EVENT_DATA_USER_INFO* EventData = reinterpret_cast<EVENT_DATA_USER_INFO*>(Event.Data.get());
        Event_Act_Place->SetUserInfo(EventData->UserName, EventData->UserRank);
    }
    break;
    case FEC_SET_KDA_SCORE:
    {
        EVENT_DATA_KDA_SCORE* EventData = reinterpret_cast<EVENT_DATA_KDA_SCORE*>(Event.Data.get());
        Event_Act_Place->SetKDAScore(EventData->Count_Kill, EventData->Count_Death, EventData->Count_Assistance);
    }
    break;
    case FEC_SET_KILL_LOG:
    {
        EVENT_DATA_KILL_LOG* EventData = reinterpret_cast<EVENT_DATA_KILL_LOG*>(Event.Data.get());
        Event_Act_Place->SetKillLog(EventData->kill_player_id, EventData->death_player_id);
    }
    break;
    case FEC_SET_CHAT_LOG:
    {
        EVENT_DATA_CHAT_LOG* EventData = reinterpret_cast<EVENT_DATA_CHAT_LOG*>(Event.Data.get());
        Event_Act_Place->SetChatLog(EventData->Message);
    }
    break;
    case FEC_SET_GAME_PLAY_TIME_LIMIT:
    {
        EVENT_DATA_GAME_PLAY_TIME_LIMIT* EventData = reinterpret_cast<EVENT_DATA_GAME_PLAY_TIME_LIMIT*>(Event.Data.get());
        Event_Act_Place->SetGamePlayTimeLimit(EventData->Sec);
    }
    break;
    case FEC_SET_PLAYER_HP:
    {
        EVENT_DATA_PLAYER_HP* EventData = reinterpret_cast<EVENT_DATA_PLAYER_HP*>(Event.Data.get());
        Event_Act_Place->SetPlayerHP(Event.Act_Object, EventData->HP);
    }
    break;
    case FEC_SET_MATCH_STATISTIC_INFO:
    {
        EVENT_DATA_MATCH_STATISTIC_INFO* EventData = reinterpret_cast<EVENT_DATA_MATCH_STATISTIC_INFO*>(Event.Data.get());
        Event_Act_Place->SetMatchStatisticInfo(EventData->UserName, EventData->UserRank,
            EventData->Count_Kill, EventData->Count_Death, EventData->Count_Assistance,
            EventData->TotalScore_Damage, EventData->TotalScore_Heal,
            EventData->PlayedCharacterType);
    }
    break;


    case FEC_MATCH_ENQUEUE:
    {
        LobyScene* loby = reinterpret_cast<LobyScene*>(Event_Act_Place);
        loby->SetMatchStatus(true);
    }
    break;
    case FEC_MATCH_DEQUEUE:
    {
        LobyScene* loby = reinterpret_cast<LobyScene*>(Event_Act_Place);
        loby->SetMatchStatus(false);
    }
    break;
    case FEC_ACCESS_MATCH:
    {
        EVENT_DATA_ACCESS_MATCH* EventData = reinterpret_cast<EVENT_DATA_ACCESS_MATCH*>(Event.Data.get());
        LobyScene* loby = reinterpret_cast<LobyScene*>(Event_Act_Place);
        loby->SetAccessMatch(true);
    }
    break;

    default:
        MessageBox(NULL, L"Unknown Event Command.", L"Event Error", MB_OK);
        while (true);
        break;
    }
}

void Framework::OnInit(HWND hwnd, UINT width, UINT height, std::wstring name, ResourceManager* ExternalResource, std::string* additionalAssetPath)
{
    DXSample::OnInit(hwnd, width, height, name);
    if (additionalAssetPath != nullptr) m_additionalAssetPath = *additionalAssetPath;
    m_frameIndex = 0;
    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));
    LoadPipeline();
    LoadAssets(ExternalResource);

    m_Timer.Start();
    m_Timer.Reset();
}

void Framework::OnInitAllSceneProperties()
{
    for (auto& scene_iter : m_Scenes)
    {
        auto scene = scene_iter.second.get();
        scene->OnInitProperties(m_Timer);
    }
}

// Load the rendering pipeline dependencies.
void Framework::LoadPipeline()
{
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    if (m_useWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);

        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    if (m_hasWindow == true)
    {
        // Describe and create the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = FrameCount;
        swapChainDesc.Width = m_width;
        swapChainDesc.Height = m_height;
        swapChainDesc.Format = m_BackBufferFormat;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> swapChain;
        ThrowIfFailed(factory->CreateSwapChainForHwnd(
            m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
            m_hwnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain
        ));

        // This app does not support fullscreen transitions.
        ThrowIfFailed(factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));

        ThrowIfFailed(swapChain.As(&m_swapChain));
        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    }

    // Create rtv & dsv descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        // Describe and create a depth stencil view (DSV) descriptor heap.
        // Each frame has its own depth stencils (to write shadows onto) 
        // and then there is one for the scene itself.
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 2; // Add +1 DSV for shadow map.
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_cbvsrvuavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        m_DsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    }

    // Create rtv buffer view.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            if (m_hasWindow == true)
            {
                ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            }
            else // Create Render Target Frame
            {
                CD3DX12_RESOURCE_DESC RenderTargetDesc(
                    D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                    65536,
                    static_cast<UINT>(WND_WIDTH),
                    static_cast<UINT>(WND_HEIGHT),
                    1,
                    1,
                    m_BackBufferFormat,
                    1,
                    0,
                    D3D12_TEXTURE_LAYOUT_UNKNOWN,
                    D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

                //d3dResourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;
                HRESULT hResult = m_device->CreateCommittedResource(
                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                    D3D12_HEAP_FLAG_NONE,
                    &RenderTargetDesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    NULL,
                    IID_PPV_ARGS(&m_renderTargets[n]));
            }
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    // Create the depth stencil buffer.
    {
        CD3DX12_RESOURCE_DESC depthStencilDesc(
            D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            0,
            static_cast<UINT>(m_viewport.Width),
            static_cast<UINT>(m_viewport.Height),
            1,
            1,
            m_DepthStencilFormat,
            1,
            0,
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

        D3D12_CLEAR_VALUE clearValue;    // Performance tip: Tell the runtime at resource creation the desired clear value.
        clearValue.Format = m_DepthStencilFormat;
        clearValue.DepthStencil.Depth = 1.0f;
        clearValue.DepthStencil.Stencil = 0;

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &depthStencilDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS(&m_depthStencil)));

        // Create the depth stencil view.
        m_device->CreateDepthStencilView(m_depthStencil.Get(), nullptr, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

        // 두번째 depth stencil view(for shadowMap)에 대한 생성은
        // ShadowMap 객체에서 이루어진다.
    }

    // Create the scree shot buffer
    {
        CoInitialize(NULL);
        m_FrameDataLength = m_width * m_height * 4;
        
        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(m_FrameDataLength),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_ScreenshotBuffer)));

        m_FrameData = std::make_unique<BYTE[]>(m_FrameDataLength);
    }

    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
}

// Load the assets.
void Framework::LoadAssets(ResourceManager* ExternalResource)
{
    BuildRootSignature();
    BuildShadersAndInputLayout();
    BuildPSOs();

    m_ShadowMap = std::make_unique<ShadowMap>(m_device.Get(), SIZE_ShadowMap, SIZE_ShadowMap);

    // 반드시 외부 리소스를 먼저 생성(임포트)한 다음에 Scene을 생성해야 한다.
    BuildScene(ExternalResource);

    // Texture에 대한 SRV 생성
    // (LoadExternalResource()를 통해 외부 리소스가 먼저 임포트 되어있어야 한다.)
    auto& AllTextures = ExternalResource->GetTextures();
    for (auto& Tex_iter : AllTextures)
        m_Textures.push_back(Tex_iter.second.get());
    m_nMatCB = ExternalResource->m_nMatCB;
    BuildDescriptorHeaps();
    BuildConstantBufferView();

    BuildFrameResources();


    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValue = 1;

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForPreviousFrame();
    }

    m_FontsRef = &(ExternalResource->GetFonts());
    BuildFontSpriteBatchs();
}

// Create the root signature.
void Framework::BuildRootSignature()
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); // t0: diffuseMap
    ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); // t1: shadowMap

    CD3DX12_ROOT_PARAMETER1 rootParameters[6];
    rootParameters[0].InitAsConstantBufferView(0); // ObjectConstants
    rootParameters[1].InitAsConstantBufferView(1); // SkinnedConstants
    rootParameters[2].InitAsConstantBufferView(2); // MaterialConstants
    rootParameters[3].InitAsConstantBufferView(3); // PassConstants
    rootParameters[4].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL); // t0: diffuseMap
    rootParameters[5].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL); // t1: shadowMap

    auto staticSamplers = GetStaticSamplers();

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(
        _countof(rootParameters), rootParameters,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
    ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
}

void Framework::BuildDescriptorHeaps()
{
    UINT TextureCount = (UINT)m_Textures.size();
    UINT ShadowMapCount = 1;

    UINT numDescriptors = TextureCount * gNumFrameResources + (ShadowMapCount + 1); // add +1 srv for nullSrv

    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc;
    srvHeapDesc.NumDescriptors = numDescriptors;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    srvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
}

// Srv buffer view case
// ┌───────────────────────┐
// │             Srv descriptor heaps             │
// ├───────────┬───────────┤
// │     texture_views    │                      │
// ├───┬───┬───┤ null_view(for shadow)│
// │frame0│frame1│frame2│                      │
// └───┴───┴───┴───────────┘
void Framework::BuildConstantBufferView()
{
    UINT TextureCount = (UINT)m_Textures.size();
    UINT ShadowMapCount = 1;

    // Describe and create a SRV for the texture.
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = m_BackBufferFormat;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    // Need a SRV descriptor for each object for each frame resource.
    for (int frameIndex = 0; frameIndex < gNumFrameResources; ++frameIndex)
    {
        UINT i = 0;
        for (auto& tex : m_Textures)
        {
            int heapIndex = TextureCount * frameIndex + i;
            auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvHeap->GetCPUDescriptorHandleForHeapStart());
            handle.Offset(heapIndex, m_cbvsrvuavDescriptorSize);

            m_device->CreateShaderResourceView(tex->Resource.Get(), &srvDesc, handle);

            i++;
        }
    }

    // Offset to the tex+shadowmap Srvs in the descriptor heap.
    int heapIndex = TextureCount * gNumFrameResources + 1;
    auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvHeap->GetCPUDescriptorHandleForHeapStart());
    handle.Offset(heapIndex, m_cbvsrvuavDescriptorSize);
    m_NullSrv = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvHeap->GetGPUDescriptorHandleForHeapStart());
    m_NullSrv.Offset(heapIndex, m_cbvsrvuavDescriptorSize);

    m_device->CreateShaderResourceView(nullptr, &srvDesc, handle);


    // Create srv & dsv for shadowMap.
    {
        UINT ShadowMapHeapIndex = TextureCount * gNumFrameResources;

        auto hCPUSrv = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvHeap->GetCPUDescriptorHandleForHeapStart());
        hCPUSrv.Offset(ShadowMapHeapIndex, m_cbvsrvuavDescriptorSize);

        auto hGPUSrv = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvHeap->GetGPUDescriptorHandleForHeapStart());
        hGPUSrv.Offset(ShadowMapHeapIndex, m_cbvsrvuavDescriptorSize);

        auto hCPUDsv = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
        hCPUDsv.Offset(1, m_DsvDescriptorSize);

        m_ShadowMap->BuildDescriptors(hCPUSrv, hGPUSrv, hCPUDsv);
    }
}

void Framework::BuildShadersAndInputLayout()
{
    const D3D_SHADER_MACRO skinnedDefines[] =
    {
        "SKINNED", "1",
        NULL, NULL
    };

    const D3D_SHADER_MACRO alphaTestDefines[] =
    {
        "ALPHA_TEST", "1",
        NULL, NULL
    };

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring additionalAssetPath = converter.from_bytes(m_additionalAssetPath.c_str());

    m_Shaders["standardVS"] = d3dUtil::CompileShader(additionalAssetPath + L"Shaders/Default.hlsl", nullptr, "VS", "vs_5_1");
    m_Shaders["skinnedVS"] = d3dUtil::CompileShader(additionalAssetPath + L"Shaders/Default.hlsl", skinnedDefines, "VS", "vs_5_1");
    m_Shaders["opaquePS"] = d3dUtil::CompileShader(additionalAssetPath + L"Shaders/Default.hlsl", nullptr, "PS", "ps_5_1");
    m_Shaders["TransparentPS"] = d3dUtil::CompileShader(additionalAssetPath + L"Shaders/Default.hlsl", alphaTestDefines, "PS", "ps_5_1");

    m_Shaders["shadowVS"] = d3dUtil::CompileShader(additionalAssetPath + L"Shaders/Shadows.hlsl", nullptr, "VS", "vs_5_1");
    m_Shaders["skinnedShadowVS"] = d3dUtil::CompileShader(additionalAssetPath + L"Shaders/Shadows.hlsl", skinnedDefines, "VS", "vs_5_1");
    m_Shaders["shadowOpaquePS"] = d3dUtil::CompileShader(additionalAssetPath + L"Shaders/Shadows.hlsl", nullptr, "PS", "ps_5_1");

    m_Shaders["UILayout_Background_VS"] = d3dUtil::CompileShader(additionalAssetPath + L"Shaders/UILayout_Background.hlsl", nullptr, "VS", "vs_5_1");
    m_Shaders["UILayout_Background_PS"] = d3dUtil::CompileShader(additionalAssetPath + L"Shaders/UILayout_Background.hlsl", alphaTestDefines, "PS", "ps_5_1");

    m_InputLayout =
    {
        { "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    m_SkinnedInputLayout =
    {
        { "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BONE_WEIHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BONE_IDS",    0, DXGI_FORMAT_R32G32B32A32_SINT,  0, 60, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void Framework::BuildPSOs()
{
    D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
    transparencyBlendDesc.BlendEnable = true;
    transparencyBlendDesc.LogicOpEnable = false;
    transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
    transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

    //
    // PSO for opaque objects.
    //
    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    opaquePsoDesc.InputLayout = { m_InputLayout.data(), (UINT)m_InputLayout.size() };
    opaquePsoDesc.pRootSignature = m_rootSignature.Get();
    opaquePsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(m_Shaders["standardVS"]->GetBufferPointer()),
        m_Shaders["standardVS"]->GetBufferSize()
    };
    opaquePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(m_Shaders["opaquePS"]->GetBufferPointer()),
        m_Shaders["opaquePS"]->GetBufferSize()
    };
    opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    opaquePsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
    opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    opaquePsoDesc.SampleMask = UINT_MAX;
    opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    opaquePsoDesc.NumRenderTargets = 1;
    opaquePsoDesc.RTVFormats[0] = m_BackBufferFormat;
    opaquePsoDesc.SampleDesc.Count = 1;
    //opaquePsoDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
    //opaquePsoDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
    opaquePsoDesc.DSVFormat = m_DepthStencilFormat;
    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&m_PSOs["opaque"])));

    //
    // PSO for skinned objects.
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedOpaquePsoDesc = opaquePsoDesc;
    skinnedOpaquePsoDesc.InputLayout = { m_SkinnedInputLayout.data(), (UINT)m_SkinnedInputLayout.size() };
    skinnedOpaquePsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(m_Shaders["skinnedVS"]->GetBufferPointer()),
        m_Shaders["skinnedVS"]->GetBufferSize()
    };
    skinnedOpaquePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(m_Shaders["opaquePS"]->GetBufferPointer()),
        m_Shaders["opaquePS"]->GetBufferSize()
    };
    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&skinnedOpaquePsoDesc, IID_PPV_ARGS(&m_PSOs["skinnedOpaque"])));

    //
    // PSO for transparent objects
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;
    transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
    transparentPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&m_PSOs["transparent"])));

    //
    // PSO for skinned transparent objects
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC SkinnedTransparentPsoDesc = opaquePsoDesc;
    SkinnedTransparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
    SkinnedTransparentPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    SkinnedTransparentPsoDesc.InputLayout = { m_SkinnedInputLayout.data(), (UINT)m_SkinnedInputLayout.size() };
    SkinnedTransparentPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(m_Shaders["skinnedVS"]->GetBufferPointer()),
        m_Shaders["skinnedVS"]->GetBufferSize()
    };
    SkinnedTransparentPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(m_Shaders["TransparentPS"]->GetBufferPointer()),
        m_Shaders["TransparentPS"]->GetBufferSize()
    };
    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&SkinnedTransparentPsoDesc, IID_PPV_ARGS(&m_PSOs["skinnedTransparent"])));

    //
    // PSO for shadow map pass.
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC smapPsoDesc = opaquePsoDesc;
    smapPsoDesc.RasterizerState.DepthBias = 100000;
    smapPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;
    smapPsoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
    smapPsoDesc.pRootSignature = m_rootSignature.Get();
    smapPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(m_Shaders["shadowVS"]->GetBufferPointer()),
        m_Shaders["shadowVS"]->GetBufferSize()
    };
    smapPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(m_Shaders["shadowOpaquePS"]->GetBufferPointer()),
        m_Shaders["shadowOpaquePS"]->GetBufferSize()
    };

    // Shadow map pass does not have a render target.
    smapPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    smapPsoDesc.NumRenderTargets = 0;
    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&smapPsoDesc, IID_PPV_ARGS(&m_PSOs["shadow_opaque"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedSmapPsoDesc = smapPsoDesc;
    skinnedSmapPsoDesc.InputLayout = { m_SkinnedInputLayout.data(), (UINT)m_SkinnedInputLayout.size() };
    skinnedSmapPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(m_Shaders["skinnedShadowVS"]->GetBufferPointer()),
        m_Shaders["skinnedShadowVS"]->GetBufferSize()
    };
    skinnedSmapPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(m_Shaders["shadowOpaquePS"]->GetBufferPointer()),
        m_Shaders["shadowOpaquePS"]->GetBufferSize()
    };
    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&skinnedSmapPsoDesc, IID_PPV_ARGS(&m_PSOs["skinnedShadow_opaque"])));

    //
    // PSO for UI pass.
    //

    D3D12_GRAPHICS_PIPELINE_STATE_DESC UIPsoDesc = opaquePsoDesc;
    UIPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
    UIPsoDesc.InputLayout = { m_InputLayout.data(), (UINT)m_InputLayout.size() };
    UIPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(m_Shaders["UILayout_Background_VS"]->GetBufferPointer()),
        m_Shaders["UILayout_Background_VS"]->GetBufferSize()
    };
    UIPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(m_Shaders["UILayout_Background_PS"]->GetBufferPointer()),
        m_Shaders["UILayout_Background_PS"]->GetBufferSize()
    };
    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&UIPsoDesc, IID_PPV_ARGS(&m_PSOs["UILayout_Background"])));
}

void Framework::BuildScene(ResourceManager* ExternalResource)
{
    m_Scenes["LoginScene"]    = std::make_unique<LoginScene>(m_width, m_height);
    m_Scenes["LobyScene"]     = std::make_unique<LobyScene>(m_width, m_height);
    m_Scenes["PlayGameScene"] = std::make_unique<PlayGameScene>(m_width, m_height);
    m_Scenes["GameOverScene"] = std::make_unique<GameOverScene>(m_width, m_height);

    int objCB_index = 0;
    int skinnedCB_index = 0;
    int textBatch_index = 0;

    m_nSKinnedCB = 0;
    m_nObjCB = 0;
    m_nTextBatch = 0;
    for (auto& scene_iter : m_Scenes)
    {
        auto& scene = scene_iter.second;
        std::string scene_name = scene_iter.first;

        // Set External-Resource to each Scene
        {
            scene->SetGeometriesRef(&(ExternalResource->GetGeometries()));
            scene->SetTexturesRef(&(ExternalResource->GetTextures()));
            scene->SetMaterialsRef(&(ExternalResource->GetMaterials()));
            scene->SetModelSkeletonsRef(&(ExternalResource->GetModelSkeltons()));
            scene->SetCharacterModelBoundingBoxesRef(&(ExternalResource->GetCharacterModelBoundingBoxes()));

            RenderTargetScene target_scene = RenderTargetScene::LoginScene;
            if      (scene_name == "LoginScene")    target_scene = RenderTargetScene::LoginScene;
            else if (scene_name == "LobyScene")     target_scene = RenderTargetScene::LobyScene;
            else if (scene_name == "PlayGameScene") target_scene = RenderTargetScene::PlayGameScene;
            else if (scene_name == "GameOverScene") target_scene = RenderTargetScene::GameOverScene;
            scene->SetRederItemsRef(&(ExternalResource->GetRenderItems(target_scene)));
        }

        scene->OnInit(m_device.Get(), m_commandList.Get(),
            objCB_index, skinnedCB_index, textBatch_index);

        m_nSKinnedCB += scene->m_nSKinnedCB;
        m_nObjCB += scene->m_nObjCB;
        m_nTextBatch += scene->m_nTextBatch;
    }

    m_CurrSceneName = "LobyScene";
    m_CurrScene = m_Scenes["LobyScene"].get();
}

void Framework::BuildFontSpriteBatchs()
{
    RenderTargetState rtState(m_BackBufferFormat, m_DepthStencilFormat);
    SpriteBatchPipelineStateDescription psoDesc(rtState);

    // Make SpriteBatch
    for (UINT i = 0; i < m_nTextBatch; ++i)
    {
        DirectX::ResourceUploadBatch SpriteBatch_upload(m_device.Get());
        SpriteBatch_upload.Begin(); // 자체적으로 CommandAllocator와 CommadList를 생성

        auto batch = std::make_unique<DirectX::SpriteBatch>(m_device.Get(), SpriteBatch_upload, psoDesc, &m_viewport);

        auto uploadResourcesFinished = SpriteBatch_upload.End(m_commandQueue.Get()); // CommadList 실행 및 FenceEvent 발생
        uploadResourcesFinished.wait();

        m_FontSpriteBatchs.push_back(std::move(batch));
    }
}

void Framework::BuildFrameResources()
{
    UINT PassCount = 2; // Common Render Pass + Shadow Render Pass
    UINT ObjectCount = m_nObjCB;
    UINT SkinnedCount = m_nSKinnedCB;
    UINT MaterialCount = m_nMatCB;

    for (int i = 0; i < gNumFrameResources; ++i)
    {
        m_FrameResources.push_back(std::make_unique<FrameResource>(m_device.Get(), PassCount, ObjectCount, SkinnedCount, MaterialCount));
        FrameResource* frame_resource = m_FrameResources.back().get();
    }

    m_CurrFrameResource = m_FrameResources[m_CurrFrameResourceIndex].get();
}

// Update frame-based values.
void Framework::OnUpdate(std::queue<std::unique_ptr<EVENT>>& GeneratedEvents, RECT* pClientRect)
{
    m_Timer.Tick(0.0f);

    if (m_KeyState[VK_F1] == true)
    {
        m_CurrSceneName = "LoginScene";
        m_CurrScene = m_Scenes[m_CurrSceneName].get();
        m_CurrScene->OnInitProperties(m_Timer);
    }
    else if (m_KeyState[VK_F2] == true)
    {
        m_CurrSceneName = "LobyScene";
        m_CurrScene = m_Scenes[m_CurrSceneName].get();
        m_CurrScene->OnInitProperties(m_Timer);
    }
    else if (m_KeyState[VK_F3] == true)
    {
        m_CurrSceneName = "PlayGameScene";
        m_CurrScene = m_Scenes[m_CurrSceneName].get();
        m_CurrScene->OnInitProperties(m_Timer);
    }
    else if (m_KeyState[VK_F4] == true)
    {
        m_CurrSceneName = "GameOverScene";
        m_CurrScene = m_Scenes[m_CurrSceneName].get();
        m_CurrScene->OnInitProperties(m_Timer);
    }

    RECT ClientRect;
    if (pClientRect != nullptr) ClientRect = *pClientRect;
    else ClientRect = { 0, 0, (LONG)m_width, (LONG)m_height };
    m_CurrScene->OnUpdate(m_CurrFrameResource, m_ShadowMap.get(), m_KeyState, m_OldCursorPos, ClientRect, m_Timer, GeneratedEvents);
}

// Render the scene.
void Framework::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    if(m_hasWindow == true) ThrowIfFailed(m_swapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void Framework::PopulateCommandList()
{
    auto cmdListAlloc = m_CurrFrameResource->CmdListAlloc;

    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(cmdListAlloc->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(cmdListAlloc.Get(), m_PSOs["opaque"].Get()));

    ID3D12DescriptorHeap* descriptorHeaps[] = { m_srvHeap.Get() };
    m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    // Shadow map pass.
    {
        m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

        // Bind null SRV for shadow map pass.
        m_commandList->SetGraphicsRootDescriptorTable(5, m_NullSrv);

        DrawSceneToShadowMap();
    }

    // Set CameraInfo(world, view, proj)
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));


    // Main render pass.
    {
        m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

        // Bind ShadowMap SRV for main render map pass.
        m_commandList->SetGraphicsRootDescriptorTable(5, m_ShadowMap->Srv());

        DrawSceneToBackBuffer();
    }

    // Frame capture
    {
        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));

        D3D12_TEXTURE_COPY_LOCATION dst, src;
        dst.pResource = m_ScreenshotBuffer.Get();
        dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        dst.PlacedFootprint.Offset = 0;
        dst.PlacedFootprint.Footprint.Format = m_BackBufferFormat;
        dst.PlacedFootprint.Footprint.Width = m_width;
        dst.PlacedFootprint.Footprint.Height = m_height;
        dst.PlacedFootprint.Footprint.Depth = 1;
        dst.PlacedFootprint.Footprint.RowPitch = m_width * 4;

        src.pResource = m_renderTargets[m_frameIndex].Get();
        src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        src.SubresourceIndex = 0;

        m_commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
    }

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
}

void Framework::DrawObjRenderLayer(ID3D12GraphicsCommandList* cmdList, const std::vector<Object*>& ObjLayer)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT skinnedCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SkinnedConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objCB = m_CurrFrameResource->ObjectCB->Resource();
    auto skinnedCB = m_CurrFrameResource->SkinnedCB->Resource();
    auto matCB = m_CurrFrameResource->MaterialCB->Resource();

    for (auto& obj : ObjLayer)
    {
        if (obj->Activated == false) continue;
        if (obj->m_Name == "SpawnStageGround") continue;
        if (obj->m_Name.find("Background") != std::string::npos) continue;
        UINT ObjCBIndex = obj->m_TransformInfo->ObjCBIndex;
        UINT SkinCBIndex = -1;
        if (obj->m_SkeletonInfo != nullptr)
            SkinCBIndex = obj->m_SkeletonInfo->SkinCBIndex;

        // 오브젝트가 동일한 Geo를 갖는 렌더아이템들로 구성되어 있을 경우에만 가능.
        // 현재 애플리케이션에선 캐릭터가 이에 해당됨.
        // (나머지 오브젝트는 오브젝트당 1개의 렌더아이템만 가지고 있으므로 아래의 메소드로도 처리 가능)
        if (obj->m_RenderItems.empty() == true) continue;
        auto& mainRitem = obj->m_RenderItems[0];

        m_commandList->IASetVertexBuffers(0, 1, &mainRitem->Geo->VertexBufferView());
        m_commandList->IASetIndexBuffer(&mainRitem->Geo->IndexBufferView());
        m_commandList->IASetPrimitiveTopology(mainRitem->PrimitiveType);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objCB->GetGPUVirtualAddress();
        if (ObjCBIndex != -1) objCBAddress += ObjCBIndex * objCBByteSize;
        m_commandList->SetGraphicsRootConstantBufferView(0, objCBAddress);

        D3D12_GPU_VIRTUAL_ADDRESS skinnedCBAddress = skinnedCB->GetGPUVirtualAddress();
        if (SkinCBIndex != -1) skinnedCBAddress += SkinCBIndex * skinnedCBByteSize;
        m_commandList->SetGraphicsRootConstantBufferView(1, skinnedCBAddress);

        for (auto& Ritem : obj->m_RenderItems)
        {
            // 렌더 아이템별로 매터리얼이 다르기 때문에 아래와 같이 렌더 아이템별로 MatCB를 따로 등록한다.
            if (Ritem->Mat != nullptr)
            {
                D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + Ritem->Mat->MatCBIndex * matCBByteSize;
                m_commandList->SetGraphicsRootConstantBufferView(2, matCBAddress);

                int TexIndex = (int)m_Textures.size() * m_CurrFrameResourceIndex + Ritem->Mat->DiffuseSrvHeapIndex;
                auto TexSrvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvHeap->GetGPUDescriptorHandleForHeapStart());
                TexSrvHandle.Offset(TexIndex, m_cbvsrvuavDescriptorSize);
                m_commandList->SetGraphicsRootDescriptorTable(4, TexSrvHandle);
            }
            else
            {
                D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress();
                m_commandList->SetGraphicsRootConstantBufferView(2, matCBAddress);
                auto TexSrvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvHeap->GetGPUDescriptorHandleForHeapStart());
                m_commandList->SetGraphicsRootDescriptorTable(4, TexSrvHandle);
            }

            m_commandList->DrawIndexedInstanced(Ritem->IndexCount, 1, Ritem->StartIndexLocation, Ritem->BaseVertexLocation, 0);
        }
    }
}

void Framework::ExcludeNonShadowRenderObjects(std::vector<Object*>& newRednerLayer, const std::vector<Object*> sourceRenderLayer)
{
    for (auto& obj : sourceRenderLayer)
    {
        auto TransformInfo = obj->m_TransformInfo.get();
        if (TransformInfo != nullptr)
        {
            if (TransformInfo->m_nonShadowRender == true) continue;
            if (TransformInfo->m_TexAlpha == 0.0f) continue;
        }
        newRednerLayer.push_back(obj);
    }
}

void Framework::DrawSceneToShadowMap()
{
    m_commandList->RSSetViewports(1, &m_ShadowMap->Viewport());
    m_commandList->RSSetScissorRects(1, &m_ShadowMap->ScissorRect());

    // Change to DEPTH_WRITE.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ShadowMap->Resource(),
        D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

    // Clear the back buffer and depth buffer.
    m_commandList->ClearDepthStencilView(m_ShadowMap->Dsv(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // 장면을 깊이 버퍼에만 렌더링할 것이므로 렌더 대상은 널로 설정한다.
    // 이처럼 널 렌더 대상을 지정하면 색상 쓰기가 비활성화된다.
    // 반드시 활성 PSO의 렌더 대상 개수도 0으로 지정해야 함을 주의해야 한다.
    m_commandList->OMSetRenderTargets(0, nullptr, false, &m_ShadowMap->Dsv());

    // Bind the pass constant buffer for the shadow map pass.
    UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
    auto passCB = m_CurrFrameResource->PassCB->Resource();
    D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress() + 1 * passCBByteSize;
    m_commandList->SetGraphicsRootConstantBufferView(3, passCBAddress);

    m_commandList->SetPipelineState(m_PSOs["shadow_opaque"].Get());
    auto& OpaqueObjRenderLayer = m_CurrScene->GetObjRenderLayer(RenderLayer::Opaque);
    std::vector<Object*> OnlyOpaqueObjRenderLayer;
    Framework::ExcludeNonShadowRenderObjects(OnlyOpaqueObjRenderLayer, OpaqueObjRenderLayer);
    DrawObjRenderLayer(m_commandList.Get(), OnlyOpaqueObjRenderLayer);

    m_commandList->SetPipelineState(m_PSOs["skinnedShadow_opaque"].Get());
    auto& SkinnedOpaqueObjRenderLayer = m_CurrScene->GetObjRenderLayer(RenderLayer::SkinnedOpaque);
    std::vector<Object*> OnlySkinnedOpaqueObjRenderLayer;
    Framework::ExcludeNonShadowRenderObjects(OnlySkinnedOpaqueObjRenderLayer, SkinnedOpaqueObjRenderLayer);
    DrawObjRenderLayer(m_commandList.Get(), OnlySkinnedOpaqueObjRenderLayer);

    // Change back to GENERIC_READ so we can read the texture in a shader.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ShadowMap->Resource(),
        D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void Framework::DrawSceneToBackBuffer()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    m_commandList->OMSetRenderTargets(1, &rtvHandle, TRUE, &m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    auto passCB = m_CurrFrameResource->PassCB->Resource();
    m_commandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());

    // Background render.
    DrawSceneToBackground();

    m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    m_commandList->SetPipelineState(m_PSOs["skinnedOpaque"].Get());
    auto& SkinnedOpaqueObjRenderLayer = m_CurrScene->GetObjRenderLayer(RenderLayer::SkinnedOpaque);
    DrawObjRenderLayer(m_commandList.Get(), SkinnedOpaqueObjRenderLayer);

    m_commandList->SetPipelineState(m_PSOs["opaque"].Get());
    auto& OpaqueObjRenderLayer = m_CurrScene->GetObjRenderLayer(RenderLayer::Opaque);
    DrawObjRenderLayer(m_commandList.Get(), OpaqueObjRenderLayer);

    m_commandList->SetPipelineState(m_PSOs["transparent"].Get());
    auto& TransparencyObjRenderLayer = m_CurrScene->GetObjRenderLayer(RenderLayer::Transparent);
    DrawObjRenderLayer(m_commandList.Get(), TransparencyObjRenderLayer);

    m_commandList->SetPipelineState(m_PSOs["skinnedTransparent"].Get());
    auto& SkinnedTransparencyObjRenderLayer = m_CurrScene->GetObjRenderLayer(RenderLayer::SkinnedTransparent);
    DrawObjRenderLayer(m_commandList.Get(), SkinnedTransparencyObjRenderLayer);

    // UI render.
    DrawSceneToUI();
}

void Framework::DrawSceneToUI()
{
    m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    m_commandList->SetPipelineState(m_PSOs["UILayout_Background"].Get());
    auto& UIObjRenderLayer = m_CurrScene->GetObjRenderLayer(RenderLayer::UILayout_Background);
    DrawObjRenderLayer(m_commandList.Get(), UIObjRenderLayer);

    auto& TextObjects = m_CurrScene->GetTextObjects();
    for (auto& obj : TextObjects)
    {
        if (obj->Activated == false) continue;
        if (obj->m_Textinfo == nullptr) continue;

        auto TextInfo = obj->m_Textinfo.get();
        if (TextInfo->m_Text.empty() == true) continue;
        auto font_iter = m_FontsRef->find(TextInfo->m_FontName);
        if (font_iter != m_FontsRef->end())
        {
            auto font_render = font_iter->second.get();
            auto TextBatch = m_FontSpriteBatchs[TextInfo->TextBatchIndex].get();
            font_render->DrawString(m_commandList.Get(), TextBatch,
                TextInfo->m_TextPos, TextInfo->m_TextPivot, TextInfo->m_TextColor,
                TextInfo->m_Text);
        }
    }
}

void Framework::DrawSceneToBackground()
{
    m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    m_commandList->SetPipelineState(m_PSOs["UILayout_Background"].Get());
    auto& UIObjRenderLayer = m_CurrScene->GetObjRenderLayer(RenderLayer::UILayout_Background);

    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT skinnedCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SkinnedConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objCB = m_CurrFrameResource->ObjectCB->Resource();
    auto skinnedCB = m_CurrFrameResource->SkinnedCB->Resource();
    auto matCB = m_CurrFrameResource->MaterialCB->Resource();

    for (auto& obj : UIObjRenderLayer)
    {
        if (obj->Activated == false) continue;
        if (obj->m_Name.find("Background") == std::string::npos) continue;

        UINT ObjCBIndex = obj->m_TransformInfo->ObjCBIndex;
        UINT SkinCBIndex = -1;
        if (obj->m_SkeletonInfo != nullptr)
            SkinCBIndex = obj->m_SkeletonInfo->SkinCBIndex;

        // 오브젝트가 동일한 Geo를 갖는 렌더아이템들로 구성되어 있을 경우에만 가능.
        // 현재 애플리케이션에선 캐릭터가 이에 해당됨.
        // (나머지 오브젝트는 오브젝트당 1개의 렌더아이템만 가지고 있으므로 아래의 메소드로도 처리 가능)
        auto& Ritem = obj->m_RenderItems[0];

        m_commandList->IASetVertexBuffers(0, 1, &Ritem->Geo->VertexBufferView());
        m_commandList->IASetIndexBuffer(&Ritem->Geo->IndexBufferView());
        m_commandList->IASetPrimitiveTopology(Ritem->PrimitiveType);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objCB->GetGPUVirtualAddress();
        if (ObjCBIndex != -1) objCBAddress += ObjCBIndex * objCBByteSize;
        m_commandList->SetGraphicsRootConstantBufferView(0, objCBAddress);

        D3D12_GPU_VIRTUAL_ADDRESS skinnedCBAddress = skinnedCB->GetGPUVirtualAddress();
        if (SkinCBIndex != -1) skinnedCBAddress += SkinCBIndex * skinnedCBByteSize;
        m_commandList->SetGraphicsRootConstantBufferView(1, skinnedCBAddress);

        if (Ritem->Mat != nullptr)
        {
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + Ritem->Mat->MatCBIndex * matCBByteSize;
            m_commandList->SetGraphicsRootConstantBufferView(2, matCBAddress);

            int TexIndex = (int)m_Textures.size() * m_CurrFrameResourceIndex + Ritem->Mat->DiffuseSrvHeapIndex;
            auto TexSrvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvHeap->GetGPUDescriptorHandleForHeapStart());
            TexSrvHandle.Offset(TexIndex, m_cbvsrvuavDescriptorSize);
            m_commandList->SetGraphicsRootDescriptorTable(4, TexSrvHandle);
        }
        else
        {
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress();
            m_commandList->SetGraphicsRootConstantBufferView(2, matCBAddress);
            auto TexSrvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvHeap->GetGPUDescriptorHandleForHeapStart());
            m_commandList->SetGraphicsRootDescriptorTable(4, TexSrvHandle);
        }

        m_commandList->DrawIndexedInstanced(Ritem->IndexCount, 1, Ritem->StartIndexLocation, Ritem->BaseVertexLocation, 0);
    }
}

void Framework::WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Cycle through the circular frame resource array.
    m_CurrFrameResource = m_FrameResources[m_CurrFrameResourceIndex].get();

    // Signal and increment the fence value.
    const UINT64 fence = m_fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
    m_CurrFrameResource->FenceValue = m_fenceValue++;

    // Wait until the previous frame is finished.
    if (m_fence->GetCompletedValue() < m_CurrFrameResource->FenceValue)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_CurrFrameResource->FenceValue, m_CurrFrameResource->FenceEvent));
        WaitForSingleObject(m_CurrFrameResource->FenceEvent, INFINITE);
    }

    if (m_hasWindow == true) m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    m_CurrFrameResourceIndex = (m_CurrFrameResourceIndex + 1) % gNumFrameResources;
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> Framework::GetStaticSamplers()
{
    // 그래픽 응용 프로그램이 사용하는 표본추출기의 수는 그리 많지 않으므로,
    // 미리 만들어서 루트 시그니처에 포함시켜 둔다.

    const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
        0, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
        1, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
        2, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
        3, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
        4, // shaderRegister
        D3D12_FILTER_ANISOTROPIC, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
        0.0f,                             // mipLODBias
        8);                               // maxAnisotropy

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
        5, // shaderRegister
        D3D12_FILTER_ANISOTROPIC, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
        0.0f,                              // mipLODBias
        8);                                // maxAnisotropy

    const CD3DX12_STATIC_SAMPLER_DESC shadow(
        6, // shaderRegister
        D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
        0.0f,                               // mipLODBias
        16,                                 // maxAnisotropy
        D3D12_COMPARISON_FUNC_LESS_EQUAL,
        D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

    return {
        pointWrap, pointClamp,
        linearWrap, linearClamp,
        anisotropicWrap, anisotropicClamp,
        shadow
    };
}