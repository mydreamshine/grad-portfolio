#include "stdafx.h"
#include "ResourceManager.h"

using namespace std;
using namespace DirectX;

ResourceManager::~ResourceManager()
{
}

void ResourceManager::OnInit(
    ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
    DXGI_FORMAT BackBufferFormat,
    int& matCB_index, int& diffuseSrvHeap_Index)
{
    // 아래 메소드 순서는 반드시 지켜져야 한다.
    ResourceManager::BuildShapeGeometry(device, commandList);
    ResourceManager::LoadSkinnedModels(device, commandList);
    ResourceManager::LoadTextures(device, commandList, BackBufferFormat);
    ResourceManager::BuildMaterials(matCB_index, diffuseSrvHeap_Index);
    ResourceManager::BuildRenderItems();
}

void ResourceManager::DisposeUploaders()
{
    for (auto& Geo_iter : m_Geometries)
        Geo_iter.second->DisposeUploaders();

    for (auto& tex_iter : m_Textures)
        tex_iter.second->UploadHeap = nullptr;
}

std::unique_ptr<MeshGeometry> ResourceManager::BuildMeshGeometry(
	ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
	const std::string& geoName,
	std::unordered_map<std::string, GeometryGenerator::MeshData>& Meshes)
{
    std::vector<DXTexturedVertex> total_vertices;
    std::vector<std::uint16_t> total_indices;

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = geoName;
    UINT VertexOffset = 0;
    UINT IndexOffset = 0;
    UINT k = 0;

    for (auto& mesh_iter : Meshes)
    {
        auto& meshData = mesh_iter.second;

        total_vertices.insert(total_vertices.end(), meshData.Vertices.size(), DXTexturedVertex());
        DirectX::XMFLOAT3 minPosition = { FLT_MAX, FLT_MAX, FLT_MAX };
        DirectX::XMFLOAT3 maxPosition = meshData.Vertices[0].Position;

        for (int i = 0; i < meshData.Vertices.size(); ++i, ++k)
        {
            total_vertices[k].xmf3Position = meshData.Vertices[i].Position;
            total_vertices[k].xmf3Normal = meshData.Vertices[i].Normal;
            total_vertices[k].xmf2TextureUV = meshData.Vertices[i].TexC;
            total_vertices[k].xmf3Tangent = meshData.Vertices[i].TangentU;

            minPosition.x = min(minPosition.x, total_vertices[k].xmf3Position.x);
            minPosition.y = min(minPosition.y, total_vertices[k].xmf3Position.y);
            minPosition.z = min(minPosition.z, total_vertices[k].xmf3Position.z);
            maxPosition.x = max(maxPosition.x, total_vertices[k].xmf3Position.x);
            maxPosition.y = max(maxPosition.y, total_vertices[k].xmf3Position.y);
            maxPosition.z = max(maxPosition.z, total_vertices[k].xmf3Position.z);
        }
        total_indices.insert(total_indices.end(), std::begin(meshData.GetIndices16()), std::end(meshData.GetIndices16()));

        SubmeshGeometry Submesh;
        Submesh.IndexCount = (UINT)meshData.Indices32.size();
        Submesh.StartIndexLocation = IndexOffset;
        Submesh.BaseVertexLocation = VertexOffset;
        Submesh.Bounds.Center = {
                (maxPosition.x + minPosition.x) * 0.5f,
                (maxPosition.y + minPosition.y) * 0.5f,
                (maxPosition.z + minPosition.z) * 0.5f };
        Submesh.Bounds.Extents = {
            (maxPosition.x - minPosition.x) * 0.5f,
            (maxPosition.y - minPosition.y) * 0.5f,
            (maxPosition.z - minPosition.z) * 0.5f };

        geo->DrawArgs[mesh_iter.first] = Submesh;

        VertexOffset += (UINT)meshData.Vertices.size();
        IndexOffset += (UINT)meshData.Indices32.size();
    }

    const UINT vbByteSize = (UINT)total_vertices.size() * sizeof(DXTexturedVertex);
    const UINT ibByteSize = (UINT)total_indices.size() * sizeof(std::uint16_t);

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), total_vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), total_indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device, commandList,
        total_vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device, commandList,
        total_indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(DXTexturedVertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;
    return std::move(geo);
}

