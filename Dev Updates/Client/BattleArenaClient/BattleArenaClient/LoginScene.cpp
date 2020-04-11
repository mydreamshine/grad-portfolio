#include "stdafx.h"
#include "LoginScene.h"

LoginScene::~LoginScene()
{
}

void LoginScene::OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
    DXGI_FORMAT BackBufferFormat,
    int& matCB_index, int& diffuseSrvHeap_Index, int& objCB_index, int& skinnedCB_index)
{
    Scene::OnInit(device, commandList,
        BackBufferFormat,
        matCB_index, diffuseSrvHeap_Index, objCB_index, skinnedCB_index);
}

void LoginScene::OnInitProperties()
{
    
}

void LoginScene::OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map, CTimer& gt)
{
    Scene::OnUpdate(frame_resource, shadow_map, gt);
}

void LoginScene::DisposeUploaders()
{
    Scene::DisposeUploaders();
}

void LoginScene::BuildShapeGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	GeometryGenerator geoGen;

    float wnd_width = (float)m_width;
    float wnd_height = (float)m_height;
    float wnd_top = wnd_height / 2;
    float wnd_bottom = -wnd_top;
    float wnd_right = wnd_width / 2;
    float wnd_left = -wnd_right;

    // UI Position relative to DC(Device Coordinate) with origin at screen center.
    std::unordered_map<std::string, GeometryGenerator::MeshData> Meshs;
    Meshs["UI_background"] = geoGen.CreateQuad(wnd_left, wnd_top, wnd_width, wnd_height, 0.99f);
    Meshs["UI_idBox"] = geoGen.CreateQuad(-100.0f, -100, 200.0f, 75.0f, 0.0f);
    Meshs["UI_passwordBox"] = geoGen.CreateQuad(-100.0f, -205.0f, 200.0f, 75.0f, 0.0f);

    std::vector<DXTexturedVertex> total_vertices;
    std::vector<std::uint16_t> total_indices;

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "LoginSceneUIGeo";
    UINT VertexOffset = 0;
    UINT IndexOffset = 0;
    UINT k = 0;
    for (auto& mesh_iter : Meshs)
    {
        auto& meshData = mesh_iter.second;

        total_vertices.insert(total_vertices.end(), meshData.Vertices.size(), DXTexturedVertex());
        for (int i = 0; i < meshData.Vertices.size(); ++i, ++k)
        {
            total_vertices[k].xmf3Position = meshData.Vertices[i].Position;
            total_vertices[k].xmf3Normal = meshData.Vertices[i].Normal;
            total_vertices[k].xmf2TextureUV = meshData.Vertices[i].TexC;
            total_vertices[k].xmf3Tangent = meshData.Vertices[i].TangentU;
        }
        total_indices.insert(total_indices.end(), std::begin(meshData.GetIndices16()), std::end(meshData.GetIndices16()));


        SubmeshGeometry Submesh;
        Submesh.IndexCount = (UINT)meshData.Indices32.size();
        Submesh.StartIndexLocation = IndexOffset;
        Submesh.BaseVertexLocation = VertexOffset;

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

    m_Geometries[geo->Name] = std::move(geo);
}

void LoginScene::LoadTextures(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DXGI_FORMAT BackBufferFormat)
{
    std::vector<std::string> texture_filepaths =
    {
        "UI/background_test.tga",
        "UI/id_input_box_layout_test.tga",
        "UI/password_input_box_layout_test.tga"
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
}

void LoginScene::BuildMaterials(int& matCB_index, int& diffuseSrvHeap_index)
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
}

void LoginScene::BuildRenderItems(int& objCB_index, int& skinnedCB_index)
{
    MeshGeometry* Geo = m_Geometries["LoginSceneUIGeo"].get();
    for (auto& subMesh : Geo->DrawArgs)
    {
        auto Ritem = std::make_unique<RenderItem>();
        Ritem->NumFramesDirty = gNumFrameResources;
        Ritem->ObjCBIndex = objCB_index++;
        Ritem->Geo = m_Geometries["LoginSceneUIGeo"].get();
        Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        Ritem->IndexCount = subMesh.second.IndexCount;
        Ritem->StartIndexLocation = subMesh.second.StartIndexLocation;
        Ritem->BaseVertexLocation = subMesh.second.BaseVertexLocation;

        if (subMesh.first == "UI_background")
            Ritem->Mat = m_Materials["background_test"].get();
        else if (subMesh.first == "UI_idBox")
            Ritem->Mat = m_Materials["id_input_box_layout_test"].get();
        else if (subMesh.first == "UI_passwordBox")
            Ritem->Mat = m_Materials["password_input_box_layout_test"].get();

        m_RitemLayer[(int)RenderLayer::UIOpaque].push_back(Ritem.get());
        m_AllRitems.push_back(std::move(Ritem));
    }
}

void LoginScene::UpdateObjectCBs(UploadBuffer<ObjectConstants>* objCB, CTimer& gt)
{
    Scene::UpdateObjectCBs(objCB, gt);
}

void LoginScene::UpdateSkinnedCBs(UploadBuffer<SkinnedConstants>* skinnedCB, CTimer& gt)
{
    Scene::UpdateSkinnedCBs(skinnedCB, gt);
}

void LoginScene::UpdateMaterialCBs(UploadBuffer<MaterialConstants>* matCB, CTimer& gt)
{
    Scene::UpdateMaterialCBs(matCB, gt);
}

void LoginScene::UpdateMainPassCB(UploadBuffer<PassConstants>* passCB, CTimer& gt)
{
    XMFLOAT3 EyePosition = { 0.0f, 0.0f, -1.0f };
    XMFLOAT3 LookAtPosition = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 UpDirection = { 0.0f, 1.0f, 0.0f };

    m_MainCamera.SetPosition(EyePosition);
    m_MainCamera.SetOrthographicLens(m_width, m_height, 1.0f, 1000.0f);
    m_MainCamera.LookAt(EyePosition, LookAtPosition, UpDirection);
    m_MainCamera.UpdateViewMatrix();

    Scene::UpdateMainPassCB(passCB, gt);
}

void LoginScene::UpdateShadowPassCB(UploadBuffer<PassConstants>* passCB, ShadowMap* shadow_map, CTimer& gt)
{
    Scene::UpdateShadowPassCB(passCB, shadow_map, gt);
}

void LoginScene::UpdateShadowTransform(CTimer& gt)
{
    Scene::UpdateShadowTransform(gt);
}
