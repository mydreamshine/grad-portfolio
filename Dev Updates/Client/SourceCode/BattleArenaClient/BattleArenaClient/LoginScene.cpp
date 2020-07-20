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
    OnceSendChatLog = false;
    IDText.clear();
    PasswordText.clear();
}

void LoginScene::OnUpdate(FrameResource* frame_resource, ShadowMap* shadow_map,
    const bool key_state[], const POINT& oldCursorPos,
    const RECT& ClientRect,
    CTimer& gt,
    std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    Scene::OnUpdate(frame_resource, shadow_map, key_state, oldCursorPos, ClientRect, gt, GeneratedEvents);
}

void LoginScene::BuildObjects(int& objCB_index, int& skinnedCB_index, int& textBatch_index)
{
    if (m_AllRitemsRef == nullptr || m_GeometriesRef == nullptr) return;

    auto& AllRitems = *m_AllRitemsRef;
    auto& Geometries = *m_GeometriesRef;

    ObjectManager objManager;
    const UINT maxUILayOutObject = (UINT)Geometries["LoginSceneUIGeo"]->DrawArgs.size();
    for (auto& Ritem_iter : AllRitems)
    {
        auto& Ritem_name = Ritem_iter.first;
        auto Ritem = Ritem_iter.second.get();
        if (Ritem_name.find("UI") != std::string::npos)
        {
            auto newObj = objManager.CreateUILayOutObject(objCB_index++, m_AllObjects, m_UILayOutObjects, maxUILayOutObject);
            m_ObjRenderLayer[(int)RenderLayer::UILayout_Background].push_back(newObj);

            std::string objName = Ritem_iter.first;
            std::vector<RenderItem*> Ritems = { Ritem };
            objManager.SetObjectComponent(newObj, objName, Ritems);

            if (objName.find("Id") != std::string::npos || objName.find("Password") != std::string::npos)
            {
                auto newLayoutTextObj = objManager.CreateTextObject(textBatch_index++, m_AllObjects, m_TextObjects, m_MaxTextObject);
                auto newEditTextObj = objManager.CreateTextObject(textBatch_index++, m_AllObjects, m_TextObjects, m_MaxTextObject);

                if (objName.find("Id") != std::string::npos)
                {
                    newLayoutTextObj->m_Name = "LayOutText_ID";
                    newEditTextObj->m_Name = "EditText_ID";
                }
                else if (objName.find("Password") != std::string::npos)
                {
                    newLayoutTextObj->m_Name = "LayOutText_Password";
                    newEditTextObj->m_Name = "EditText_Password";
                }

                newLayoutTextObj->m_Textinfo->m_FontName = L"¸¼Àº °íµñ";
                newEditTextObj->m_Textinfo->m_FontName = L"¸¼Àº °íµñ";
                newLayoutTextObj->m_Textinfo->m_TextColor = DirectX::Colors::Blue;
                newEditTextObj->m_Textinfo->m_TextColor = DirectX::Colors::Blue;

                auto UI_LayoutPos = Ritem->Geo->DrawArgs[objName].Bounds.Center;
                float UI_LayoutWidth = Ritem->Geo->DrawArgs[objName].Bounds.Extents.x * 2.0f;
                newEditTextObj->m_Textinfo->m_TextPos.x = UI_LayoutPos.x;
                newEditTextObj->m_Textinfo->m_TextPos.y = UI_LayoutPos.y;
                newLayoutTextObj->m_Textinfo->m_TextPos.x = UI_LayoutPos.x - UI_LayoutWidth * 0.2f;
                newLayoutTextObj->m_Textinfo->m_TextPos.y = UI_LayoutPos.y;
            }
        }
    }

    m_nObjCB = (UINT)m_UILayOutObjects.size();
    m_nSKinnedCB = 0;
    m_nTextBatch = (UINT)m_TextObjects.size();
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

void LoginScene::ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{

    // Process Input Box (ID, Password)
    {

    }

    if (OnceSendLogin == false) {
        if (true == key_state[VK_RETURN])
        {
            std::wstring id{ L"test001" };
            std::wstring pw{ L"0000" };
            EventManager eventManager;
            eventManager.ReservateEvent_TryLogin(GeneratedEvents, id, pw);
            OnceSendLogin = true;
        }
    }

    if (OnceSendChatLog == true)
    {
        EventManager eventManager;
        eventManager.ReservateEvent_TryLogin(GeneratedEvents, IDText, PasswordText);
        IDText.clear();
        PasswordText.clear();
        OnceSendChatLog = false;
    }
}