void ResourceManager::BuildShapeGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    GeometryGenerator geoGen;
    float wnd_width = (float)WND_WIDTH;
    float wnd_height = (float)WND_HEIGHT;
    float wnd_top = wnd_height / 2;
    float wnd_bottom = -wnd_top;
    float wnd_right = wnd_width / 2;
    float wnd_left = -wnd_right;


    // UI Position relative to DC(Device Coordinate) with origin at screen center.

    // LoginScene UI Geometry
    {
        std::unordered_map<std::string, GeometryGenerator::MeshData> Meshes;
        Meshes["UI_Layout_LoginSceneBackground"] = geoGen.CreateQuad(wnd_left, wnd_top, wnd_width, wnd_height, 0.0f);
        Meshes["UI_Layout_IdBox"] = geoGen.CreateQuad(-100.0f, -100, 200.0f, 75.0f, 0.0f);
        Meshes["UI_Layout_PasswordBox"] = geoGen.CreateQuad(-100.0f, -205.0f, 200.0f, 75.0f, 0.0f);

        m_Geometries["LoginSceneUIGeo"]
            = std::move(ResourceManager::BuildMeshGeometry(device, commandList, "LoginSceneUIGeo", Meshes));
    }

    // LobyScene UI Geometry
    {
        std::unordered_map<std::string, GeometryGenerator::MeshData> Meshes;
        Meshes["UI_Layout_LobySceneBackground"]            = geoGen.CreateQuad(-640.0f,  360.0f, 1280.0f, 720.0f, 0.0f);
        Meshes["UI_Layout_LobyChattingLog"]                = geoGen.CreateQuad(-600.0f,  228.0f, 200.0f,  300.0f, 0.0f);
        Meshes["UI_Layout_MatchWaiting"]                   = geoGen.CreateQuad(-600.0f, -105.0f, 200.0f,  60.0f,  0.0f);
        Meshes["UI_Layout_GameStartButton"]                = geoGen.CreateQuad(-600.0f, -197.9f, 200.0f,  100.0f, 0.0f);
        Meshes["UI_Layout_LobyCharacterName"]              = geoGen.CreateQuad(-100.0f, -212.9f, 200.0f,  70.0f,  0.0f);
        Meshes["UI_Layout_CharacterSelection_LeftButton"]  = geoGen.CreateQuad(-192.0f, -212.9f, 70.0f,   70.0f,  0.0f);
        Meshes["UI_Layout_CharacterSelection_RightButton"] = geoGen.CreateQuad( 122.0f, -212.9f, 70.0f,   70.0f,  0.0f);
        Meshes["UI_Layout_LobyUserInfo"]                   = geoGen.CreateQuad( 400.0f,  320.0f, 200.0f,  60.0f,  0.0f);
        Meshes["UI_Layout_LobyCharacterDescrition"]        = geoGen.CreateQuad( 400.0f,  137.5f, 200.0f,  275.0f, 0.0f);

        m_Geometries["LobySceneUIGeo"]
            = std::move(ResourceManager::BuildMeshGeometry(device, commandList, "LobySceneUIGeo", Meshes));
    }

    // PlayGameScene Geometry
    {
        std::unordered_map<std::string, GeometryGenerator::MeshData> UILayerBacground_Meshes;
        UILayerBacground_Meshes["UI_Layout_GameTimeLimit"] = geoGen.CreateQuad(-50.0f,   360.0F, 100.0f, 60.0f,  0.0f);
        UILayerBacground_Meshes["UI_Layout_KDA"]           = geoGen.CreateQuad(-640.0f,  360.0f, 200.0f, 70.0f,  0.0f);
        UILayerBacground_Meshes["UI_Layout_KillLog"]       = geoGen.CreateQuad(-640.0f,  280.0f, 200.0f, 200.0f, 0.0f);
        UILayerBacground_Meshes["UI_Layout_ChattingLog"]   = geoGen.CreateQuad(-640.0f,  20.0f,  200.0f, 300.0f, 0.0f);
        UILayerBacground_Meshes["UI_Layout_SkillList"]     = geoGen.CreateQuad(-250.0f, -280.0f, 500.0f, 80.0f,  0.1f);
        UILayerBacground_Meshes["UI_Layout_Skill1"]        = geoGen.CreateQuad(-200.0f, -295.0f, 50.0f,  50.0f,  0.0f);
        UILayerBacground_Meshes["UI_Layout_Skill2"]        = geoGen.CreateQuad(-80.0f,  -295.0f, 50.0f,  50.0f,  0.0f);
        UILayerBacground_Meshes["UI_Layout_Skill3"]        = geoGen.CreateQuad(30.0f,   -295.0f, 50.0f,  50.0f,  0.0f);
        UILayerBacground_Meshes["UI_Layout_Skill4"]        = geoGen.CreateQuad(140.0f,  -295.0f, 50.0f,  50.0f,  0.0f);

        m_Geometries["PlayGameSceneUIGeo"]
            = std::move(ResourceManager::BuildMeshGeometry(device, commandList, "PlayGameSceneUIGeo", UILayerBacground_Meshes));

        std::unordered_map<std::string, GeometryGenerator::MeshData> StageGround_Meshes;
        StageGround_Meshes["SpawnStageGround"] = geoGen.CreateGrid(1000.0f, 1000.0f, 10, 10);
        m_Geometries["SpawnStageGround"]
            = std::move(ResourceManager::BuildMeshGeometry(device, commandList, "SpawnStageGround", StageGround_Meshes));

        std::unordered_map<std::string, GeometryGenerator::MeshData> EffectGeo_Meshs;
        EffectGeo_Meshs["SkillEffect_SwordSlash_a"]  = geoGen.CreateGrid(326.0f, 200.0f, 10, 10);
        EffectGeo_Meshs["PickingEffect_CrossTarget"] = geoGen.CreateGrid(150.0f, 150.0f, 10, 10);
        m_Geometries["GameEffectGeo"]
            = std::move(ResourceManager::BuildMeshGeometry(device, commandList, "GameEffectGeo", EffectGeo_Meshs));
    }

    // GameOverScene UI Geometry
    {
        std::unordered_map<std::string, GeometryGenerator::MeshData> Meshes;
        Meshes["UI_Layout_GameOverBackground"] = geoGen.CreateQuad(-640.0f,  360.0f, 1280.0f, 720.0f, 0.0f);
        Meshes["UI_Layout_GameOverInfo"]       = geoGen.CreateQuad(-550.0f,  270.0f, 320.0f,  380.0f, 0.0f);
        Meshes["UI_Layout_GameOverResult"]     = geoGen.CreateQuad(-85.0f,   340.0f, 170.0f,  70.0f,  0.0f);
        Meshes["UI_Layout_ReturnMainMenu"]     = geoGen.CreateQuad(-100.0f, -210.0f, 200.0f,  80.0f,  0.0f);

        m_Geometries["GameOverSceneUIGeo"]
            = std::move(ResourceManager::BuildMeshGeometry(device, commandList, "GameOverSceneUIGeo", Meshes));
    }
}

