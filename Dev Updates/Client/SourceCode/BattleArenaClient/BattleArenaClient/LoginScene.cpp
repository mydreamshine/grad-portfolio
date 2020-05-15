#include "stdafx.h"
#include "LoginScene.h"

LoginScene::~LoginScene()
{
}

void LoginScene::OnInit(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
    int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    Scene::OnInit(device, commandList,
        objCB_index, skinnedCB_index, textBatch_index);
}

void LoginScene::OnInitProperties(CTimer& gt)
{
    
}

void LoginScene::OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
    const bool key_state[], const POINT& oldCursorPos,
    const RECT& ClientRect,
    CTimer& gt)
{
    Scene::OnUpdate(frame_resource, shadow_map, key_state, oldCursorPos, ClientRect, gt);
}

void LoginScene::BuildObjects(int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    if (m_AllRitemsRef == nullptr || m_GeometriesRef == nullptr) return;

    auto& AllRitems = *m_AllRitemsRef;
    auto& Geometries = *m_GeometriesRef;

    ObjectManager objManager;
    const UINT maxUIObject = (UINT)Geometries["LoginSceneUIGeo"]->DrawArgs.size();
    for (auto& Ritem_iter : AllRitems)
    {
        auto Ritem = Ritem_iter.second.get();
        if (Ritem_iter.first.find("UI") != std::string::npos)
        {
            auto newObj = objManager.CreateUIObject(objCB_index++, m_AllObjects, m_UIObjects, maxUIObject);
            m_ObjRenderLayer[(int)RenderLayer::UILayout_Background].push_back(newObj);

            std::string objName = Ritem_iter.first;
            objManager.SetObjectComponent(newObj, objName, Ritem);
        }
    }
    m_nObjCB = (UINT)m_UIObjects.size();
    m_nSKinnedCB = 0;
    for (auto& ui_obj : m_UIObjects) m_nTextBatch += (UINT)ui_obj->m_UIinfos.size();
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

void LoginScene::ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt)
{
}
