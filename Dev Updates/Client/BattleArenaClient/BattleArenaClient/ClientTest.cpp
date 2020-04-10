#include "stdafx.h"
#include "ClientTest.h"

const int gNumFrameResources = 3;

ClientTest::ClientTest(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height))
{
}

void ClientTest::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    for (int i = 0; i < gNumFrameResources; ++i)
        CloseHandle(m_FrameResources[i]->FenceEvent);
}

void ClientTest::OnInit()
{
    LoadPipeline();
    LoadAssets();

    m_Timer.Start();
    m_Timer.Reset();
}

// Load the rendering pipeline dependencies.
void ClientTest::LoadPipeline()
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
        Win32Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
        ));

    // This app does not support fullscreen transitions.
    ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

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
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
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

    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
}

// Load the assets.
void ClientTest::LoadAssets()
{
    BuildRootSignature();
    BuildShadersAndInputLayout();
    BuildPSOs();

    m_ShadowMap = std::make_unique<ShadowMap>(m_device.Get(), 2048, 2048);

    BuildScene();

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

    for (auto& scene : m_Scenes)
        scene.second->DisposeUploaders();
}

// Create the root signature.
void ClientTest::BuildRootSignature()
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

void ClientTest::BuildDescriptorHeaps()
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
void ClientTest::BuildConstantBufferView()
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

void ClientTest::BuildShadersAndInputLayout()
{
    const D3D_SHADER_MACRO skinnedDefines[] =
    {
        "SKINNED", "1",
        NULL, NULL
    };

    m_Shaders["standardVS"] = d3dUtil::CompileShader(L"Shaders/Default.hlsl", nullptr, "VS", "vs_5_1");
    m_Shaders["skinnedVS"] = d3dUtil::CompileShader(L"Shaders/Default.hlsl", skinnedDefines, "VS", "vs_5_1");
    m_Shaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders/Default.hlsl", nullptr, "PS", "ps_5_1");

    m_Shaders["shadowVS"] = d3dUtil::CompileShader(L"Shaders/Shadows.hlsl", nullptr, "VS", "vs_5_1");
    m_Shaders["skinnedShadowVS"] = d3dUtil::CompileShader(L"Shaders/Shadows.hlsl", skinnedDefines, "VS", "vs_5_1");
    m_Shaders["shadowOpaquePS"] = d3dUtil::CompileShader(L"Shaders/Shadows.hlsl", nullptr, "PS", "ps_5_1");

    m_Shaders["UIOpaqueVS"] = d3dUtil::CompileShader(L"Shaders/UI.hlsl", nullptr, "VS", "vs_5_1");
    m_Shaders["UIOpaquePS"] = d3dUtil::CompileShader(L"Shaders/UI.hlsl", nullptr, "PS", "ps_5_1");

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

void ClientTest::BuildPSOs()
{
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
    // PSO for skinned pass.
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
    D3D12_GRAPHICS_PIPELINE_STATE_DESC UIOpaquePsoDesc = opaquePsoDesc;
    UIOpaquePsoDesc.InputLayout = { m_InputLayout.data(), (UINT)m_InputLayout.size() };
    UIOpaquePsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(m_Shaders["UIOpaqueVS"]->GetBufferPointer()),
        m_Shaders["UIOpaqueVS"]->GetBufferSize()
    };
    UIOpaquePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(m_Shaders["UIOpaquePS"]->GetBufferPointer()),
        m_Shaders["UIOpaquePS"]->GetBufferSize()
    };
    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&UIOpaquePsoDesc, IID_PPV_ARGS(&m_PSOs["UI_opaque"])));
}

