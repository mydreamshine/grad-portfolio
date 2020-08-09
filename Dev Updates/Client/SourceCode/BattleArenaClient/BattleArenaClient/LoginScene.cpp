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
    ObjectManager ObjManager;

    auto& AllRitems = *m_AllRitemsRef;

    Object* BackgroundLayerT1 = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_LoginSceneBackground_T1");
    Object* BackgroundLayerT2 = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_LoginSceneBackground_T2");
    BackgroundLayerT1->m_TransformInfo->m_TexAlpha = 1.0f;
    BackgroundLayerT2->m_TransformInfo->m_TexAlpha = 0.0f;
    BackgroundLayerT1->m_RenderItems = { AllRitems["UI_Layout_LoginSceneBackground_1"].get() };
    BackgroundLayerT2->m_RenderItems = { AllRitems["UI_Layout_LoginSceneBackground_2"].get() };


    // Init Texts
    {
        Object* ID_RenderTextObject = ObjManager.FindObjectName(m_TextObjects, "TextIDRenderObj");
        Object* Password_RenderTextObject = ObjManager.FindObjectName(m_TextObjects, "TextPasswordRenderObj");
        Object* ID_InputCaretObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_ID_InputBoxCaret");
        Object* Password_InputCaretObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_Password_InputBoxCaret");

        ID_RenderTextObject->m_Textinfo->m_TextColor = XMVectorSet(127.0f / 255.0f, 127.0f / 255.0f, 127.0f / 255.0f, 1.0f);
        Password_RenderTextObject->m_Textinfo->m_TextColor = XMVectorSet(127.0f / 255.0f, 127.0f / 255.0f, 127.0f / 255.0f, 1.0f);
        ID_RenderTextObject->m_Textinfo->m_Text = L"ID";
        Password_RenderTextObject->m_Textinfo->m_Text = L"Password";

        inputTextBox_ID.Init();
        inputTextBox_ID.SetActivate(false); // Deactivate Caret
        inputTextBox_ID.SetTextRenderObj(ID_RenderTextObject);
        inputTextBox_ID.SetCaretRenderObj(ID_InputCaretObject);
        inputTextBox_ID.SetFont((*m_FontsRef)[L"¸¼Àº °íµñ(12pt)"].get());
        inputTextBox_ID.SetMaxSize(InputBoxMaxWidth, 24.0f);
        inputTextBox_ID.SetMaxInputText(32);

        inputTextBox_Password.Init();
        inputTextBox_Password.SetActivate(false); // Deactivate Caret
        inputTextBox_Password.SetTextRenderObj(Password_RenderTextObject);
        inputTextBox_Password.SetCaretRenderObj(Password_InputCaretObject);
        inputTextBox_Password.SetFont((*m_FontsRef)[L"¸¼Àº °íµñ(12pt)"].get());
        inputTextBox_Password.SetMaxSize(InputBoxMaxWidth, 24.0f);
        inputTextBox_Password.SetMaxInputText(32);
        inputTextBox_Password.SetHide(true);
    }

    m_CurrBackgroundLayerIndex = 0;
    m_BackgroundLayers.clear();
    for (auto& Ritem_iter : AllRitems)
    {
        auto& Ritem_name = Ritem_iter.first;
        auto Ritem = Ritem_iter.second.get();
        if (Ritem_name.find("LoginSceneBackground") != std::string::npos)
            m_BackgroundLayers.push_back(Ritem);
    }

    LoginButtonPress = false;
    LoginButtonUp = true;

    ID_InputBoxSelected = false;
    InputBoxSelected = false;
    InputBoxSelectChange = false;

    BackgroundLayeT1IsFront = true;
    BackgroundLayerTransforming = false;

    BackgroundLayerTransformingTimeStack = 0.0f;
    BackgroundLayerTransformIntervalTimeStack = 0.0f;

    BackgroundLayerAlphaIncreaseSign = 1.0f;

    OnceSendLogin = false;
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

    int BacgroundCount = 0;
    XMFLOAT2 ID_InputCaretPos = { 0.0f, 0.0f };
    XMFLOAT2 Password_InputCaretPos = { 0.0f, 0.0f };

    ObjectManager objManager;
    const UINT maxUILayOutObject = (UINT)Geometries["LoginSceneUIGeo"]->DrawArgs.size();

    for (auto& Ritem_iter : AllRitems)
    {
        auto& Ritem_name = Ritem_iter.first;
        auto Ritem = Ritem_iter.second.get();
        if (Ritem_name.find("LoginSceneBackground") != std::string::npos)
            m_BackgroundLayers.push_back(Ritem);
    }

    for (auto& Ritem_iter : AllRitems)
    {
        auto& Ritem_name = Ritem_iter.first;
        auto Ritem = Ritem_iter.second.get();

        if (Ritem_name.find("UI") != std::string::npos)
        {
            if (Ritem_name.find("Press") != std::string::npos) continue;
            if (Ritem_name.find("Background") != std::string::npos && BacgroundCount == 2) continue;
            auto newObj = objManager.CreateUILayOutObject(objCB_index++, m_AllObjects, m_UILayOutObjects, maxUILayOutObject);
            m_ObjRenderLayer[(int)RenderLayer::UILayout_Background].push_back(newObj);

            std::string objName = Ritem_iter.first;
            if (objName.find("Background") != std::string::npos)
            {
                BacgroundCount++;
                objName = "UI_Layout_LoginSceneBackground_T" + std::to_string(BacgroundCount);
            }

            std::vector<RenderItem*> Ritems = { Ritem };
            objManager.SetObjectComponent(newObj, objName, Ritems);

            if (objName.find("Background") != std::string::npos)
            {
                if (BacgroundCount == 2)
                    newObj->m_TransformInfo->m_TexAlpha = 0.0f;
                newObj->m_RenderItems = { m_BackgroundLayers[0] };
            }

            if (objName.find("TitleLayer") != std::string::npos)
            {
                XMFLOAT3 UI_LayoutPos = Ritem->Geo->DrawArgs[objName].Bounds.Center;
                XMFLOAT3 UI_LayoutExtents = Ritem->Geo->DrawArgs[objName].Bounds.Extents;

                int TextObjCreatNum = 2;
                std::vector<Object*> additionalTextObjets;

                Object* newLayoutTextObj = nullptr;
                TextInfo* text_info = nullptr;
                for (int i = 0; i < TextObjCreatNum; ++i)
                {
                    newLayoutTextObj = objManager.CreateTextObject(textBatch_index++, m_AllObjects, m_TextObjects, m_MaxTextObject);
                    text_info = newLayoutTextObj->m_Textinfo.get();
                    newLayoutTextObj->m_Name = "Text" + objName;
                    text_info->m_FontName = L"¸¼Àº °íµñ(12pt)";
                    text_info->m_TextColor = XMVectorSet(127.0f / 255.0f, 127.0f / 255.0f, 127.0f / 255.0f, 1.0f);
                    text_info->m_TextPivot = DXTK_FONT::TEXT_PIVOT::LEFT;
                    text_info->m_TextPos.x = UI_LayoutPos.x - UI_LayoutExtents.x + 82.0f;
                    text_info->m_TextPos.y = UI_LayoutPos.y;
                    text_info->m_TextPos.x += m_width / 2.0f; // offset
                    text_info->m_TextPos.y = m_height / 2.0f - text_info->m_TextPos.y; // offset

                    additionalTextObjets.push_back(newLayoutTextObj);
                }

                additionalTextObjets[0]->m_Name = "TextIDRenderObj";
                additionalTextObjets[0]->m_Textinfo->m_Text = L"ID";
                additionalTextObjets[0]->m_Textinfo->m_TextPos.y = UI_LayoutPos.y - 72.0f;
                additionalTextObjets[0]->m_Textinfo->m_TextPos.y *= -1.0f;          // offset
                additionalTextObjets[0]->m_Textinfo->m_TextPos.y += m_height / 2.0f; // offset
                ID_InputCaretPos = additionalTextObjets[0]->m_Textinfo->m_TextPos;

                additionalTextObjets[1]->m_Name = "TextPasswordRenderObj";
                additionalTextObjets[1]->m_Textinfo->m_Text = L"Password";
                additionalTextObjets[1]->m_Textinfo->m_TextPos.y = UI_LayoutPos.y - 125.0f;
                additionalTextObjets[1]->m_Textinfo->m_TextPos.y *= -1.0f;          // offset
                additionalTextObjets[1]->m_Textinfo->m_TextPos.y += m_height / 2.0f; // offset
                Password_InputCaretPos = additionalTextObjets[1]->m_Textinfo->m_TextPos;
            }
        }
    }

    // Init Texts
    {
        ObjectManager ObjManager;
        Object* ID_RenderTextObject = ObjManager.FindObjectName(m_TextObjects, "TextIDRenderObj");
        Object* Password_RenderTextObject = ObjManager.FindObjectName(m_TextObjects, "TextPasswordRenderObj");
        Object* ID_InputCaretObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_ID_InputBoxCaret");
        Object* Password_InputCaretObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_Password_InputBoxCaret");

        inputTextBox_ID.Init();
        inputTextBox_ID.SetActivate(false); // Deactivate Caret
        inputTextBox_ID.SetTextRenderObj(ID_RenderTextObject);
        inputTextBox_ID.SetCaretRenderObj(ID_InputCaretObject);
        inputTextBox_ID.SetFont((*m_FontsRef)[L"¸¼Àº °íµñ(12pt)"].get());
        inputTextBox_ID.SetMaxSize(InputBoxMaxWidth, 24.0f);
        inputTextBox_ID.SetMaxInputText(32);

        inputTextBox_Password.Init();
        inputTextBox_Password.SetActivate(false); // Deactivate Caret
        inputTextBox_Password.SetTextRenderObj(Password_RenderTextObject);
        inputTextBox_Password.SetCaretRenderObj(Password_InputCaretObject);
        inputTextBox_Password.SetFont((*m_FontsRef)[L"¸¼Àº °íµñ(12pt)"].get());
        inputTextBox_Password.SetMaxSize(InputBoxMaxWidth, 24.0f);
        inputTextBox_Password.SetMaxInputText(32);
        inputTextBox_Password.SetHide(true);
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

void LoginScene::UpdateTextInfo(CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    inputTextBox_ID.Update(gt, (float)m_width, (float)m_height);
    inputTextBox_Password.Update(gt, (float)m_width, (float)m_height);
}

void LoginScene::AnimateCameras(CTimer& gt)
{
    LoginScene::UpdateUITransform(gt);
}

void LoginScene::UpdateUITransform(CTimer& gt)
{
    ObjectManager ObjManager;
    Object* BackgroundLayerT1 = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_LoginSceneBackground_T1");
    Object* BackgroundLayerT2 = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_LoginSceneBackground_T2");
    float BackgroundLayerT1Alpha = BackgroundLayerT1->m_TransformInfo->m_TexAlpha;
    float BackgroundLayerT2Alpha = BackgroundLayerT2->m_TransformInfo->m_TexAlpha;

    if (BackgroundLayerTransforming == false)
    {
        if (BackgroundLayerTransformIntervalTimeStack >= BackgroundLayerTransformInterval)
        {
            if (compareFloat(BackgroundLayerT1Alpha, 0.0f) == true)
            {
                m_CurrBackgroundLayerIndex = (m_CurrBackgroundLayerIndex + 1) % m_BackgroundLayers.size();
                BackgroundLayerT1->m_RenderItems = { m_BackgroundLayers[m_CurrBackgroundLayerIndex] };
            }
            else if (compareFloat(BackgroundLayerT2Alpha, 0.0f) == true)
            {
                m_CurrBackgroundLayerIndex = (m_CurrBackgroundLayerIndex + 1) % m_BackgroundLayers.size();
                BackgroundLayerT2->m_RenderItems = { m_BackgroundLayers[m_CurrBackgroundLayerIndex] };
            }

            BackgroundLayerTransforming = true;
            BackgroundLayerTransformIntervalTimeStack = 0.0f;
            BackgroundLayerAlphaIncreaseSign *= -1.0f;
        }
        else BackgroundLayerTransformIntervalTimeStack += gt.GetTimeElapsed();
    }

    if (BackgroundLayerTransforming == true)
    {
        if (BackgroundLayerTransformingTimeStack >= BackgroundLayerTransformingDoneTime)
        {
            BackgroundLayerTransforming = false;
            BackgroundLayerTransformingTimeStack = 0.0f;
        }
        else
        {
            BackgroundLayerTransformingTimeStack += gt.GetTimeElapsed();

            if (BackgroundLayerTransformingTimeStack > BackgroundLayerTransformingDoneTime)
                BackgroundLayerTransformingTimeStack = BackgroundLayerTransformingDoneTime;

            BackgroundLayerT1Alpha += BackgroundLayerAlphaIncreaseSign * (BackgroundLayerTransformingTimeStack / BackgroundLayerTransformingDoneTime);
            BackgroundLayerT2Alpha += -BackgroundLayerAlphaIncreaseSign * (BackgroundLayerTransformingTimeStack / BackgroundLayerTransformingDoneTime);
            BackgroundLayerT1Alpha = MathHelper::Clamp(BackgroundLayerT1Alpha, 0.0f, 1.0f);
            BackgroundLayerT2Alpha = MathHelper::Clamp(BackgroundLayerT2Alpha, 0.0f, 1.0f);

            BackgroundLayerT1->m_TransformInfo->m_TexAlpha = BackgroundLayerT1Alpha;
            BackgroundLayerT2->m_TransformInfo->m_TexAlpha = BackgroundLayerT2Alpha;
        }
    }
}

void LoginScene::ProcessInput(const bool key_state[], const POINT& oldCursorPos, CTimer& gt, std::queue<std::unique_ptr<EVENT>>& GeneratedEvents)
{
    ObjectManager ObjManager;
    Object* ID_RenderTextObject = ObjManager.FindObjectName(m_TextObjects, "TextIDRenderObj");
    Object* Password_RenderTextObject = ObjManager.FindObjectName(m_TextObjects, "TextPasswordRenderObj");

    // Process Input Box (ID, Password)
    {
        if (key_state[VK_LBUTTON] == true)
        {
            RECT ID_InputBoxLayerAreainScreen =
            {
                133, -57, 262, -86
            };
            RECT Password_InputBoxLayerAreainScreen =
            {
                133, -110, 262, -139
            };

            float CursorPos_x = oldCursorPos.x - m_width * 0.5f;
            float CursorPos_y = -(oldCursorPos.y - m_height * 0.5f);

            if (PointInRect(CursorPos_x, CursorPos_y, ID_InputBoxLayerAreainScreen) == true && ID_InputBoxSelected == false)
            {
                inputTextBox_ID.SetActivate(true);
                inputTextBox_Password.SetActivate(false);

            if (inputTextBox_ID.GetFullinputTexts().size() == 0)
            {
                ID_RenderTextObject->m_Textinfo->m_Text.clear();
                ID_RenderTextObject->m_Textinfo->m_TextColor = XMVectorSet(38.0f / 255.0f, 38.0f / 255.0f, 38.0f / 255.0f, 1.0f);
            }
            
            if (inputTextBox_Password.GetFullinputTexts().size() == 0)
            {
                Password_RenderTextObject->m_Textinfo->m_Text = L"Password";
                Password_RenderTextObject->m_Textinfo->m_TextColor = XMVectorSet(127.0f / 255.0f, 127.0f / 255.0f, 127.0f / 255.0f, 1.0f);
            }
            
            ID_InputBoxSelected = true;
            InputBoxSelected = true;
            }
            else if (PointInRect(CursorPos_x, CursorPos_y, Password_InputBoxLayerAreainScreen) == true && ID_InputBoxSelected == true)
            {
            inputTextBox_Password.SetActivate(true);
            inputTextBox_ID.SetActivate(false);

            if (inputTextBox_Password.GetFullinputTexts().size() == 0)
            {
                Password_RenderTextObject->m_Textinfo->m_Text.clear();
                Password_RenderTextObject->m_Textinfo->m_TextColor = XMVectorSet(38.0f / 255.0f, 38.0f / 255.0f, 38.0f / 255.0f, 1.0f);
            }

            if (inputTextBox_ID.GetFullinputTexts().size() == 0)
            {
                ID_RenderTextObject->m_Textinfo->m_Text = L"ID";
                ID_RenderTextObject->m_Textinfo->m_TextColor = XMVectorSet(127.0f / 255.0f, 127.0f / 255.0f, 127.0f / 255.0f, 1.0f);
            }

            ID_InputBoxSelected = false;
            InputBoxSelected = true;
            }
        }

        if (InputBoxSelected == true)
        {
            if (ID_InputBoxSelected == true)
                inputTextBox_ID.ProcessInput(key_state, oldCursorPos, gt, GeneratedEvents);
            else
                inputTextBox_Password.ProcessInput(key_state, oldCursorPos, gt, GeneratedEvents);
        }

        // Change Selected InputBox
        {
            if (key_state[VK_TAB] == true)
                InputBoxSelectChange = true;
            else
            {
                if (InputBoxSelectChange == true)
                {
                    if (ID_InputBoxSelected == false)
                    {
                        inputTextBox_ID.SetActivate(true);
                        inputTextBox_Password.SetActivate(false);

                        if (inputTextBox_ID.GetFullinputTexts().size() == 0)
                        {
                            ID_RenderTextObject->m_Textinfo->m_Text.clear();
                            ID_RenderTextObject->m_Textinfo->m_TextColor = XMVectorSet(38.0f / 255.0f, 38.0f / 255.0f, 38.0f / 255.0f, 1.0f);
                        }

                        if (inputTextBox_Password.GetFullinputTexts().size() == 0)
                        {
                            Password_RenderTextObject->m_Textinfo->m_Text = L"Password";
                            Password_RenderTextObject->m_Textinfo->m_TextColor = XMVectorSet(127.0f / 255.0f, 127.0f / 255.0f, 127.0f / 255.0f, 1.0f);
                        }

                        ID_InputBoxSelected = true;
                    }
                    else
                    {
                        inputTextBox_Password.SetActivate(true);
                        inputTextBox_ID.SetActivate(false);

                        if (inputTextBox_Password.GetFullinputTexts().size() == 0)
                        {
                            Password_RenderTextObject->m_Textinfo->m_Text.clear();
                            Password_RenderTextObject->m_Textinfo->m_TextColor = XMVectorSet(38.0f / 255.0f, 38.0f / 255.0f, 38.0f / 255.0f, 1.0f);
                        }

                        if (inputTextBox_ID.GetFullinputTexts().size() == 0)
                        {
                            ID_RenderTextObject->m_Textinfo->m_Text = L"ID";
                            ID_RenderTextObject->m_Textinfo->m_TextColor = XMVectorSet(127.0f / 255.0f, 127.0f / 255.0f, 127.0f / 255.0f, 1.0f);
                        }

                        ID_InputBoxSelected = false;
                    }

                    InputBoxSelected = true;
                    InputBoxSelectChange = false;
                }
            }
        }
    }

    // Process Login Enter Key
    {
        if (key_state[VK_RETURN] && OnceSendLogin == false)
        {
            std::wstring id = inputTextBox_ID.GetFullinputTexts();
            std::wstring pw = inputTextBox_Password.GetFullinputTexts();

            if (id.size() > 0 && pw.size() > 0)
            {
                inputTextBox_ID.InitTexts();
                inputTextBox_Password.InitTexts();

                EventManager eventManager;
                eventManager.ReservateEvent_TryLogin(GeneratedEvents, id, pw);
            }

            OnceSendLogin = true;
        }
        else OnceSendLogin = false;
    }

    // Process Login Button
    {
        Object* LoginButtonObject = ObjManager.FindObjectName(m_UILayOutObjects, "UI_Layout_LoginButton");

        if (key_state[VK_LBUTTON] == true)
        {
            auto& LoginButtonBound = LoginButtonObject->m_TransformInfo->m_Bound;

            RECT LoginButtonAreainScreen =
            {
                (LONG)(LoginButtonBound.Center.x - LoginButtonBound.Extents.x), // left
                (LONG)(LoginButtonBound.Center.y + LoginButtonBound.Extents.y), // top
                (LONG)(LoginButtonBound.Center.x + LoginButtonBound.Extents.x), // right
                (LONG)(LoginButtonBound.Center.y - LoginButtonBound.Extents.y), // bottom
            };

            float CursorPos_x = oldCursorPos.x - m_width * 0.5f;
            float CursorPos_y = -(oldCursorPos.y - m_height * 0.5f);

            if (PointInRect(CursorPos_x, CursorPos_y, LoginButtonAreainScreen) == true)
            {
                if (LoginButtonPress == false && LoginButtonUp == true)
                {
                    auto& AllRitems = *m_AllRitemsRef;
                    auto Ritem_ButtonPress = AllRitems.find("UI_Layout_LoginButton_Press")->second.get();
                    LoginButtonObject->m_RenderItems = { Ritem_ButtonPress };

                    LoginButtonPress = true;
                    LoginButtonUp = false;
                }
            }
        }
        else
        {
            if (LoginButtonPress == true && LoginButtonUp == false)
            {
                auto& AllRitems = *m_AllRitemsRef;
                auto Ritem_ButtonPress = AllRitems.find("UI_Layout_LoginButton")->second.get();
                LoginButtonObject->m_RenderItems = { Ritem_ButtonPress };

                std::wstring id = inputTextBox_ID.GetFullinputTexts();
                std::wstring pw = inputTextBox_Password.GetFullinputTexts();

                if (id.size() > 0 && pw.size() > 0)
                {
                    inputTextBox_ID.InitTexts();
                    inputTextBox_Password.InitTexts();

                    EventManager eventManager;
                    eventManager.ReservateEvent_TryLogin(GeneratedEvents, id, pw);
                }

                LoginButtonPress = false;
                LoginButtonUp = true;
            }
        }
    }

    /*if (OnceSendLogin == false) {
        if (true == key_state['D'])
        {
            std::wstring id{ L"test001" };
            std::wstring pw{ L"0000" };
            EventManager eventManager;
            eventManager.ReservateEvent_TryLogin(GeneratedEvents, id, pw);
            OnceSendLogin = true;
        }
    }*/
}
