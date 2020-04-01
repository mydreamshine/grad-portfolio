#include "stdafx.h"
#include "ClientTest.h"

const int gNumFrameResources = 3;

ClientTest::ClientTest(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_rtvDescriptorSize(0)
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

    BuildShapeGeometry();
    LoadSkinnedModels();

    LoadTextures();

    BuildMaterials();
    BuildRenderItems();

    BuildDescriptorHeaps();
    BuildConstantBufferView();

    BuildFrameResources();

    BuildScene();

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

    for (auto& e : m_AllRitems)
    {
        if (e->Geo != nullptr)
            e->Geo->DisposeUploaders();
    }

    for (auto& tex_iter : m_Textures)
        tex_iter.second->UploadHeap = nullptr;
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
        for (auto& tex_iter : m_Textures)
        {
            int heapIndex = TextureCount * frameIndex + i;
            auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvHeap->GetCPUDescriptorHandleForHeapStart());
            handle.Offset(heapIndex, m_cbvsrvuavDescriptorSize);

            m_device->CreateShaderResourceView(tex_iter.second->Resource.Get(), &srvDesc, handle);

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
}

void ClientTest::BuildScene()
{
    // Estimate the scene bounding sphere manually since we know how the scene was constructed.
    // The grid is the "widest object" with a width of 500 and depth of 500.0f, and centered at
    // the world space origin.  In general, you need to loop over every world space vertex
    // position and compute the bounding sphere.
    m_SceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_SceneBounds.Radius = sqrtf(250.0f * 250.0f + 250.0f * 250.0f);
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

    UpdateMainPassCB(m_Timer);
}