void ClientTest::BuildScene()
{
    m_Scenes["LoginScene"] = std::make_unique<LoginScene>(m_width, m_height);
    m_Scenes["PlayGameScene"] = std::make_unique<PlayGameScene>(m_width, m_height);

    int matCB_index = 0;
    int diffuseSrvHeap_index = 0;
    int objCB_index = 0;
    int skinnedCB_index = 0;
    for (auto& scene_iter : m_Scenes)
    {
        auto& scene = scene_iter.second;
        scene->OnInit(m_device.Get(), m_commandList.Get(),
            m_BackBufferFormat,
            matCB_index, diffuseSrvHeap_index, objCB_index, skinnedCB_index);

        // unique_ptr이라서 copy메소드를 쓸 수가 없다.
        /*m_AllRitems.insert(m_AllRitems.end(), scene->GetAllRitems().begin(), scene->GetAllRitems().end());
        m_Geometries.insert(std::begin(scene->GetGeometries()), std::end(scene->GetGeometries()));
        m_Materials.insert(std::begin(scene->GetMaterials()), std::end(scene->GetMaterials()));
        m_Textures.insert(std::begin(scene->GetTextures()), std::end(scene->GetTextures()));
        m_ModelSkeltons.insert(std::begin(scene->GetModelSkeltons()), std::end(scene->GetModelSkeltons()));*/
        auto& sceneAllRitems = scene->GetAllRitems();
        for (auto& Ritem : sceneAllRitems)
            m_AllRitems.push_back(Ritem.get());

        auto& sceneGeometries = scene->GetGeometries();
        for (auto& Geo_iter : sceneGeometries)
            m_Geometries.insert(std::make_pair<std::string, MeshGeometry*>(Geo_iter.first.c_str(), Geo_iter.second.get()));

        auto& sceneMaterials = scene->GetMaterials();
        for (auto& Mat_iter : sceneMaterials)
            m_Materials.insert(std::make_pair<std::string, Material*>(Mat_iter.first.c_str(), Mat_iter.second.get()));

        auto& sceneTextures = scene->GetTextures();
        for (auto& Tex_iter : sceneTextures)
            m_Textures.push_back(Tex_iter.second.get());

        auto& sceneModelSkeletons = scene->GetModelSkeltons();
        for (auto& ModelSkeleton_iter : sceneModelSkeletons)
            m_ModelSkeltons.insert(std::make_pair<std::string, aiModelData::aiSkeleton*>(ModelSkeleton_iter.first.c_str(), ModelSkeleton_iter.second.get()));
    }

    m_CurrSceneName = "PlayGameScene";
    m_CurrScene = m_Scenes["PlayGameScene"].get();
}

void ClientTest::BuildFrameResources()
{
    UINT PassCount = 2; // Common Render Pass + Shadow Render Pass
    UINT ObjectCount = (UINT)m_AllRitems.size();
    UINT SkinnedCount = (UINT)m_ModelSkeltons.size();
    UINT MaterialCount = (UINT)m_Materials.size();

    for (int i = 0; i < gNumFrameResources; ++i)
    {
        m_FrameResources.push_back(std::make_unique<FrameResource>(m_device.Get(), PassCount, ObjectCount, SkinnedCount, MaterialCount));
        FrameResource* frame_resource = m_FrameResources.back().get();

        ObjectConstants ObjectData;
        ObjectData.World = MathHelper::Identity4x4();
        XMStoreFloat4x4(&ObjectData.World, XMMatrixTranspose(XMLoadFloat4x4(&ObjectData.World)));

        SkinnedConstants SkinnedData;
        for (auto& skinnedTransform : SkinnedData.BoneTransform)
            XMStoreFloat4x4(&skinnedTransform, XMMatrixIdentity());

        for (UINT j = 0; j < ObjectCount; ++j)
            frame_resource->ObjectCB->CopyData(j, ObjectData);
        for (UINT j = 0; j < SkinnedCount; ++j)
            frame_resource->SkinnedCB->CopyData(j, SkinnedData);
    }

    m_CurrFrameResource = m_FrameResources[m_CurrFrameResourceIndex].get();
}

// Update frame-based values.
void ClientTest::OnUpdate()
{
    m_Timer.Tick(0.0f);

    static const std::string sceneNames[2] = { "LoginScene", "PlayGameScene" };
    static int sn = 0;
    static int sub = 0;
    float totalTime = m_Timer.GetTotalTime();
    int totalTime_i = (int)totalTime;
    if ((totalTime_i != 0) && (totalTime_i % 3 == 0))
    {
        if (sub < totalTime_i / 3)
        {
            sub = totalTime_i / 3;
            sn = (sn + 1) % 2;
            m_CurrSceneName = sceneNames[sn];
            m_CurrScene = m_Scenes[m_CurrSceneName].get();
            m_CurrScene->OnInitProperties();
        }
    }
    m_CurrScene->OnUpdate(m_CurrFrameResource, m_ShadowMap.get(), m_Timer);
}

