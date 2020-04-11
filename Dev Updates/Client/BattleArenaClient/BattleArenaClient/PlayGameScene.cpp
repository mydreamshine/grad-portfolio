#include "stdafx.h"
#include "PlayGameScene.h"

PlayGameScene::~PlayGameScene()
{
}

void PlayGameScene::OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
    DXGI_FORMAT BackBufferFormat,
    int& matCB_index, int& diffuseSrvHeap_Index, int& objCB_index, int& skinnedCB_index)
{
    Scene::OnInit(device, commandList,
        BackBufferFormat,
        matCB_index, diffuseSrvHeap_Index, objCB_index, skinnedCB_index);

    // Estimate the scene bounding sphere manually since we know how the scene was constructed.
    // The grid is the "widest object" with a width of 500 and depth of 500.0f, and centered at
    // the world space origin.  In general, you need to loop over every world space vertex
    // position and compute the bounding sphere.
    m_SceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_SceneBounds.Radius = sqrtf(250.0f * 250.0f + 250.0f * 250.0f);
}

void PlayGameScene::OnInitProperties()
{
    for (auto& e : m_AllRitems)
    {
        if (e->Geo->Name == "Meshtint Free Knight")
        {
            e->NumFramesDirty = gNumFrameResources;
            XMMATRIX PosM = XMMatrixTranslation(-100.0f, 0.0f, -100.0f);
            XMStoreFloat4x4(&(e->World), PosM);
            e->TexTransform = MathHelper::Identity4x4();
            e->Skeleton->AnimStop(e->Skeleton->mCurrPlayingAnimName);
            e->Skeleton->AnimPlay("Meshtint Free Knight@Battle Idle");
        }
        else if (e->Geo->Name == "TT_RTS_Demo_Character")
        {
            e->NumFramesDirty = gNumFrameResources;
            XMMATRIX PosM = XMMatrixTranslation(100.0f, 0.0f, -100.0f);
            XMStoreFloat4x4(&(e->World), PosM);
            e->TexTransform = MathHelper::Identity4x4();
            e->Skeleton->AnimStop(e->Skeleton->mCurrPlayingAnimName);
            e->Skeleton->AnimPlay("infantry_01_idle");
        }
        else if (e->Geo->Name == "claire@Dancing")
        {
            e->NumFramesDirty = gNumFrameResources;
            XMMATRIX PosM = XMMatrixTranslation(-100.0f, 0.0f, 100.0f);
            XMStoreFloat4x4(&(e->World), PosM);
            e->TexTransform = MathHelper::Identity4x4();
            e->Skeleton->AnimStop(e->Skeleton->mCurrPlayingAnimName);
            e->Skeleton->AnimPlay("claire@Dancing");
        }
        else if (e->Geo->Name == "Soldier_demo")
        {
            e->NumFramesDirty = gNumFrameResources;
            XMMATRIX PosM = XMMatrixTranslation(100.0f, 0.0f, 100.0f);
            XMStoreFloat4x4(&(e->World), PosM);
            e->TexTransform = MathHelper::Identity4x4();
            e->Skeleton->AnimStop(e->Skeleton->mCurrPlayingAnimName);
            e->Skeleton->AnimPlay("demo_mixamo_idle");
        }
    }

    XMFLOAT3 EyePosition = { 0.0f, 300.0f, -500.0f };
    XMFLOAT3 LookAtPosition = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 UpDirection = { 0.0f, 1.0f, 0.0f };

    m_MainCamera.SetPosition(EyePosition);
    m_MainCamera.SetPerspectiveLens(XM_PIDIV4, (float)m_width / m_height, 1.0f, 1000.0f);
    m_MainCamera.LookAt(EyePosition, LookAtPosition, UpDirection);
    m_MainCamera.UpdateViewMatrix();

    m_LightRotationAngle = 0.0f;
    m_BaseLightDirections[0] = m_BaseLightDirections[0] = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
    m_BaseLightDirections[1] = m_BaseLightDirections[1] = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);
    m_BaseLightDirections[2] = m_BaseLightDirections[2] = XMFLOAT3(0.0f, -0.707f, -0.707f);
}

void PlayGameScene::OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map, CTimer& gt)
{
    Scene::OnUpdate(frame_resource, shadow_map, gt);
}