void ClientTest::BuildShapeGeometry()
{
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData grid = geoGen.CreateGrid(500.0f, 500.0f, 50, 50);

    //
    // Concatenating all the geometry into one big vertex/index buffer.
    // So define the regions in the buffer each submesh covers.
    //

    // Cache the vertex offsets to each object in the concatenated vertex buffer.
    UINT gridVertexOffset = 0;

    // Cache the starting index for each object in the concatenated index buffer.
    UINT gridIndexOffset = 0;

    SubmeshGeometry gridSubmesh;
    gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
    gridSubmesh.StartIndexLocation = gridIndexOffset;
    gridSubmesh.BaseVertexLocation = gridVertexOffset;

    //
    // Extract the vertex elements we are interested in and pack the
    // vertices of all the meshes into one vertex buffer.
    //

    auto totalVertexCount = grid.Vertices.size();

    std::vector<DXTexturedVertex> vertices(totalVertexCount);

    UINT k = 0;
    for (int i = 0; i < grid.Vertices.size(); ++i, ++k)
    {
        vertices[k].xmf3Position = grid.Vertices[i].Position;
        vertices[k].xmf3Normal = grid.Vertices[i].Normal;
        vertices[k].xmf2TextureUV = grid.Vertices[i].TexC;
        vertices[k].xmf3Tangent = grid.Vertices[i].TangentU;
    }

    std::vector<std::uint16_t> indices;
    indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(DXTexturedVertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "shapeGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(m_device.Get(),
        m_commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(m_device.Get(),
        m_commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(DXTexturedVertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["grid"] = gridSubmesh;

    m_Geometries[geo->Name] = std::move(geo);
}

void ClientTest::LoadSkinnedModels()
{
    std::string mesh_path = "Models/Meshtint Free Knight/Meshtint Free Knight.fbx";
    std::vector<std::string> anim_paths = { "Models/Meshtint Free Knight/Animations/Meshtint Free Knight@Battle Idle.fbx" };
    LoadSkinnedModelData(mesh_path, anim_paths);

    mesh_path = "Models/ToonyTinyPeople/TT_RTS_Demo_Character.fbx";
    anim_paths = { "Models/ToonyTinyPeople/Animations/infantry_01_idle.fbx" };
    LoadSkinnedModelData(mesh_path, anim_paths);

    mesh_path = "Models/claire@Dancing.fbx";
    anim_paths = { "Models/claire@Dancing.fbx" };
    LoadSkinnedModelData(mesh_path, anim_paths);

    mesh_path = "Models/Soldier_demo/Soldier_demo.fbx";
    anim_paths = { "Models/Soldier_demo/Animations/demo_mixamo_idle.fbx" };
    LoadSkinnedModelData(mesh_path, anim_paths);
}

void ClientTest::LoadSkinnedModelData(const std::string& mesh_filepath, const std::vector<std::string>& anim_filepaths)
{
    m_ModelLoader.loadMeshAndSkeleton(mesh_filepath);
    for (auto& anim_path : anim_filepaths)
        m_ModelLoader.loadAnimation(anim_path);

    std::string ModelName, Anim0_Name;
    getFileName(mesh_filepath.c_str(), ModelName);
    getFileName(anim_filepaths[0].c_str(), Anim0_Name);

    // aiSkeleton data move
    if (m_ModelLoader.mSkeleton != nullptr)
    {
        std::string skeletonName = m_ModelLoader.mSkeleton->mName;
        auto animSk = std::move(m_ModelLoader.mSkeleton);
        animSk->AnimPlay(Anim0_Name);
        animSk->AnimLoop(Anim0_Name, true);
        animSk->SkinnedCBIndex = (int)m_ModelSkeltons.size();
        m_ModelSkeltons[skeletonName] = std::move(animSk);
    }

    // aiMesh to dxMesh
    if (!m_ModelLoader.mMeshes.empty())
    {
        auto& meshGeo = m_Geometries[ModelName] = std::make_unique<MeshGeometry>();
        meshGeo->Name = ModelName;

        std::vector<DXSkinnedVertex> vertices;
        std::vector<std::uint32_t>   indices;

        int sm_id = 0;
        for (size_t m = 0; m < m_ModelLoader.mMeshes.size(); ++m)
        {
            std::vector<aiModelData::aiVertex>& aiVertices = m_ModelLoader.mMeshes[m].mVertices;
            std::vector<UINT>& dxIndices = m_ModelLoader.mMeshes[m].mIndices;
            std::vector<DXSkinnedVertex> dxVertieces;
            for (size_t v = 0; v < aiVertices.size(); ++v)
            {
                dxVertieces.emplace_back();
                DXSkinnedVertex& Vertex = dxVertieces.back();
                Vertex.xmf3Position = { (float)aiVertices[v].vPosition.x, (float)aiVertices[v].vPosition.y, (float)aiVertices[v].vPosition.z };
                Vertex.xmf2TextureUV = { (float)aiVertices[v].vTextureUV[0].x,  (float)aiVertices[v].vTextureUV[0].y }; // diffuseMap_desc + normalMap_desc + etc.texMap + ...
                Vertex.xmf3Normal = { (float)aiVertices[v].vNormal.x, (float)aiVertices[v].vNormal.y, (float)aiVertices[v].vNormal.z };
                Vertex.xmf3Tangent = { (float)aiVertices[v].vTangent.x, (float)aiVertices[v].vTangent.y, (float)aiVertices[v].vTangent.z };
                for (int w = 0; w < 4; ++w)
                {
                    Vertex.fBoneWeights[w] = (float)aiVertices[v].fBoneWeights[w];
                    Vertex.i32BoneIndices[w] = (int)aiVertices[v].i32BoneIndices[w];
                }
            }
            vertices.insert(vertices.end(), dxVertieces.begin(), dxVertieces.end());
            indices.insert(indices.end(), dxIndices.begin(), dxIndices.end());

            SubmeshGeometry submesh;
            submesh.IndexCount = (UINT)dxIndices.size();
            submesh.StartIndexLocation = (UINT)(indices.size() - dxIndices.size());
            submesh.BaseVertexLocation = (INT)(vertices.size() - dxVertieces.size());

            std::string& meshName = m_ModelLoader.mMeshes[m].mName;
            if (meshGeo->DrawArgs.find(meshName) != meshGeo->DrawArgs.end())
                meshName = meshName + "_" + std::to_string(sm_id++);
            meshGeo->DrawArgs[meshName] = submesh;
        }

        const UINT vbByteSize = (UINT)vertices.size() * sizeof(DXSkinnedVertex);
        const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

        ThrowIfFailed(D3DCreateBlob(vbByteSize, &meshGeo->VertexBufferCPU));
        CopyMemory(meshGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

        ThrowIfFailed(D3DCreateBlob(ibByteSize, &meshGeo->IndexBufferCPU));
        CopyMemory(meshGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

        meshGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(m_device.Get(),
            m_commandList.Get(), vertices.data(), vbByteSize, meshGeo->VertexBufferUploader);

        meshGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(m_device.Get(),
            m_commandList.Get(), indices.data(), ibByteSize, meshGeo->IndexBufferUploader);

        meshGeo->VertexByteStride = sizeof(DXSkinnedVertex);
        meshGeo->VertexBufferByteSize = vbByteSize;
        meshGeo->IndexFormat = DXGI_FORMAT_R32_UINT;
        meshGeo->IndexBufferByteSize = ibByteSize;
    }
}

void ClientTest::LoadTextures()
{
    std::vector<std::string> material_filepaths = 
    {
        "Models/Meshtint Free Knight/Materials/Meshtint Free Knight.tga",
        "Models/ToonyTinyPeople/Materials/TT_RTS_Units_blue.tga",
        "Models/Soldier_demo/Materials/demo_soldier_512.tga",
        "Models/Soldier_demo/Materials/demo_weapon.tga"
    };

    for (auto& texture_path : material_filepaths)
    {
        TextureLoader texLoader(texture_path.c_str());
        TextureData texInfo;
        texLoader.MoveTextureData(texInfo);
        if (texInfo.Pixels == nullptr) continue;
        std::vector<std::uint8_t>& Pixels = *texInfo.Pixels;
        if (Pixels.empty()) continue;

        UINT texWidth = (UINT)texInfo.Width;
        UINT texHeight = (UINT)texInfo.Height;
        UINT texPixelSize = texInfo.BytesPerPixel;

        std::string texName;
        getFileName(texture_path.c_str(), texName);
        auto& tex = m_Textures[texName] = std::make_unique<Texture>();
        tex->Name = texName;

        // Describe and create a Texture2D.
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = m_BackBufferFormat;
        textureDesc.Width = texWidth;
        textureDesc.Height = texHeight;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&tex->Resource)));

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(tex->Resource.Get(), 0, 1);

        // Create the GPU upload buffer.
        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&tex->UploadHeap)));

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = Pixels.data();
        textureData.RowPitch = texWidth * texPixelSize;
        textureData.SlicePitch = textureData.RowPitch * texHeight;

        UpdateSubresources(m_commandList.Get(), tex->Resource.Get(), tex->UploadHeap.Get(), 0, 0, 1, &textureData);
        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(tex->Resource.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    }

    // Create the texture.
    {
        auto& tex = m_Textures["checkerboard"] = std::make_unique<Texture>();
        tex->Name = "checkerboard";

        // Describe and create a Texture2D.
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Width = 256;
        textureDesc.Height = 256;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&tex->Resource)));

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(tex->Resource.Get(), 0, 1);

        // Create the GPU upload buffer.
        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&tex->UploadHeap)));

        // Copy data to the intermediate upload heap and then schedule a copy 
        // from the upload heap to the Texture2D.
        std::vector<UINT8> texture = GenerateTextureData();

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = &texture[0];
        textureData.RowPitch = 256 * 4;
        textureData.SlicePitch = textureData.RowPitch * 256;

        UpdateSubresources(m_commandList.Get(), tex->Resource.Get(), tex->UploadHeap.Get(), 0, 0, 1, &textureData);
        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(tex->Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    }
}