void ResourceManager::LoadSkinnedModelData(
    ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
    ModelLoader& model_loader,
    const std::string& mesh_filepath,
    const std::vector<std::string>& anim_filepaths,
    std::vector<std::string>* execpt_nodes)
{
    model_loader.loadMeshAndSkeleton(mesh_filepath, execpt_nodes);
    for (auto& anim_path : anim_filepaths)
        model_loader.loadAnimation(anim_path);

    std::string ModelName;
    getFileName(mesh_filepath.c_str(), ModelName);

    // aiSkeleton data move
    if (model_loader.mSkeleton != nullptr)
    {
        std::string skeletonName = model_loader.mSkeleton->mName;
        m_ModelSkeltons[skeletonName] = std::move(model_loader.mSkeleton);
    }

    // aiMesh to dxMesh
    if (!model_loader.mMeshes.empty())
    {
        auto& meshGeo = m_Geometries[ModelName] = std::make_unique<MeshGeometry>();
        meshGeo->Name = ModelName;

        std::vector<DXSkinnedVertex> vertices;
        std::vector<std::uint32_t>   indices;

        int sm_id = 0;
        for (size_t m = 0; m < model_loader.mMeshes.size(); ++m)
        {
            std::vector<aiModelData::aiVertex>& aiVertices = model_loader.mMeshes[m].mVertices;
            std::vector<UINT>& dxIndices = model_loader.mMeshes[m].mIndices;
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
            auto& aabb = model_loader.mMeshes[m].mAABB;
            submesh.Bounds.Center = {
                (aabb.mMax.x + aabb.mMin.x) * 0.5f,
                (aabb.mMax.y + aabb.mMin.y) * 0.5f,
                (aabb.mMax.z + aabb.mMin.z) * 0.5f };
            submesh.Bounds.Extents = {
                (aabb.mMax.x - aabb.mMin.x) * 0.5f,
                (aabb.mMax.y - aabb.mMin.y) * 0.5f,
                (aabb.mMax.z - aabb.mMin.z) * 0.5f };
            submesh.ParentBoneID = model_loader.mMeshes[m].mParentBoneIndex;

            std::string& meshName = model_loader.mMeshes[m].mName;
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

        meshGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device,
            commandList, vertices.data(), vbByteSize, meshGeo->VertexBufferUploader);

        meshGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device,
            commandList, indices.data(), ibByteSize, meshGeo->IndexBufferUploader);

        meshGeo->VertexByteStride = sizeof(DXSkinnedVertex);
        meshGeo->VertexBufferByteSize = vbByteSize;
        meshGeo->IndexFormat = DXGI_FORMAT_R32_UINT;
        meshGeo->IndexBufferByteSize = ibByteSize;
    }
}