void PlayGameScene::DisposeUploaders()
{
    Scene::DisposeUploaders();
}

void PlayGameScene::BuildShapeGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData grid = geoGen.CreateGrid(500.0f, 500.0f, 50, 50);

    // UI Position relative to DC(Device Coordinate) with origin at screen center.
    GeometryGenerator::MeshData UI_quad1 = geoGen.CreateQuad(-(float)m_width / 2 + 10.0f, (float)m_height / 2 - 10.0f, 400.0f, 150.0f, 0.0f);
    GeometryGenerator::MeshData UI_quad2 = geoGen.CreateQuad((float)m_width / 2 - 410.0f, (float)m_height / 2 - 10.0f, 400.0f, 150.0f, 0.0f);
    GeometryGenerator::MeshData UI_quad3 = geoGen.CreateQuad(-200.0f, -(float)m_height / 2 + 160.0f, 400.0f, 150.0f, 0.0f);

    //
    // Concatenating all the geometry into one big vertex/index buffer.
    // So define the regions in the buffer each submesh covers.
    //

    // Cache the vertex offsets to each object in the concatenated vertex buffer.
    UINT gridVertexOffset = 0;
    UINT UI_quad1_VertexOffset = (UINT)grid.Vertices.size();
    UINT UI_quad2_VertexOffset = UI_quad1_VertexOffset + (UINT)UI_quad1.Vertices.size();
    UINT UI_quad3_VertexOffset = UI_quad2_VertexOffset + (UINT)UI_quad2.Vertices.size();

    // Cache the starting index for each object in the concatenated index buffer.
    UINT gridIndexOffset = 0;
    UINT UI_quad1_IndexOffset = (UINT)grid.Indices32.size();
    UINT UI_quad2_IndexOffset = UI_quad1_IndexOffset + (UINT)UI_quad1.Indices32.size();
    UINT UI_quad3_IndexOffset = UI_quad2_IndexOffset + (UINT)UI_quad2.Indices32.size();

    SubmeshGeometry gridSubmesh;
    gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
    gridSubmesh.StartIndexLocation = gridIndexOffset;
    gridSubmesh.BaseVertexLocation = gridVertexOffset;

    SubmeshGeometry quad1Submesh;
    quad1Submesh.IndexCount = (UINT)UI_quad1.Indices32.size();
    quad1Submesh.StartIndexLocation = UI_quad1_IndexOffset;
    quad1Submesh.BaseVertexLocation = UI_quad1_VertexOffset;

    SubmeshGeometry quad2Submesh;
    quad2Submesh.IndexCount = (UINT)UI_quad2.Indices32.size();
    quad2Submesh.StartIndexLocation = UI_quad2_IndexOffset;
    quad2Submesh.BaseVertexLocation = UI_quad2_VertexOffset;

    SubmeshGeometry quad3Submesh;
    quad3Submesh.IndexCount = (UINT)UI_quad3.Indices32.size();
    quad3Submesh.StartIndexLocation = UI_quad3_IndexOffset;
    quad3Submesh.BaseVertexLocation = UI_quad3_VertexOffset;

    //
    // Extract the vertex elements we are interested in and pack the
    // vertices of all the meshes into one vertex buffer.
    //

    auto totalVertexCount =
        grid.Vertices.size() +
        UI_quad1.Vertices.size() +
        UI_quad2.Vertices.size() +
        UI_quad3.Vertices.size();

    std::vector<DXTexturedVertex> vertices(totalVertexCount);

    UINT k = 0;
    for (int i = 0; i < grid.Vertices.size(); ++i, ++k)
    {
        vertices[k].xmf3Position = grid.Vertices[i].Position;
        vertices[k].xmf3Normal = grid.Vertices[i].Normal;
        vertices[k].xmf2TextureUV = grid.Vertices[i].TexC;
        vertices[k].xmf3Tangent = grid.Vertices[i].TangentU;
    }

    for (int i = 0; i < UI_quad1.Vertices.size(); ++i, ++k)
    {
        vertices[k].xmf3Position = UI_quad1.Vertices[i].Position;
        vertices[k].xmf3Normal = UI_quad1.Vertices[i].Normal;
        vertices[k].xmf2TextureUV = UI_quad1.Vertices[i].TexC;
        vertices[k].xmf3Tangent = UI_quad1.Vertices[i].TangentU;
    }

    for (int i = 0; i < UI_quad2.Vertices.size(); ++i, ++k)
    {
        vertices[k].xmf3Position = UI_quad2.Vertices[i].Position;
        vertices[k].xmf3Normal = UI_quad2.Vertices[i].Normal;
        vertices[k].xmf2TextureUV = UI_quad2.Vertices[i].TexC;
        vertices[k].xmf3Tangent = UI_quad2.Vertices[i].TangentU;
    }

    for (int i = 0; i < UI_quad3.Vertices.size(); ++i, ++k)
    {
        vertices[k].xmf3Position = UI_quad3.Vertices[i].Position;
        vertices[k].xmf3Normal = UI_quad3.Vertices[i].Normal;
        vertices[k].xmf2TextureUV = UI_quad3.Vertices[i].TexC;
        vertices[k].xmf3Tangent = UI_quad3.Vertices[i].TangentU;
    }

    std::vector<std::uint16_t> indices;
    indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
    indices.insert(indices.end(), std::begin(UI_quad1.GetIndices16()), std::end(UI_quad1.GetIndices16()));
    indices.insert(indices.end(), std::begin(UI_quad2.GetIndices16()), std::end(UI_quad2.GetIndices16()));
    indices.insert(indices.end(), std::begin(UI_quad3.GetIndices16()), std::end(UI_quad3.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(DXTexturedVertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "shapeGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device, commandList,
        vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device, commandList,
        indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(DXTexturedVertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["grid"] = gridSubmesh;
    geo->DrawArgs["UI_quad1"] = quad1Submesh;
    geo->DrawArgs["UI_quad2"] = quad2Submesh;
    geo->DrawArgs["UI_quad3"] = quad3Submesh;

    m_Geometries[geo->Name] = std::move(geo);
}

void PlayGameScene::LoadSkinnedModels(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    ModelLoader model_loader;
    std::string mesh_path = "Models/Meshtint Free Knight/Meshtint Free Knight.fbx";
    std::vector<std::string> anim_paths = { "Models/Meshtint Free Knight/Animations/Meshtint Free Knight@Battle Idle.fbx" };
    LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths);

    mesh_path = "Models/ToonyTinyPeople/TT_RTS_Demo_Character.fbx";
    anim_paths = { "Models/ToonyTinyPeople/Animations/infantry_01_idle.fbx" };
    LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths);

    mesh_path = "Models/claire@Dancing.fbx";
    anim_paths = { "Models/claire@Dancing.fbx" };
    LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths);

    mesh_path = "Models/Soldier_demo/Soldier_demo.fbx";
    anim_paths = { "Models/Soldier_demo/Animations/demo_mixamo_idle.fbx" };
    LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths);
}