// Generate a simple black and white checkerboard texture.
std::vector<UINT8> ClientTest::GenerateTextureData()
{
    const UINT rowPitch = 256 * 4;
    const UINT cellPitch = rowPitch >> 4;        // The width of a cell in the checkboard texture.
    const UINT cellHeight = 256 >> 4;    // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * 256;

    std::vector<UINT8> data(textureSize);
    UINT8* pData = &data[0];

    for (UINT n = 0; n < textureSize; n += 4)
    {
        UINT x = n % rowPitch;
        UINT y = n / rowPitch;
        UINT i = x / cellPitch;
        UINT j = y / cellHeight;

        if (i % 2 == j % 2)
        {
            pData[n] = 0x00;        // R
            pData[n + 1] = 0x00;    // G
            pData[n + 2] = 0x00;    // B
            pData[n + 3] = 0xff;    // A
        }
        else
        {
            pData[n] = 0xff;        // R
            pData[n + 1] = 0xff;    // G
            pData[n + 2] = 0xff;    // B
            pData[n + 3] = 0xff;    // A
        }
    }

    return data;
}

void ClientTest::BuildMaterials()
{
    int matCB_index = 0;
    int diffuseSrvHeap_Index = 0;
    for (auto& tex_iter : m_Textures)
    {
        auto& tex = tex_iter.second;

        auto mat = std::make_unique<Material>();
        mat->Name = tex->Name;
        mat->NumFramesDirty = gNumFrameResources;
        mat->MatCBIndex = matCB_index++;
        mat->DiffuseSrvHeapIndex = diffuseSrvHeap_Index++;
        mat->DiffuseAlbedo = XMFLOAT4(0.88f, 0.88f, 0.88f, 1.0f);
        mat->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
        mat->Roughness = 0.8f;

        m_Materials[mat->Name] = std::move(mat);
    }
}

