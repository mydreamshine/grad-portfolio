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
    int& matCB_index, int& diffuseSrvHeap_Index, int& objCB_index, int& skinnedCB_index)
{
    BuildShapeGeometry(device, commandList);
    LoadSkinnedModels(device, commandList);
    LoadTextures(device, commandList, BackBufferFormat);
    BuildMaterials(matCB_index, diffuseSrvHeap_Index);
    BuildRenderItems(objCB_index, skinnedCB_index);

    // Estimate the scene bounding sphere manually since we know how the scene was constructed.
    // The grid is the "widest object" with a width of 1 and depth of 1.0f, and centered at
    // the world space origin.  In general, you need to loop over every world space vertex
    // position and compute the bounding sphere.
    m_SceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_SceneBounds.Radius = sqrtf(0.5f * 0.5f + 0.5f * 0.5f);
}


void Scene::OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map, CTimer& gt)
{
    AnimateLights(gt);
    AnimateSkeletons(gt);
    AnimateCameras(gt);

    ProcessInput(gt);

    UpdateObjectCBs(frame_resource->ObjectCB.get(), gt);
    UpdateSkinnedCBs(frame_resource->SkinnedCB.get(), gt);
    UpdateMaterialCBs(frame_resource->MaterialCB.get(), gt);
    UpdateShadowTransform(gt);
    UpdateMainPassCB(frame_resource->PassCB.get(), gt);
    UpdateShadowPassCB(frame_resource->PassCB.get(), shadow_map, gt);
}

void Scene::DisposeUploaders()
{
    for (auto& e : m_AllRitems)
    {
        if (e->Geo != nullptr)
            e->Geo->DisposeUploaders();
    }

    for (auto& tex_iter : m_Textures)
        tex_iter.second->UploadHeap = nullptr;
}

void Scene::LoadSkinnedModelData(
    ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
    ModelLoader& model_loader,
    const std::string& mesh_filepath, const std::vector<std::string>& anim_filepaths)
{
    model_loader.loadMeshAndSkeleton(mesh_filepath);
    for (auto& anim_path : anim_filepaths)
        model_loader.loadAnimation(anim_path);

    std::string ModelName, Anim0_Name;
    getFileName(mesh_filepath.c_str(), ModelName);
    getFileName(anim_filepaths[0].c_str(), Anim0_Name);

    // aiSkeleton data move
    if (model_loader.mSkeleton != nullptr)
    {
        std::string skeletonName = model_loader.mSkeleton->mName;
        auto animSk = std::move(model_loader.mSkeleton);
        animSk->AnimPlay(Anim0_Name);
        animSk->AnimLoop(Anim0_Name, true);
        animSk->SkinnedCBIndex = (int)m_ModelSkeltons.size();
        m_ModelSkeltons[skeletonName] = std::move(animSk);
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

void Scene::UpdateObjectCBs(UploadBuffer<ObjectConstants>* objCB, CTimer& gt)
{
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

            objCB->CopyData(e->ObjCBIndex, objConstants);

            // Next FrameResource need to be updated too.
            e->NumFramesDirty--;
        }
    }
}

void Scene::aiM2dxM(XMFLOAT4X4& dest, aiMatrix4x4& source)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
            dest.m[i][j] = source[i][j];
    }
}

void Scene::UpdateSkinnedCBs(UploadBuffer<SkinnedConstants>* skinnedCB, CTimer& gt)
{
    for (auto& skleton_iter : m_ModelSkeltons)
    {
        auto skleton = skleton_iter.second.get();
        std::string animName = skleton->mCurrPlayingAnimName;
        if (skleton->AnimIsPlaying(animName))
        {
            SkinnedConstants SkinnedInfo;
            for (size_t i = 0; i < skleton->mCurrAnimTransforms.size(); ++i)
                aiM2dxM(SkinnedInfo.BoneTransform[i], skleton->mCurrAnimTransforms[i]);

            skinnedCB->CopyData(skleton->SkinnedCBIndex, SkinnedInfo);
        }
    }
}

void Scene::UpdateMaterialCBs(UploadBuffer<MaterialConstants>* matCB, CTimer& gt)
{
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