// Render the scene.
void ClientTest::OnRender(HWND hwnd)
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    DrawSceneToTexts(hwnd);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void ClientTest::PopulateCommandList()
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

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
}

void ClientTest::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT skinnedCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SkinnedConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objCB = m_CurrFrameResource->ObjectCB->Resource();
    auto skinnedCB = m_CurrFrameResource->SkinnedCB->Resource();
    auto matCB = m_CurrFrameResource->MaterialCB->Resource();

    for (auto& e : ritems)
    {
        m_commandList->IASetVertexBuffers(0, 1, &e->Geo->VertexBufferView());
        m_commandList->IASetIndexBuffer(&e->Geo->IndexBufferView());
        m_commandList->IASetPrimitiveTopology(e->PrimitiveType);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objCB->GetGPUVirtualAddress() + e->ObjCBIndex * objCBByteSize;
        D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + e->Mat->MatCBIndex * matCBByteSize;

        m_commandList->SetGraphicsRootConstantBufferView(0, objCBAddress);
        if (e->Skeleton != nullptr)
        {
            D3D12_GPU_VIRTUAL_ADDRESS skinnedCBAddress = skinnedCB->GetGPUVirtualAddress() + e->SkinCBIndex * skinnedCBByteSize;
            m_commandList->SetGraphicsRootConstantBufferView(1, skinnedCBAddress);
        }
        else
            m_commandList->SetGraphicsRootConstantBufferView(1, 0);
        m_commandList->SetGraphicsRootConstantBufferView(2, matCBAddress);

        int TexIndex = (int)m_Textures.size() * m_CurrFrameResourceIndex + e->Mat->DiffuseSrvHeapIndex;
        auto TexSrvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvHeap->GetGPUDescriptorHandleForHeapStart());
        TexSrvHandle.Offset(TexIndex, m_cbvsrvuavDescriptorSize);
        m_commandList->SetGraphicsRootDescriptorTable(4, TexSrvHandle);

        m_commandList->DrawIndexedInstanced(e->IndexCount, 1, e->StartIndexLocation, e->BaseVertexLocation, 0);
    }
}

void ClientTest::DrawSceneToShadowMap()
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
    auto& OpaqueRitemLayer = m_CurrScene->GetRitemLayer(RenderLayer::Opaque);
    DrawRenderItems(m_commandList.Get(), OpaqueRitemLayer);

    m_commandList->SetPipelineState(m_PSOs["skinnedShadow_opaque"].Get());
    auto& SkinnedOpaqueRitemLayer = m_CurrScene->GetRitemLayer(RenderLayer::SkinnedOpaque);
    DrawRenderItems(m_commandList.Get(), SkinnedOpaqueRitemLayer);

    // Change back to GENERIC_READ so we can read the texture in a shader.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ShadowMap->Resource(),
        D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void ClientTest::DrawSceneToBackBuffer()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    m_commandList->OMSetRenderTargets(1, &rtvHandle, TRUE, &m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    auto passCB = m_CurrFrameResource->PassCB->Resource();
    m_commandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());

    m_commandList->SetPipelineState(m_PSOs["opaque"].Get());
    auto& OpaqueRitemLayer = m_CurrScene->GetRitemLayer(RenderLayer::Opaque);
    DrawRenderItems(m_commandList.Get(), OpaqueRitemLayer);

    m_commandList->SetPipelineState(m_PSOs["skinnedOpaque"].Get());
    auto& SkinnedOpaqueRitemLayer = m_CurrScene->GetRitemLayer(RenderLayer::SkinnedOpaque);
    DrawRenderItems(m_commandList.Get(), SkinnedOpaqueRitemLayer);

    // UI render.
    DrawSceneToUI();
}

void ClientTest::DrawSceneToUI()
{
    m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    m_commandList->SetPipelineState(m_PSOs["UI_opaque"].Get());
    auto& UIOpaqueRitemLayer = m_CurrScene->GetRitemLayer(RenderLayer::UIOpaque);
    DrawRenderItems(m_commandList.Get(), UIOpaqueRitemLayer);
}

void ClientTest::DrawSceneToTexts(HWND hwnd)
{
}

void ClientTest::WaitForPreviousFrame()
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

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    m_CurrFrameResourceIndex = (m_CurrFrameResourceIndex + 1) % gNumFrameResources;
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> ClientTest::GetStaticSamplers()
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