void ClientTest::BuildRenderItems()
{
    UINT ObjCBIndex = 0;
    UINT SkinCBIndex = 0;

    MeshGeometry* Geo = m_Geometries["Meshtint Free Knight"].get();
    for (auto& subMesh : Geo->DrawArgs)
    {
        auto ModelRitem = std::make_unique<RenderItem>();
        ModelRitem->NumFramesDirty = gNumFrameResources;
        XMMATRIX PosM = XMMatrixTranslation(-100.0f, 0.0f, -100.0f);
        XMStoreFloat4x4(&(ModelRitem->World), PosM);
        ModelRitem->TexTransform = MathHelper::Identity4x4();
        ModelRitem->ObjCBIndex = ObjCBIndex++;
        ModelRitem->Geo = m_Geometries["Meshtint Free Knight"].get();
        ModelRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        ModelRitem->Mat = m_Materials["Meshtint Free Knight"].get();
        ModelRitem->Skeleton = m_ModelSkeltons["Meshtint Free Knight"].get();
        ModelRitem->SkinCBIndex = SkinCBIndex;
        ModelRitem->IndexCount = subMesh.second.IndexCount;
        ModelRitem->StartIndexLocation = subMesh.second.StartIndexLocation;
        ModelRitem->BaseVertexLocation = subMesh.second.BaseVertexLocation;
        m_RitemLayer[(int)RenderLayer::SkinnedOpaque].push_back(ModelRitem.get());
        m_AllRitems.push_back(std::move(ModelRitem));
    }

    SkinCBIndex++;
    Geo = m_Geometries["TT_RTS_Demo_Character"].get();
    for (auto& subMesh : Geo->DrawArgs)
    {
        auto ModelRitem = std::make_unique<RenderItem>();
        ModelRitem->NumFramesDirty = gNumFrameResources;
        XMMATRIX PosM = XMMatrixTranslation(100.0f, 0.0f, -100.0f);
        XMStoreFloat4x4(&(ModelRitem->World), PosM);
        ModelRitem->TexTransform = MathHelper::Identity4x4();
        ModelRitem->ObjCBIndex = ObjCBIndex++;
        ModelRitem->Geo = m_Geometries["TT_RTS_Demo_Character"].get();
        ModelRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        ModelRitem->Mat = m_Materials["TT_RTS_Units_blue"].get();
        ModelRitem->Skeleton = m_ModelSkeltons["TT_RTS_Demo_Character"].get();
        ModelRitem->SkinCBIndex = SkinCBIndex;
        ModelRitem->IndexCount = subMesh.second.IndexCount;
        ModelRitem->StartIndexLocation = subMesh.second.StartIndexLocation;
        ModelRitem->BaseVertexLocation = subMesh.second.BaseVertexLocation;
        m_RitemLayer[(int)RenderLayer::SkinnedOpaque].push_back(ModelRitem.get());
        m_AllRitems.push_back(std::move(ModelRitem));
    }

    SkinCBIndex++;
    Geo = m_Geometries["claire@Dancing"].get();
    for (auto& subMesh : Geo->DrawArgs)
    {
        auto ModelRitem = std::make_unique<RenderItem>();
        ModelRitem->NumFramesDirty = gNumFrameResources;
        XMMATRIX PosM = XMMatrixTranslation(-100.0f, 0.0f, 100.0f);
        XMStoreFloat4x4(&(ModelRitem->World), PosM);
        ModelRitem->TexTransform = MathHelper::Identity4x4();
        ModelRitem->ObjCBIndex = ObjCBIndex++;
        ModelRitem->Geo = m_Geometries["claire@Dancing"].get();
        ModelRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        ModelRitem->Mat = m_Materials["checkerboard"].get();
        ModelRitem->Skeleton = m_ModelSkeltons["claire@Dancing"].get();
        ModelRitem->SkinCBIndex = SkinCBIndex;
        ModelRitem->IndexCount = subMesh.second.IndexCount;
        ModelRitem->StartIndexLocation = subMesh.second.StartIndexLocation;
        ModelRitem->BaseVertexLocation = subMesh.second.BaseVertexLocation;
        m_RitemLayer[(int)RenderLayer::SkinnedOpaque].push_back(ModelRitem.get());
        m_AllRitems.push_back(std::move(ModelRitem));
    }

    SkinCBIndex++;
    Geo = m_Geometries["Soldier_demo"].get();
    for (auto& subMesh : Geo->DrawArgs)
    {
        auto ModelRitem = std::make_unique<RenderItem>();
        ModelRitem->NumFramesDirty = gNumFrameResources;
        XMMATRIX PosM = XMMatrixTranslation(100.0f, 0.0f, 100.0f);
        XMStoreFloat4x4(&(ModelRitem->World), PosM);
        ModelRitem->TexTransform = MathHelper::Identity4x4();
        ModelRitem->ObjCBIndex = ObjCBIndex++;
        ModelRitem->Geo = m_Geometries["Soldier_demo"].get();
        ModelRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        if(subMesh.first == "Soldier_mesh_0")
            ModelRitem->Mat = m_Materials["demo_weapon"].get();
        else
            ModelRitem->Mat = m_Materials["demo_soldier_512"].get();
        ModelRitem->Skeleton = m_ModelSkeltons["Soldier_demo"].get();
        ModelRitem->SkinCBIndex = SkinCBIndex;
        ModelRitem->IndexCount = subMesh.second.IndexCount;
        ModelRitem->StartIndexLocation = subMesh.second.StartIndexLocation;
        ModelRitem->BaseVertexLocation = subMesh.second.BaseVertexLocation;
        m_RitemLayer[(int)RenderLayer::SkinnedOpaque].push_back(ModelRitem.get());
        m_AllRitems.push_back(std::move(ModelRitem));
    }

    Geo = m_Geometries["shapeGeo"].get();
    for (auto& subMesh : Geo->DrawArgs)
    {
        auto ModelRitem = std::make_unique<RenderItem>();
        ModelRitem->NumFramesDirty = gNumFrameResources;
        ModelRitem->World = MathHelper::Identity4x4();
        ModelRitem->TexTransform = MathHelper::Identity4x4();
        ModelRitem->ObjCBIndex = ObjCBIndex++;
        ModelRitem->Geo = m_Geometries["shapeGeo"].get();
        ModelRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        ModelRitem->Mat = m_Materials["checkerboard"].get();
        ModelRitem->IndexCount = subMesh.second.IndexCount;
        ModelRitem->StartIndexLocation = subMesh.second.StartIndexLocation;
        ModelRitem->BaseVertexLocation = subMesh.second.BaseVertexLocation;
        m_RitemLayer[(int)RenderLayer::Opaque].push_back(ModelRitem.get());
        m_AllRitems.push_back(std::move(ModelRitem));
    }
}