void PlayGameScene::LoadTextures(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DXGI_FORMAT BackBufferFormat)
{
    std::vector<std::string> material_filepaths =
    {
        "Models/Meshtint Free Knight/Materials/Meshtint Free Knight.tga",
        "Models/ToonyTinyPeople/Materials/TT_RTS_Units_blue.tga",
        "Models/Soldier_demo/Materials/demo_soldier_512.tga",
        "Models/Soldier_demo/Materials/demo_weapon.tga",
        "UI/ui_test.tga"
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
        textureDesc.Format = BackBufferFormat;
        textureDesc.Width = texWidth;
        textureDesc.Height = texHeight;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&tex->Resource)));

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(tex->Resource.Get(), 0, 1);

        // Create the GPU upload buffer.
        ThrowIfFailed(device->CreateCommittedResource(
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

        UpdateSubresources(commandList, tex->Resource.Get(), tex->UploadHeap.Get(), 0, 0, 1, &textureData);
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(tex->Resource.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    }

    // Create the non-tex texture.
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

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&tex->Resource)));

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(tex->Resource.Get(), 0, 1);

        // Create the GPU upload buffer.
        ThrowIfFailed(device->CreateCommittedResource(
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

        UpdateSubresources(commandList, tex->Resource.Get(), tex->UploadHeap.Get(), 0, 0, 1, &textureData);
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(tex->Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    }
}

void PlayGameScene::BuildMaterials(int& matCB_index, int& diffuseSrvHeap_Index)
{
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

void PlayGameScene::BuildRenderItems(int& objCB_index, int& skinnedCB_index)
{
    MeshGeometry* Geo = m_Geometries["Meshtint Free Knight"].get();
    for (auto& subMesh : Geo->DrawArgs)
    {
        auto ModelRitem = std::make_unique<RenderItem>();
        ModelRitem->NumFramesDirty = gNumFrameResources;
        XMMATRIX PosM = XMMatrixTranslation(-100.0f, 0.0f, -100.0f);
        XMStoreFloat4x4(&(ModelRitem->World), PosM);
        ModelRitem->TexTransform = MathHelper::Identity4x4();
        ModelRitem->ObjCBIndex = objCB_index++;
        ModelRitem->Geo = m_Geometries["Meshtint Free Knight"].get();
        ModelRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        ModelRitem->Mat = m_Materials["Meshtint Free Knight"].get();
        ModelRitem->Skeleton = m_ModelSkeltons["Meshtint Free Knight"].get();
        ModelRitem->SkinCBIndex = skinnedCB_index;
        ModelRitem->IndexCount = subMesh.second.IndexCount;
        ModelRitem->StartIndexLocation = subMesh.second.StartIndexLocation;
        ModelRitem->BaseVertexLocation = subMesh.second.BaseVertexLocation;
        m_RitemLayer[(int)RenderLayer::SkinnedOpaque].push_back(ModelRitem.get());
        m_AllRitems.push_back(std::move(ModelRitem));
    }

    skinnedCB_index++;
    Geo = m_Geometries["TT_RTS_Demo_Character"].get();
    for (auto& subMesh : Geo->DrawArgs)
    {
        auto ModelRitem = std::make_unique<RenderItem>();
        ModelRitem->NumFramesDirty = gNumFrameResources;
        XMMATRIX PosM = XMMatrixTranslation(100.0f, 0.0f, -100.0f);
        XMStoreFloat4x4(&(ModelRitem->World), PosM);
        ModelRitem->TexTransform = MathHelper::Identity4x4();
        ModelRitem->ObjCBIndex = objCB_index++;
        ModelRitem->Geo = m_Geometries["TT_RTS_Demo_Character"].get();
        ModelRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        ModelRitem->Mat = m_Materials["TT_RTS_Units_blue"].get();
        ModelRitem->Skeleton = m_ModelSkeltons["TT_RTS_Demo_Character"].get();
        ModelRitem->SkinCBIndex = skinnedCB_index;
        ModelRitem->IndexCount = subMesh.second.IndexCount;
        ModelRitem->StartIndexLocation = subMesh.second.StartIndexLocation;
        ModelRitem->BaseVertexLocation = subMesh.second.BaseVertexLocation;
        m_RitemLayer[(int)RenderLayer::SkinnedOpaque].push_back(ModelRitem.get());
        m_AllRitems.push_back(std::move(ModelRitem));
    }

    skinnedCB_index++;
    Geo = m_Geometries["claire@Dancing"].get();
    for (auto& subMesh : Geo->DrawArgs)
    {
        auto ModelRitem = std::make_unique<RenderItem>();
        ModelRitem->NumFramesDirty = gNumFrameResources;
        XMMATRIX PosM = XMMatrixTranslation(-100.0f, 0.0f, 100.0f);
        XMStoreFloat4x4(&(ModelRitem->World), PosM);
        ModelRitem->TexTransform = MathHelper::Identity4x4();
        ModelRitem->ObjCBIndex = objCB_index++;
        ModelRitem->Geo = m_Geometries["claire@Dancing"].get();
        ModelRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        ModelRitem->Mat = m_Materials["checkerboard"].get();
        ModelRitem->Skeleton = m_ModelSkeltons["claire@Dancing"].get();
        ModelRitem->SkinCBIndex = skinnedCB_index;
        ModelRitem->IndexCount = subMesh.second.IndexCount;
        ModelRitem->StartIndexLocation = subMesh.second.StartIndexLocation;
        ModelRitem->BaseVertexLocation = subMesh.second.BaseVertexLocation;
        m_RitemLayer[(int)RenderLayer::SkinnedOpaque].push_back(ModelRitem.get());
        m_AllRitems.push_back(std::move(ModelRitem));
    }

    skinnedCB_index++;
    Geo = m_Geometries["Soldier_demo"].get();
    for (auto& subMesh : Geo->DrawArgs)
    {
        auto ModelRitem = std::make_unique<RenderItem>();
        ModelRitem->NumFramesDirty = gNumFrameResources;
        XMMATRIX PosM = XMMatrixTranslation(100.0f, 0.0f, 100.0f);
        XMStoreFloat4x4(&(ModelRitem->World), PosM);
        ModelRitem->TexTransform = MathHelper::Identity4x4();
        ModelRitem->ObjCBIndex = objCB_index++;
        ModelRitem->Geo = m_Geometries["Soldier_demo"].get();
        ModelRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        if (subMesh.first == "Soldier_mesh_0")
            ModelRitem->Mat = m_Materials["demo_weapon"].get();
        else
            ModelRitem->Mat = m_Materials["demo_soldier_512"].get();
        ModelRitem->Skeleton = m_ModelSkeltons["Soldier_demo"].get();
        ModelRitem->SkinCBIndex = skinnedCB_index;
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
        ModelRitem->ObjCBIndex = objCB_index++;
        ModelRitem->Geo = m_Geometries["shapeGeo"].get();
        ModelRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        ModelRitem->IndexCount = subMesh.second.IndexCount;
        ModelRitem->StartIndexLocation = subMesh.second.StartIndexLocation;
        ModelRitem->BaseVertexLocation = subMesh.second.BaseVertexLocation;
        if (subMesh.first.find("UI") != std::string::npos)
        {
            m_RitemLayer[(int)RenderLayer::UIOpaque].push_back(ModelRitem.get());
            ModelRitem->Mat = m_Materials["ui_test"].get();
        }
        else
        {
            m_RitemLayer[(int)RenderLayer::Opaque].push_back(ModelRitem.get());
            ModelRitem->Mat = m_Materials["checkerboard"].get();
        }
        m_AllRitems.push_back(std::move(ModelRitem));
    }
}

std::vector<UINT8> PlayGameScene::GenerateTextureData()
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

void PlayGameScene::UpdateObjectCBs(UploadBuffer<ObjectConstants>* objCB, CTimer& gt)
{
    Scene::UpdateObjectCBs(objCB, gt);
}

void PlayGameScene::UpdateSkinnedCBs(UploadBuffer<SkinnedConstants>* skinnedCB, CTimer& gt)
{
    Scene::UpdateSkinnedCBs(skinnedCB, gt);
}

void PlayGameScene::UpdateMaterialCBs(UploadBuffer<MaterialConstants>* matCB, CTimer& gt)
{
    Scene::UpdateMaterialCBs(matCB, gt);
}

void PlayGameScene::UpdateMainPassCB(UploadBuffer<PassConstants>* passCB, CTimer& gt)
{
    Scene::UpdateMainPassCB(passCB, gt);
}

void PlayGameScene::UpdateShadowPassCB(UploadBuffer<PassConstants>* passCB, ShadowMap* shadow_map, CTimer& gt)
{
    Scene::UpdateShadowPassCB(passCB, shadow_map, gt);
}

void PlayGameScene::UpdateShadowTransform(CTimer& gt)
{
    Scene::UpdateShadowTransform(gt);
}

void PlayGameScene::AnimateLights(CTimer& gt)
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

void PlayGameScene::AnimateSkeletons(CTimer& gt)
{
    for (auto& skleton_iter : m_ModelSkeltons)
    {
        auto skleton = skleton_iter.second.get();
        std::string animName = skleton->mCurrPlayingAnimName;
        if (skleton->AnimIsPlaying(animName))
            skleton->UpdateAnimationTransforms(animName, gt.GetTimeElapsed());
    }
}

void PlayGameScene::AnimateCameras(CTimer& gt)
{
    XMFLOAT3 EyePosition = { 0.0f, 300.0f, -500.0f };
    XMFLOAT3 LookAtPosition = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 UpDirection = { 0.0f, 1.0f, 0.0f };

    m_MainCamera.SetPosition(EyePosition);
    m_MainCamera.SetPerspectiveLens(XM_PIDIV4, (float)m_width / m_height, 1.0f, 1000.0f);
    m_MainCamera.LookAt(EyePosition, LookAtPosition, UpDirection);
    m_MainCamera.UpdateViewMatrix();
}