void ResourceManager::LoadSkinnedModels(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    ModelLoader model_loader;

    std::string mesh_path = "Models/Environment/Environment.fbx";
    std::vector<std::string> anim_paths;
    std::vector<std::string> execptProcessing_file_nodes = { "Environment_root", "RootNode" };
    ResourceManager::LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths, &execptProcessing_file_nodes);

    mesh_path = "Models/Meshtint Free Knight/Meshtint Free Knight.fbx";
    anim_paths = {
        "Models/Meshtint Free Knight/Animations/Meshtint Free Knight@Battle Idle.fbx",
        "Models/Meshtint Free Knight/Animations/Meshtint Free Knight@Stride Walking.fbx",
        "Models/Meshtint Free Knight/Animations/Meshtint Free Knight@Sword And Shield Walk.fbx",
        "Models/Meshtint Free Knight/Animations/Meshtint Free Knight@Sword And Shield Slash.fbx",
        "Models/Meshtint Free Knight/Animations/Meshtint Free Knight@Sword And Shield Impact.fbx",
        "Models/Meshtint Free Knight/Animations/Meshtint Free Knight@Sword And Shield Death.fbx" };
    ResourceManager::LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths);

    mesh_path = "Models/Meshtint Free Knight/Meshes/Shield.fbx";
    anim_paths.clear();
    execptProcessing_file_nodes = { "PreRotation", "RootNode" };
    ResourceManager::LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths, &execptProcessing_file_nodes);

    mesh_path = "Models/Meshtint Free Knight/Meshes/Sword.fbx";
    anim_paths.clear();
    execptProcessing_file_nodes = { "PreRotation", "RootNode" };
    ResourceManager::LoadSkinnedModelData(device, commandList, model_loader, mesh_path, anim_paths, &execptProcessing_file_nodes);
}

std::vector<UINT8> ResourceManager::GenerateTexture_chechboardPattern()
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

void ResourceManager::LoadTextures(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DXGI_FORMAT BackBufferFormat)
{
    std::vector<std::string> texture_filepaths =
    {
        "UI/Layout/background_test.tga",
        "UI/Layout/Background_SubTitle.png",
        "UI/Layout/Background_SkullPattern.png",
        "UI/Layout/id_input_box_layout_test.tga",
        "UI/Layout/password_input_box_layout_test.tga",
        "UI/Layout/LeftButton.png",
        "UI/Layout/RightButton.png",
        "UI/Layout/White_Transparency50.png",
        "UI/Layout/LightGreen_Transparency50.png",
        "UI/Effect/SwordSlash_a.png",
        "UI/Effect/CrossTarget.png",
        "Models/Environment/Materials/TextureWorld.png",
        "Models/Meshtint Free Knight/Materials/Meshtint Free Knight.tga"
    };

    for (auto& texture_path : texture_filepaths)
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
        std::vector<UINT8> texture = ResourceManager::GenerateTexture_chechboardPattern();

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = &texture[0];
        textureData.RowPitch = 256 * 4;
        textureData.SlicePitch = textureData.RowPitch * 256;

        UpdateSubresources(commandList, tex->Resource.Get(), tex->UploadHeap.Get(), 0, 0, 1, &textureData);
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(tex->Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    }
}