// Update frame-based values.
void ClientTest::OnUpdate()
{
    m_Timer.Tick(0.0f);

    AnimateLights(m_Timer);
    AnimateSkeletons(m_Timer);

    UpdateObjectCBs(m_Timer);
    UpdateSkinnedCBs(m_Timer);
    UpdateMaterialCBs(m_Timer);
    UpdateShadowTransform(m_Timer);
    UpdateMainPassCB(m_Timer);
    UpdateShadowPassCB(m_Timer);
}

void ClientTest::AnimateLights(CTimer& gt)
{
    m_LightRotationAngle += 1.0f * gt.GetTimeElapsed();

    XMMATRIX R = XMMatrixRotationY(m_LightRotationAngle);
    for (int i = 0; i < 3; ++i)
    {
        XMVECTOR lightDir = XMLoadFloat3(&m_BaseLightDirections[i]);
        lightDir = XMVector3TransformNormal(lightDir, R);
        XMStoreFloat3(&m_RotatedLightDirections[i], lightDir);
    }
}

void ClientTest::AnimateSkeletons(CTimer& gt)
{
    for (auto& skleton_iter : m_ModelSkeltons)
    {
        auto skleton = skleton_iter.second.get();
        std::string animName = skleton->mCurrPlayingAnimName;
        if (skleton->AnimIsPlaying(animName))
            skleton->UpdateAnimationTransforms(animName, m_Timer.GetTimeElapsed());
    }
}

