#include "stdafx.h"
#include "Scene.h"

Scene::Scene(UINT width, UINT height)
{
    m_width = width; m_height = height;
}

Scene::~Scene()
{
}

void Scene::OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
    DXGI_FORMAT BackBufferFormat,
    int& matCB_index, int& diffuseSrvHeap_Index,
    int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    // 아래 메소드 순서는 반드시 지켜져야 한다.
    BuildShapeGeometry(device, commandList);
    LoadSkinnedModels(device, commandList);
    LoadTextures(device, commandList, BackBufferFormat);
    BuildMaterials(matCB_index, diffuseSrvHeap_Index);
    BuildRenderItems();
    BuildObjects(objCB_index, skinnedCB_index, textBatch_index);
}


void Scene::OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
    const bool key_state[], const POINT& oldCursorPos,
    const RECT& ClientRect,
    CTimer& gt)
{
    m_ClientRect = ClientRect;

    AnimateLights(gt);
    AnimateSkeletons(gt);
    AnimateCameras(gt);

    UpdateObjectCBs(frame_resource->ObjectCB.get(), gt);
    UpdateSkinnedCBs(frame_resource->SkinnedCB.get(), gt);
    UpdateMaterialCBs(frame_resource->MaterialCB.get(), gt);
    UpdateShadowTransform(gt);
    UpdateMainPassCB(frame_resource->PassCB.get(), gt);
    UpdateShadowPassCB(frame_resource->PassCB.get(), shadow_map, gt);

    ProcessInput(key_state, oldCursorPos, gt);
}

void Scene::DisposeUploaders()
{
    for (auto& Geo_iter : m_Geometries)
        Geo_iter.second->DisposeUploaders();

    for (auto& tex_iter : m_Textures)
        tex_iter.second->UploadHeap = nullptr;
}

std::unique_ptr<MeshGeometry> Scene::BuildMeshGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
    const std::string& geoName, std::unordered_map<std::string, GeometryGenerator::MeshData>& Meshes)
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

void Scene::LoadSkinnedModelData(
    ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
    ModelLoader& model_loader,
    const std::string& mesh_filepath, const std::vector<std::string>& anim_filepaths,
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

void Scene::BuildRenderItem(std::unordered_map<std::string, std::unique_ptr<RenderItem>>& GenDestList, MeshGeometry* Geo)
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

void Scene::UpdateObjectCBs(UploadBuffer<ObjectConstants>* objCB, CTimer& gt)
{
    for (auto& obj : m_WorldObjects)
    {
        auto objInfo = obj->m_TransformInfo.get();
        if (objInfo->NumObjectCBDirty > 0)
        {
            XMFLOAT4X4 WorldTransform, LocalTransform;
            WorldTransform = objInfo->GetWorldTransform();
            LocalTransform = objInfo->GetLocalTransform();

            XMMATRIX WorldM = XMLoadFloat4x4(&WorldTransform);
            XMMATRIX LocalM = XMLoadFloat4x4(&LocalTransform);
            XMMATRIX TexM = XMLoadFloat4x4(&objInfo->m_TexTransform);

            ObjectConstants objConstants;
            XMStoreFloat4x4(&objConstants.Local, XMMatrixTranspose(LocalM));
            XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(WorldM));
            XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(TexM));
            objConstants.TexAlpha = objInfo->m_TexAlpha;

            objCB->CopyData(objInfo->ObjCBIndex, objConstants);

            // Next FrameResource need to be updated too.
            objInfo->NumObjectCBDirty--;
        }
    }

    for (auto& obj : m_UIObjects)
    {
        auto objInfo = obj->m_TransformInfo.get();
        if (objInfo->NumObjectCBDirty > 0)
        {
            XMFLOAT4X4 WorldTransform, LocalTransform;
            WorldTransform = objInfo->GetWorldTransform();
            LocalTransform = objInfo->GetLocalTransform();

            XMMATRIX WorldM = XMLoadFloat4x4(&WorldTransform);
            XMMATRIX LocalM = XMLoadFloat4x4(&LocalTransform);
            XMMATRIX TexM = XMLoadFloat4x4(&objInfo->m_TexTransform);

            ObjectConstants objConstants;
            XMStoreFloat4x4(&objConstants.Local, XMMatrixTranspose(LocalM));
            XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(WorldM));
            XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(TexM));
            objConstants.TexAlpha = objInfo->m_TexAlpha;

            objCB->CopyData(objInfo->ObjCBIndex, objConstants);

            // Next FrameResource need to be updated too.
            objInfo->NumObjectCBDirty--;
        }
    }
}