void ResourceManager::BuildMaterials(int& matCB_index, int& diffuseSrvHeap_index)
{
    for (auto& tex_iter : m_Textures)
    {
        auto& tex = tex_iter.second;

        auto mat = std::make_unique<Material>();
        mat->Name = tex->Name;
        mat->NumFramesDirty = gNumFrameResources;
        mat->MatCBIndex = matCB_index++;
        mat->DiffuseSrvHeapIndex = diffuseSrvHeap_index++;
        mat->DiffuseAlbedo = XMFLOAT4(0.88f, 0.88f, 0.88f, 1.0f);
        mat->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
        mat->Roughness = 0.8f;

        m_Materials[mat->Name] = std::move(mat);
    }

    m_nMatCB = (UINT)m_Materials.size();
}

void ResourceManager::BuildRenderItem(std::unordered_map<std::string, std::unique_ptr<RenderItem>>& GenDestList, MeshGeometry* Geo)
{
    for (auto& subMesh_iter : Geo->DrawArgs)
    {
        auto& subMesh = subMesh_iter.second;
        auto Ritem = std::make_unique<RenderItem>();
        Ritem->Name = subMesh_iter.first;
        Ritem->Geo = Geo;
        Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        Ritem->IndexCount = subMesh.IndexCount;
        Ritem->StartIndexLocation = subMesh.StartIndexLocation;
        Ritem->BaseVertexLocation = subMesh.BaseVertexLocation;
        GenDestList[subMesh_iter.first] = std::move(Ritem);
    }
}