void ClientTest::UpdateObjectCBs(CTimer& gt)
{
    auto currObjectCB = m_CurrFrameResource->ObjectCB.get();
    for (auto& e : m_AllRitems)
    {
        // Only update the cbuffer data if the constants have changed.  
        // This needs to be tracked per frame resource.
        if (e->NumFramesDirty > 0)
        {
            XMMATRIX world = XMLoadFloat4x4(&e->World);
            XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

            ObjectConstants objConstants;
            XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
            XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

            currObjectCB->CopyData(e->ObjCBIndex, objConstants);

            // Next FrameResource need to be updated too.
            e->NumFramesDirty--;
        }
    }
}

void aiM2dxM(XMFLOAT4X4& dest, aiMatrix4x4& source)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
            dest.m[i][j] = source[i][j];
    }
}
void ClientTest::UpdateSkinnedCBs(CTimer& gt)
{
    auto currSkinnedCB = m_CurrFrameResource->SkinnedCB.get();
    for(auto& skleton_iter : m_ModelSkeltons)
    {
        auto skleton = skleton_iter.second.get();
        std::string animName = skleton->mCurrPlayingAnimName;
        if (skleton->AnimIsPlaying(animName))
        {
            SkinnedConstants SkinnedInfo;
            for (size_t i = 0; i < skleton->mCurrAnimTransforms.size(); ++i)
                aiM2dxM(SkinnedInfo.BoneTransform[i], skleton->mCurrAnimTransforms[i]);

            currSkinnedCB->CopyData(skleton->SkinnedCBIndex, SkinnedInfo);
        }
    }
}

void ClientTest::UpdateMaterialCBs(CTimer& gt)
{
    auto currMaterialCB = m_CurrFrameResource->MaterialCB.get();
    for (auto& e : m_Materials)
    {
        // Only update the cbuffer data if the constants have changed.  If the cbuffer
        // data changes, it needs to be updated for each FrameResource.
        Material* mat = e.second.get();
        if (mat->NumFramesDirty > 0)
        {
            XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

            MaterialConstants matConstants;
            matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
            matConstants.FresnelR0 = mat->FresnelR0;
            matConstants.Roughness = mat->Roughness;
            XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

            currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

            // Next FrameResource need to be updated too.
            mat->NumFramesDirty--;
        }
    }
}

void ClientTest::UpdateMainPassCB(CTimer& gt)
{
    XMFLOAT3 EyePosition = { 0.0f, 300.0f, -500.0f };
    XMFLOAT3 LookAtPosition = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 UpDirection = { 0.0f, 1.0f, 0.0f };

    m_Camera.SetPosition(EyePosition);
    m_Camera.SetLens(XM_PIDIV4, (float)m_width / m_height, 1.0f, 1000.0f);
    m_Camera.LookAt(EyePosition, LookAtPosition, UpDirection);
    m_Camera.UpdateViewMatrix();

    XMMATRIX view = m_Camera.GetView();
    XMMATRIX proj = m_Camera.GetProj();

    XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
    XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
    XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    XMMATRIX viewProjTex = XMMatrixMultiply(viewProj, T);
    XMMATRIX shadowTransform = XMLoadFloat4x4(&m_ShadowTransform);

    XMStoreFloat4x4(&m_MainPassCB.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&m_MainPassCB.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&m_MainPassCB.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&m_MainPassCB.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&m_MainPassCB.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&m_MainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    XMStoreFloat4x4(&m_MainPassCB.ViewProjTex, XMMatrixTranspose(viewProjTex));
    XMStoreFloat4x4(&m_MainPassCB.ShadowTransform, XMMatrixTranspose(shadowTransform));
    m_MainPassCB.EyePosW = m_Camera.GetPosition3f();
    m_MainPassCB.RenderTargetSize = XMFLOAT2((float)m_width, (float)m_height);
    m_MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / m_width, 1.0f / m_height);
    m_MainPassCB.NearZ = 1.0f;
    m_MainPassCB.FarZ = 1000.0f;
    m_MainPassCB.TotalTime = gt.GetTotalTime();
    m_MainPassCB.DeltaTime = gt.GetTimeElapsed();
    m_MainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
    m_MainPassCB.Lights[0].Direction = m_RotatedLightDirections[0];
    m_MainPassCB.Lights[0].Strength = { 0.9f, 0.9f, 0.7f };
    m_MainPassCB.Lights[1].Direction = m_RotatedLightDirections[1];
    m_MainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
    m_MainPassCB.Lights[2].Direction = m_RotatedLightDirections[2];
    m_MainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

    auto currPassCB = m_CurrFrameResource->PassCB.get();
    currPassCB->CopyData(0, m_MainPassCB);
}

