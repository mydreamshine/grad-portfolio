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
    int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
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
    UpdateTextInfo(gt);

    ProcessInput(key_state, oldCursorPos, gt);
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
    if (m_MaterialsRef == nullptr) return;

    auto& Materials = *m_MaterialsRef;

    for (auto& mat_iter : Materials)
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