void ResourceManager::BuildRenderItems()
{
    // Build LoginScene RenderItem
    {
        auto& SceneRitems = m_AllRitems[(int)RenderTargetScene::LoginScene];

        ResourceManager::BuildRenderItem(SceneRitems, m_Geometries["LoginSceneUIGeo"].get());
        for (auto& subMesh_iter : m_Geometries["LoginSceneUIGeo"]->DrawArgs)
        {
            std::string subMeshName = subMesh_iter.first;
            if (subMeshName == "UI_Layout_LoginSceneBackground")
                SceneRitems[subMeshName]->Mat = m_Materials["background_test"].get();
            else if (subMeshName.find("IdBox") != std::string::npos)
                SceneRitems[subMeshName]->Mat = m_Materials["id_input_box_layout_test"].get();
            else if (subMeshName.find("PasswordBox") != std::string::npos)
                SceneRitems[subMeshName]->Mat = m_Materials["password_input_box_layout_test"].get();
        }
    }

    // Build LobyScene RenderItem
    {
        auto& SceneRitems = m_AllRitems[(int)RenderTargetScene::LobyScene];

        ResourceManager::BuildRenderItem(SceneRitems, m_Geometries["LobySceneUIGeo"].get());
        for (auto& subMesh_iter : m_Geometries["LobySceneUIGeo"]->DrawArgs)
        {
            std::string subMeshName = subMesh_iter.first;
            if (subMeshName == "UI_Layout_LobySceneBackground")
                SceneRitems[subMeshName]->Mat = m_Materials["Background_SkullPattern"].get();
            else if (subMeshName.find("LeftButton") != std::string::npos)
                SceneRitems[subMeshName]->Mat = m_Materials["LeftButton"].get();
            else if (subMeshName.find("RightButton") != std::string::npos)
                SceneRitems[subMeshName]->Mat = m_Materials["RightButton"].get();
            else
                SceneRitems[subMeshName]->Mat = m_Materials["White_Transparency50"].get();
        }
    }

    // Build PlayGameScene RenderItem
    {
        auto& SceneRitems = m_AllRitems[(int)RenderTargetScene::PlayGameScene];

        ResourceManager::BuildRenderItem(SceneRitems, m_Geometries["SpawnStageGround"].get());
        for (auto& subMesh : m_Geometries["SpawnStageGround"]->DrawArgs)
            SceneRitems[subMesh.first]->Mat = m_Materials["checkerboard"].get();

        ResourceManager::BuildRenderItem(SceneRitems, m_Geometries["Environment"].get());
        for (auto& subMesh : m_Geometries["Environment"]->DrawArgs)
            SceneRitems[subMesh.first]->Mat = m_Materials["TextureWorld"].get();

        ResourceManager::BuildRenderItem(SceneRitems, m_Geometries["Shield"].get());
        for (auto& subMesh : m_Geometries["Shield"]->DrawArgs)
            SceneRitems[subMesh.first]->Mat = m_Materials["Meshtint Free Knight"].get();

        ResourceManager::BuildRenderItem(SceneRitems, m_Geometries["Sword"].get());
        for (auto& subMesh : m_Geometries["Sword"]->DrawArgs)
            SceneRitems[subMesh.first]->Mat = m_Materials["Meshtint Free Knight"].get();

        ResourceManager::BuildRenderItem(SceneRitems, m_Geometries["PlayGameSceneUIGeo"].get());
        for (auto& subMesh_iter : m_Geometries["PlayGameSceneUIGeo"]->DrawArgs)
        {
            std::string subMeshName = subMesh_iter.first;
            if (subMeshName.find("Skill") != std::string::npos)
            {
                if (subMeshName.find("List") != std::string::npos)
                    SceneRitems[subMeshName]->Mat = m_Materials["White_Transparency50"].get();
                else
                    SceneRitems[subMeshName]->Mat = m_Materials["LightGreen_Transparency50"].get();
            }
            else SceneRitems[subMeshName]->Mat = m_Materials["White_Transparency50"].get();
        }

        ResourceManager::BuildRenderItem(SceneRitems, m_Geometries["GameEffectGeo"].get());
        for (auto& subMesh_iter : m_Geometries["GameEffectGeo"]->DrawArgs)
        {
            std::string subMeshName = subMesh_iter.first;
            if (subMeshName == "SkillEffect_SwordSlash_a")
                SceneRitems[subMeshName]->Mat = m_Materials["SwordSlash_a"].get();
            else if (subMeshName == "PickingEffect_CrossTarget")
                SceneRitems[subMeshName]->Mat = m_Materials["CrossTarget"].get();
        }
    }

    // Build GameOverScene RenderItem
    {
        auto& SceneRitems = m_AllRitems[(int)RenderTargetScene::GameOverScene];

        ResourceManager::BuildRenderItem(SceneRitems, m_Geometries["GameOverSceneUIGeo"].get());
        for (auto& subMesh_iter : m_Geometries["GameOverSceneUIGeo"]->DrawArgs)
        {
            std::string subMeshName = subMesh_iter.first;
            if (subMeshName.find("Background") != std::string::npos)
                SceneRitems[subMeshName]->Mat = m_Materials["Background_SubTitle"].get();
            else
                SceneRitems[subMeshName]->Mat = m_Materials["White_Transparency50"].get();
        }
    }

    // Build Character RenderItem
    // 캐릭터 렌더아이템에 대해선 각 Scene별로 중복을 허용한다.
    // (Scene별로 렌더아이템 리스트 가용성을 위해.)
    {
        auto& LobySceneRitems     = m_AllRitems[(int)RenderTargetScene::LobyScene];
        auto& PlayGameSceneRitems = m_AllRitems[(int)RenderTargetScene::PlayGameScene];
        auto& GameOverSceneRitems = m_AllRitems[(int)RenderTargetScene::GameOverScene];

        ResourceManager::BuildRenderItem(LobySceneRitems,     m_Geometries["Meshtint Free Knight"].get());
        ResourceManager::BuildRenderItem(PlayGameSceneRitems, m_Geometries["Meshtint Free Knight"].get());
        ResourceManager::BuildRenderItem(GameOverSceneRitems, m_Geometries["Meshtint Free Knight"].get());
        for (auto& subMesh : m_Geometries["Meshtint Free Knight"]->DrawArgs)
        {
            LobySceneRitems[subMesh.first]->Mat     = m_Materials["Meshtint Free Knight"].get();
            PlayGameSceneRitems[subMesh.first]->Mat = m_Materials["Meshtint Free Knight"].get();
            GameOverSceneRitems[subMesh.first]->Mat = m_Materials["Meshtint Free Knight"].get();
        }
    }
}