void ClientTest::UpdateShadowPassCB(CTimer& gt)
{
    XMMATRIX view = XMLoadFloat4x4(&m_LightView);
    XMMATRIX proj = XMLoadFloat4x4(&m_LightProj);

    XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
    XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
    XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

    UINT w = m_ShadowMap->Width();
    UINT h = m_ShadowMap->Height();

    XMStoreFloat4x4(&m_ShadowPassCB.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&m_ShadowPassCB.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&m_ShadowPassCB.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&m_ShadowPassCB.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&m_ShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&m_ShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    m_ShadowPassCB.EyePosW = m_LightPosW;
    m_ShadowPassCB.RenderTargetSize = XMFLOAT2((float)w, (float)h);
    m_ShadowPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / w, 1.0f / h);
    m_ShadowPassCB.NearZ = m_LightNearZ;
    m_ShadowPassCB.FarZ = m_LightFarZ;

    auto currPassCB = m_CurrFrameResource->PassCB.get();
    currPassCB->CopyData(1, m_ShadowPassCB);
}

void ClientTest::UpdateShadowTransform(CTimer& gt)
{
    // Only the first "main" light casts a shadow.
    XMVECTOR lightDir = XMLoadFloat3(&m_RotatedLightDirections[0]);
    XMVECTOR lightPos = -2.0f * m_SceneBounds.Radius * lightDir;
    XMVECTOR targetPos = XMLoadFloat3(&m_SceneBounds.Center);
    XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

    XMStoreFloat3(&m_LightPosW, lightPos);

    // Transform bounding sphere to light space.
    XMFLOAT3 sphereCenterLS;
    XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

    // Ortho frustum in light space encloses scene.
    float l = sphereCenterLS.x - m_SceneBounds.Radius;
    float b = sphereCenterLS.y - m_SceneBounds.Radius;
    float n = sphereCenterLS.z - m_SceneBounds.Radius;
    float r = sphereCenterLS.x + m_SceneBounds.Radius;
    float t = sphereCenterLS.y + m_SceneBounds.Radius;
    float f = sphereCenterLS.z + m_SceneBounds.Radius;

    m_LightNearZ = n;
    m_LightFarZ = f;
    // 직교 투영 행렬
    XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    XMMATRIX S = lightView * lightProj * T;
    XMStoreFloat4x4(&m_LightView, lightView);
    XMStoreFloat4x4(&m_LightProj, lightProj);
    XMStoreFloat4x4(&m_ShadowTransform, S);
}

// Render the scene.
void ClientTest::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

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

    // Main rendering pass.
    {
        m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

        // Bind null SRV for shadow map pass.
        m_commandList->SetGraphicsRootDescriptorTable(5, m_ShadowMap->Srv());

        DrawSceneToBackBuffer();
    }

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

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
    DrawRenderItems(m_commandList.Get(), m_RitemLayer[(int)RenderLayer::Opaque]);

    m_commandList->SetPipelineState(m_PSOs["skinnedShadow_opaque"].Get());
    DrawRenderItems(m_commandList.Get(), m_RitemLayer[(int)RenderLayer::SkinnedOpaque]);

    // Change back to GENERIC_READ so we can read the texture in a shader.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ShadowMap->Resource(),
        D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void ClientTest::DrawSceneToBackBuffer()
{
    // Set CameraInfo(world, view, proj)
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    m_commandList->OMSetRenderTargets(1, &rtvHandle, TRUE, &m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    auto passCB = m_CurrFrameResource->PassCB->Resource();
    m_commandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());

    m_commandList->SetPipelineState(m_PSOs["opaque"].Get());
    DrawRenderItems(m_commandList.Get(), m_RitemLayer[(int)RenderLayer::Opaque]);

    m_commandList->SetPipelineState(m_PSOs["skinnedOpaque"].Get());
    DrawRenderItems(m_commandList.Get(), m_RitemLayer[(int)RenderLayer::SkinnedOpaque]);
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