void Scene::aiM2dxM(XMFLOAT4X4& dest, const aiMatrix4x4& source)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
            dest.m[i][j] = source[i][j];
    }
}

void Scene::UpdateSkinnedCBs(UploadBuffer<SkinnedConstants>* skinnedCB, CTimer& gt)
{
    for (auto& obj : m_CharacterObjects)
    {
        auto SkeletonInfo = obj->m_SkeletonInfo.get();

        std::string& AnimName = SkeletonInfo->m_AnimInfo->CurrPlayingAnimName;
        bool isSetted = false;

        if (SkeletonInfo->m_AnimInfo->AnimIsPlaying(AnimName, isSetted))
        {
            if (isSetted == false) continue;

            SkinnedConstants skinConstants;
            auto& animTransforms = SkeletonInfo->m_AnimInfo->CurrAnimJointTransforms;
            auto& offsetTransforms = SkeletonInfo->m_AnimInfo->OffsetJointTransforms;

            for (size_t i = 0; i < animTransforms.size(); ++i)
                aiM2dxM(skinConstants.BoneTransform[i], animTransforms[i]* offsetTransforms[i]);

            skinnedCB->CopyData(SkeletonInfo->SkinCBIndex, skinConstants);
        }
    }
}

void Scene::UpdateMaterialCBs(UploadBuffer<MaterialConstants>* matCB, CTimer& gt)
{
    for (auto& mat_iter : m_Materials)
    {
        // Only update the cbuffer data if the constants have changed.  If the cbuffer
        // data changes, it needs to be updated for each FrameResource.
        Material* mat = mat_iter.second.get();
        if (mat->NumFramesDirty > 0)
        {
            XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

            MaterialConstants matConstants;
            matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
            matConstants.FresnelR0 = mat->FresnelR0;
            matConstants.Roughness = mat->Roughness;
            XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

            matCB->CopyData(mat->MatCBIndex, matConstants);

            // Next FrameResource need to be updated too.
            mat->NumFramesDirty--;
        }
    }
}

void Scene::UpdateMainPassCB(UploadBuffer<PassConstants>* passCB, CTimer& gt)
{
    XMMATRIX view = m_MainCamera.GetView();
    XMMATRIX proj = m_MainCamera.GetProj();

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
    m_MainPassCB.EyePosW = m_MainCamera.GetPosition3f();
    m_MainPassCB.RenderTargetSize = XMFLOAT2((float)m_width, (float)m_height);
    m_MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / m_width, 1.0f / m_height);
    m_MainPassCB.NearZ = 1.0f;
    m_MainPassCB.FarZ = 10000.0f;
    m_MainPassCB.TotalTime = gt.GetTotalTime();
    m_MainPassCB.DeltaTime = gt.GetTimeElapsed();
    m_MainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
    m_MainPassCB.Lights[0].Direction = m_RotatedLightDirections[0];
    m_MainPassCB.Lights[0].Strength = { 0.9f, 0.9f, 0.7f };
    m_MainPassCB.Lights[1].Direction = m_RotatedLightDirections[1];
    m_MainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
    m_MainPassCB.Lights[2].Direction = m_RotatedLightDirections[2];
    m_MainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

    // passCB index 0: MainRenderPassCB
    passCB->CopyData(0, m_MainPassCB);
}

void Scene::UpdateShadowPassCB(UploadBuffer<PassConstants>* passCB, ShadowMap* shadow_map, CTimer& gt)
{
    XMMATRIX view = XMLoadFloat4x4(&m_LightView);
    XMMATRIX proj = XMLoadFloat4x4(&m_LightProj);

    XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
    XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
    XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

    UINT w = shadow_map->Width();
    UINT h = shadow_map->Height();

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

    // passCB index 1: ShadowRenderPassCB
    passCB->CopyData(1, m_ShadowPassCB);
}

void Scene::UpdateShadowTransform(CTimer& gt)
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
